/**
 * This file, alder_kmer_thread5.c, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#define ALDER_KMER_THREAD5_WRITEBUFFER_SIZE (1 << 20)

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include "bstrlib.h"
#include "rngs.h"
#include "alder_logger.h"
#include "alder_hash.h"
#include "alder_cmacro.h"
#include "alder_progress.h"
#include "alder_encode.h"
#include "alder_file.h"
#include "alder_bit.h"
#include "alder_fileseq.h"
#include "alder_kmer.h"
#include "alder_mcas_wf.h"
#include "alder_hashtable.h"
#include "alder_kmer_binary.h"
#include "libdivide.h"
#include "alder_kmer_binary.h"
#include "alder_kmer_thread5.h"

/**
 *  Structure of buffers
 *  --------------------
 *  inbuf: There are [n_subbuf]-many sub buffers. The main inbuf does not have
 *         any header. But, each sub buffer does;
 *         state (1B) partition index (8B), and length (8B).
 *         So, the body start at 17B. The number of sub buffers is determined
 *         by the number of threads. Th size of buffer after this header is
 *         fixed at BUFSIZE + b, where b is the number of bytes for a kmer.
 *         The last byte is not used because I'd save only b-1 bytes.
 *
 *  Reader
 *  ------
 *  The reader thread:
 *  1. opens a par file.
 *  2. computes the number of buffer blocks in the par file (n_blockByReader)
 *  3. selects an empty input buffer.
 *  4. reads a block of input buffer.
 *  5. releases the input buffer full or empty.
 *  6. closes the par file.
 *  7. finishes all of the input buffers.
 *
 *  Writer
 *  ------
 *  1. selects a finished main table.
 *  2. writes the table to the output file.
 *  3. releases the empty table.
 *  4. increments the main partition index.
 *  5. checks if all of the tables are empty, when all of blocks are done.
 *
 *  Counter
 *  -------
 *  1. Variables for decoding kmers: m, m2, res_key, ib, jb.
 *  2. Write a table file if needed; or when c_table is 0 or 1.
 *  3. Exit if I reach the final block.
 *  4. Read an input buffer.
 *  5. Exit if no input buffer available.
 *
 *  2. First, it attempts to choose an input buffer from the main partition.
 *  3. Second, if the firty attempt failed, it would choose one from secondary.
 *  4. Use n_blockByReader and n_blockByCounter to check the final buffer.
 */

static int counter_id_counter = 0;

#define BUFSIZE (1 << 16)

#define ALDER_KMER_COUNTER5_INBUFFER_I_NP        1
#define ALDER_KMER_COUNTER5_INBUFFER_LENGTH      9
#define ALDER_KMER_COUNTER5_INBUFFER_BODY        17

/* from alder_kmer_encode5.c */
#define ALDER_KMER_SECONDARY_BUFFER_SIZE         1000
#define ALDER_KMER_ENCODER5_OUTBUFFER_HEADER     1
#define ALDER_KMER_ENCODER5_OUTSUBBUFFER_HEADER  8

//
#define ALDER_KMER_ENCODER5_OUTBUFFER_BODY       8

#define ALDER_KMER_ENCODER5_OUTBUFFER_A          0
#define ALDER_KMER_ENCODER5_OUTBUFFER_B          8
#define ALDER_KMER_ENCODER5_OUTBUFFER_C          16

#define ALDER_KMER_ENCODER5_INBUFFER_TYPE_INFILE 1
#define ALDER_KMER_ENCODER5_INBUFFER_CURRENT     2
#define ALDER_KMER_ENCODER5_INBUFFER_LENGTH      10
#define ALDER_KMER_ENCODER5_INBUFFER_BODY        18
#define ALDER_KMER_ENCODER5_FILETYPE_FASTA       1
#define ALDER_KMER_ENCODER5_FILETYPE_FASTQ       2
#define ALDER_KMER_ENCODER5_FILETYPE_SEQ         3
#define ALDER_KMER_ENCODER5_FILETYPE_BINARY      4


static void *counter(void *t);

/**
 *  This function creates a bstring of a partition file name.
 *  The caller is responsible for freeing the bstring.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return partition file name or NULL
 */
static bstring
filename_partition(alder_kmer_thread5_t *o, uint64_t i_np)
{
    /* File name setup */
    bstring bfpar = NULL;
    if (o->infile == NULL) {
        bfpar = bformat("%s/%s-%llu-%llu.par",
                        bdata(o->boutdir), bdata(o->boutfile),
                        o->i_ni, i_np);
        if (bfpar == NULL) return NULL;
    } else {
        bfpar = bstrcpy(o->infile->entry[i_np]);
        if (bfpar == NULL) return NULL;
    }
    return bfpar;
}

static void
alder_kmer_thread5_destroy(alder_kmer_thread5_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
//        for (size_t i = 0; i < o->n_ht; i++) {
//            alder_hashtable_mcas_destroy(o->ht[i]); <- freed by XFREE(inbuf)
//        }
        XFREE(o->ht);
        XFREE(o->inbuf);
        XFREE(o->n_blockByReader);
        XFREE(o->n_blockByCounter);
        XFREE(o->cur_inbuf);
        XFREE(o->cur_i_np);
        bdestroy(o->boutdir);
        bdestroy(o->boutfile);
        XFREE(o);
    }
}

static alder_kmer_thread5_t *
alder_kmer_thread5_create(FILE *fpout,
                          int n_counter,
                          uint64_t i_ni,
                          int kmer_size,
                          long memory_available,
                          long sizeInbuffer,
                          long sizeOutbuffer,
                          uint64_t n_ni,
                          uint64_t n_np,
                          size_t n_nh,
                          size_t totalfilesize,
                          size_t n_byte,
                          int progress_flag,
                          int progressToError_flag,
                          int nopack_flag,
                          struct bstrList *infile,
                          const char *outdir,
                          const char *outfile)
{
    alder_log5("Creating alder_kmer_thread5_t...");
    alder_kmer_thread5_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create alder_kmer_thread5_t");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    assert(i_ni == 0);
    assert(n_ni == 1);
    assert(n_counter > 0);
    
    /* init */
    o->k = kmer_size;
    o->b = alder_encode_number2_bytesize(kmer_size);
    o->i_ni = 0;
    o->n_ni = 1;
    o->n_np = n_np;
    o->n_counter = n_counter;
    o->cur_inbuf = NULL;
    o->cur_i_np = NULL;
    o->size_subinbuf = sizeInbuffer;
    o->size_inbuf = ((size_t)memory_available << 20);
    o->inbuf = NULL;
    o->n_blockByReader = NULL;
    o->n_blockByCounter = NULL;
    o->size_data = 0;
    o->size_writebuffer = sizeOutbuffer;
    o->n_ht = n_counter == 1 ? 1 : 2;
    o->main_i_np = 0;
    o->main_table = 0;
    o->ht = NULL;
    o->infile = infile;
    o->fpout = fpout;
    o->boutfile = NULL;
    o->boutdir = NULL;
    o->n_byte = n_byte;
    o->n_kmer = 0;
    o->n_hash = 0;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->isMultithreaded = (n_counter > 1);
    
    /* memory */
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->cur_inbuf = malloc(sizeof(*o->cur_inbuf) * n_counter);
    o->cur_i_np = malloc(sizeof(*o->cur_i_np) * n_counter);
    o->n_blockByReader = malloc(sizeof(*o->n_blockByReader) * o->n_ht);
    o->n_blockByCounter = malloc(sizeof(*o->n_blockByCounter) * o->n_ht);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf); /* bigmem */
    o->ht = malloc(sizeof(*o->ht) * o->n_ht);
    if (o->boutfile == NULL || o->boutdir == NULL ||
        o->cur_inbuf == NULL || o->cur_i_np == NULL ||
        o->n_blockByReader == NULL || o->n_blockByCounter == NULL ||
        o->inbuf == NULL || o->ht == NULL) {
        alder_kmer_thread5_destroy(o);
        return NULL;
    }
    memset(o->cur_inbuf, 0, sizeof(*o->cur_inbuf) * n_counter);
    memset(o->cur_i_np, 0, sizeof(*o->cur_i_np) * n_counter);
    memset(o->n_blockByReader, 0, sizeof(*o->n_blockByReader) * o->n_ht);
    memset(o->n_blockByCounter, 0, sizeof(*o->n_blockByCounter) * o->n_ht);
    memset(o->inbuf,0,sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->ht,0,sizeof(*o->ht) * o->n_ht);
    
    alder_log5("Finish - Creating alder_kmer_kmerrw_t...");
    return o;
}

/**
 *  This function
 *
 *  1. Update np and nh.
 *  2. Rewrite the head of a table file.
 *
 *  @param o data
 *
 *  @return SUCCESS or FAIL
 */
static int
alder_kmer_thread5_setup(alder_kmer_thread5_t *o)
{
    // Compute the number of partitions.
    size_t memory_space_table = o->size_inbuf - o->size_data;
    if (memory_space_table < (1LLU << 25)) {
        alder_loge(ALDER_ERR_MEMORY,
                   "not enough memory for a table; version 6 run failed!");
        return ALDER_STATUS_ERROR;
    }
    uint64_t prev_n_np = o->n_np;
    
    /* */
    size_t memory_space_each_table = memory_space_table / o->n_ht;
    o->n_nh = alder_hashtable_mcas_size_with_available_memory(o->k, memory_space_each_table);
    memory_space_each_table = alder_hashtable_mcas_sizeof(o->k, o->n_nh);
    size_t n_nh_point7 = (size_t)((double)o->n_nh * 7.0 / 10.0);
    o->n_np = (uint64_t)((double)o->n_kmer / (double)n_nh_point7);
    o->n_np = o->n_np == 0 ? 1 : o->n_np;

    /* */
    alder_log("Change of partitions: %llu -> %llu", prev_n_np, o->n_np);
    lseek(fileno(o->fpout), 0, SEEK_SET);
    alder_hashtable_mcas_printHeaderToFD(fileno(o->fpout),
                                         o->k,
                                         o->n_nh,
                                         o->n_ni,
                                         o->n_np);
    
    // This is not known till binary3 is finished.
    for (size_t i = 0; i < o->n_ht; i++) {
        void *mem = o->inbuf + o->size_data + memory_space_each_table * i;
        o->ht[i] = alder_hashtable_mcas_createWithMemory(o->k, o->n_nh, mem, memory_space_each_table);
        o->ht[i]->i_np = i;
        o->ht[i]->state = ALDER_HASHTABLE_MCAS_STATE_UNFINISHED;
    }
    
    /* Compute the number of input buffer blocks. */
    assert(o->size_data % o->size_subinbuf == 0);
    int64_t z = (int64_t)(o->size_data / o->size_subinbuf);
    o->n_blockByReader[0] = z;
    o->n_blockByReader[1] = z;
    
    for (int i = 0; i < o->n_counter; i++) {
        o->cur_inbuf[i] = o->size_data - o->size_subinbuf;
        o->cur_i_np[i] = -1;
    }

    return ALDER_STATUS_SUCCESS;
}
                              
#pragma mark main

static pthread_mutex_t mutex_write;

/**
 *  This function implements the 5th version of counters.
 *
 *  @param fpout                file out
 *  @param n_counter            number of counters
 *  @param i_ni                 iteration index
 *  @param kmer_size            kmer size
 *  @param memory_available     memory
 *  @param sizeInbuffer         input buffer size
 *  @param n_ni                 iterations
 *  @param n_np                 partitions
 *  @param n_nh                 hash table size
 *  @param totalfilesize        total file size
 *  @param n_byte               return number of bytes
 *  @param n_hash               return number of kmers
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to std err
 *  @param nopack_flag          no pack
 *  @param infile               input files
 *  @param outdir               output directory
 *  @param outfile              output file name
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_count_iteration5(FILE *fpout,
                                int n_counter,
                                int i_ni,
                                int K,
                                long memory_available,
                                long sizeInbuffer,
                                long sizeOutbuffer,
                                uint64_t n_ni,
                                uint64_t n_np,
                                size_t n_nh,
                                size_t totalfilesize,
                                size_t *n_byte,
                                size_t *n_hash,
                                int progress_flag,
                                int progressToError_flag,
                                int nopack_flag,
                                struct bstrList *infile,
                                const char *outdir,
                                const char *outfile)
{
    alder_log5("preparing counting Kmers with version 6...");
    counter_id_counter = 0;
    int s = 0;
    if (n_counter > 1 && K > 31) {
        mcas_init(n_counter);
    }
    int n_thread = n_counter;
    
    alder_kmer_thread5_t *data =
    alder_kmer_thread5_create(fpout,
                              n_counter,
                              i_ni,
                              K,
                              memory_available,
                              sizeInbuffer,
                              sizeOutbuffer,
                              n_ni,
                              n_np,
                              n_nh,
                              totalfilesize,
                              *n_byte,
                              progress_flag,
                              progressToError_flag,
                              nopack_flag,
                              infile,
                              outdir,
                              outfile);
    if (data == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "failed to creat counter threads.");
        return ALDER_STATUS_ERROR;
    }
    
    // Loading a whole input data.
    // Use binary3.
    s = alder_kmer_binary3(data->inbuf, data->size_inbuf, data->size_subinbuf,
                           &data->n_kmer, &data->n_dna, &data->n_seq,
                           3, // version 3
                           K,
                           0, // D,
                           0, // n_nt
                           totalfilesize,
                           &data->size_data,
                           progress_flag,
                           progressToError_flag,
                           infile, outdir,
                           outfile);
    // another create.
    alder_kmer_thread5_setup(data);
    
    // Counting.
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create threads for counting");
        XFREE(threads);
        alder_kmer_thread5_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    memset(threads,0,sizeof(*threads)*n_thread);
    
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_mutex_init(&mutex_write, NULL);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_counter; i++) {
        s += pthread_create(&threads[i+0], &attr, counter, (void *)data);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "cannot create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    
    /* Wait for all threads to complete */
    alder_log3("main(): waiting for all threads to complete...");
    for (int i = 0; i < n_thread; i++) {
        s += pthread_join(threads[i], NULL);
        if (s != 0) {
            alder_loge(ALDER_ERR_THREAD, "cannot join threads - %s",
                       strerror(errno));
            goto cleanup;
        }
    }
    
cleanup:
    *n_byte = data->n_byte;
    *n_hash = data->n_hash;
    /* Cleanup */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex_write);
    XFREE(threads);
    alder_kmer_thread5_destroy(data);
    alder_log5("Counting Kmers has been finished with %d threads.", n_thread);
    return 0;
}

#pragma mark buffer

/**
 *  This function assign incremental numbers to counter threads.
 *
 *  @return assigned counter id
 */
static int
assign_counter_id()
{
    int oval = counter_id_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = counter_id_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&counter_id_counter, (int)oval, (int)cval);
    }
    return oval;
}


#pragma mark thread

/**
 *  This function writes the main table to a file.
 *
 *  @param a       counter
 *  @param c_table table id
 *  @param i_np    partition id
 *
 *  @return 2 for multiple-threaded, 1 otherwise.
 */
static __attribute__ ((noinline)) size_t
write_tabfile(alder_kmer_thread5_t *a, size_t c_table, uint64_t i_np)
{
    // c_table of main i_np must enter first.
    int s;
    //    assert(a->ht[c_table]->i_np == a->main_i_np);
    while (i_np != a->main_i_np) {
        assert(i_np + 1 == a->main_i_np);
        alder_loge(ALDER_ERR_THREAD,
                   "Partition is too smaller in size; the secondary thread is sleeping..."
                   "increase the size of memory");
        sleep(1);
    }
    pthread_mutex_lock(&mutex_write);
    assert(a->main_i_np == i_np);
    
    // Use counter id to find the table from the input buffer.
    int64_t s_count = alder_hashtable_mcas_printPackToFD(a->ht[c_table],
                                                         fileno(a->fpout),
                                                         a->size_writebuffer);
    if (s_count < 0) {
        alder_loge(ALDER_ERR_HASH,
                   "cannot save a hash table (%llu-%llu)",
                   a->i_ni, a->main_i_np);
    }
    a->n_hash += s_count;
    s = alder_hashtable_mcas_printCountPerPartitionFD(fileno(a->fpout),
                                                      s_count,
                                                      a->i_ni,
                                                      i_np,
                                                      a->n_np);
//    XFCLOSE(a->fpout);
    assert(s == ALDER_STATUS_SUCCESS);
    
    /* Reset the table. */
    alder_hashtable_mcas_reset(a->ht[c_table]);
    
    /* Change the main i_np. */
    a->main_i_np++;
    a->n_blockByCounter[c_table] = 0;
    pthread_mutex_unlock(&mutex_write);
    return a->n_ht;
}

/**
 *  This function reads a block of input data. The name of the function is
 *  just a remnant from the previous version of thread4.
 *
 *  This function assigns one of fixed-size inbuf to the given counter.
 *  Each buffer has partition index. When this index reaches the number of
 *  partitions, I'd exit.
 *
 *  @param a          counter
 *  @param counter_id counter id
 *
 *  @return counter_id
 */
static __attribute__ ((noinline)) uint8_t*
read_binbuffer(alder_kmer_thread5_t *a, int counter_id)
{
    int s = alder_kmer_counter5_lock_reader(a, counter_id);
    if (s != ALDER_YES) {
        return NULL;
    }
    
    /* Read a block of input buffer. */
    uint8_t *inbuf = a->inbuf + a->cur_inbuf[counter_id];
    uint64_t i_np = (uint64_t)(to_uint32(inbuf,0)) - 1;
    assert(i_np % 2 == 0);
    i_np /= 2;
    assert((uint64_t)a->cur_i_np[counter_id] == i_np);
    assert(a->cur_inbuf[counter_id] < a->size_data);
    if (i_np < a->n_np) {
        return inbuf;
    } else {
        return NULL;
    }
}

/**
 *  This function is for the counter thread.
 *
 *  @param t argument
 *
 *  @return SUCCESS or FAIL
 */
static void *counter(void *t)
{
    int counter_id = assign_counter_id();
    alder_log("counter(%d): START", counter_id);
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_thread5_t *a = (alder_kmer_thread5_t*)t;
    assert(a != NULL);
    
    /*************************************************************************/
    /*                           alder_kmer_encode5.c                        */
    /*************************************************************************/
    alder_encode_number2_t * s1 = NULL;
    alder_encode_number2_t * s2 = NULL;
    int K = a->k;
    s1 = alder_encode_number2_createWithKmer(K);
    s2 = alder_encode_number2_createWithKmer(K);
    size_t *len_outbuf2 = malloc(sizeof(*len_outbuf2) * a->n_np);
    memset(len_outbuf2, 0, sizeof(*len_outbuf2) * a->n_np);
//    struct libdivide_u64_t fast_ni = libdivide_u64_gen(a->n_ni);
    struct libdivide_u64_t fast_np = libdivide_u64_gen(a->n_np);
    alder_kmer_binary_stream_t *bs = alder_kmer_binary_buffer_create();
//    size_t ib = s1->b / 8;
//    size_t jb = s1->b % 8;
//    uint64_t ni = a->n_ni;
    uint64_t np = a->n_np;
    /*************************************************************************/
    /*                           alder_kmer_encode5.c                        */
    /*************************************************************************/
    
    /* Numbers for decoding kmers. */
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(a->k);
    alder_encode_number2_t *m2 = alder_encode_number2_createWithKmer(a->k);
    uint64_t *res_key = malloc(sizeof(*res_key) * m->s);
    if (m == NULL || m2 == NULL || res_key == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "Fatal - counter(): not enough memory");
        pthread_exit(NULL);
    }
    memset(res_key, 0, sizeof(*res_key) * m->s);
//    size_t ib = m2->b / 8;
//    size_t jb = m2->b % 8;
    size_t kb = ALDER_BYTESIZE_KMER(m2->b, 8);
    size_t isMultithreaded = a->isMultithreaded ? 1 : 0;
    
    /* Let's read inputs, and skip writing table files. */
//    size_t c_inbuffer = a->n_counter;
    size_t c_table = a->n_ht;
    uint64_t i_np = a->n_np;
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_table < a->n_ht) {
            c_table = write_tabfile(a, c_table, i_np);
            alder_log("counter(%d): Partition: %llu (out of %llu)", counter_id, i_np, a->n_np);
        }
        
        /* Check if this is the final. */
        //FIXME when to exit
//        if (a->n_blockByReader[a->n_np-1] == a->n_blockByCounter[a->n_np-1]) {
//            /* Exit! */
//            break;
//        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
        uint8_t *inbuf = read_binbuffer(a, counter_id);
        if (inbuf == NULL) {
            /* Exit! */
            break;
        }
        i_np = a->cur_i_np[counter_id];
        // At this point, I'd must have a block of buffer. Otherwise, I would
        // have exited in the while-loop.
        
        ///////////////////////////////////////////////////////////////////////
        // Table Selector
        ///////////////////////////////////////////////////////////////////////
        if (a->main_i_np == i_np || a->main_i_np + isMultithreaded == i_np) {
            c_table = i_np % a->n_ht;
#if !defined(NDEBUG)
            if (a->n_counter == 1) {
                assert(c_table == 0);
                assert(a->main_i_np == i_np);
            }
#endif
        } else if (a->main_i_np + 1 < i_np) {
            assert(a->n_counter > 1);
            while (a->main_i_np + 1 < i_np) {
                // No code.
                // To compile this file without optimizing this while-loop,
                // I need to put
                // asm("")
                // http://stackoverflow.com/questions/7083482/how-to-prevent-compiler-optimization-on-a-small-piece-of-code
                asm("");
                alder_loge3(ALDER_ERR_THREAD,
                            "WARN: partition files are too small, "
                            "to many threads, some threads are sleeping...zzz");
            }
            assert(a->main_i_np == i_np || a->main_i_np + isMultithreaded == i_np);
            c_table = i_np % 2;
        } else {
            assert(a->n_counter > 1);
            alder_loge(ALDER_ERR, "Fatal! "
                       "The current partition must have been processed.");
            assert(0);
            abort();
        }
        assert(c_table < a->n_ht);
        
        ///////////////////////////////////////////////////////////////////////
        // Encoder (single iteration/multiple partitions)
        // Counter
        ///////////////////////////////////////////////////////////////////////
        assert(c_table < a->n_ht);
//        size_t lenbuf = to_size(inbuf,ALDER_KMER_BINARY_READBLOCK_LEN);
//        uint8_t *inbuf_body = inbuf + ALDER_KMER_BINARY_READBLOCK_BODY;
        alder_hashtable_mcas_t *c_ht = a->ht[c_table];
        
        int token = 4;
        alder_kmer_binary_buffer_open(bs, inbuf, a->size_subinbuf);
        alder_log3("encoder(%d): fresh new input buffer");
        
        /* 3. While input is not empty ... */
        while (token < 5) {
            
            if (token == 4) {
                // Create a number.
                token = 0;
                alder_log5("A starting Kmer is being created.");
                for (int i = 0; i < a->k - 1; i++) {
                    
                    token = alder_kmer_binary_buffer_read(bs,NULL);
                    
                    if (token >= 4) break;
                    int b1 = token;
                    int b2 = (b1 + 2) % 4;
                    alder_encode_number2_shiftLeftWith(s1,b1);
                    alder_encode_number2_shiftRightWith(s2,b2);
                }
                if (token >= 4) {
                    continue;
                }
            }
            /* 4. Form a Kmer and its reverse complementary. */
            token = alder_kmer_binary_buffer_read(bs,NULL);
            if (token >= 4) continue;
            alder_log5("token: %d", token);
            int b1 = token;
            int b2 = (b1 + 2) % 4;
            alder_encode_number2_shiftLeftWith(s1,b1);
            alder_encode_number2_shiftRightWith(s2,b2);
            
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            /* 5. Select partition, and kmer of the two. */
            alder_encode_number2_t *ss = NULL;
            uint64_t x_np = 0;
            uint64_t hash_s1 = alder_encode_number2_hash(s1);
            uint64_t hash_s2 = alder_encode_number2_hash(s2);
            uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
            ss = (hash_s1 < hash_s2) ? s1 : s2;
            uint64_t h_over_np = libdivide_u64_do(hash_ss, &fast_np);
            x_np = hash_ss - h_over_np * np;
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            
//            if (x_np == a->main_i_np || x_np == a->main_i_np + isMultithreaded) {
            if (x_np == i_np) {
                for (size_t i = 0; i < kb; i++) {
                    m->n[i] = ss->n[i].key64;
                }
                s = alder_hashtable_mcas_increment_with_hashcode(c_ht, m->n,
                                                                 hash_ss,
                                                                 res_key,
                                                                 a->isMultithreaded);
                if (s != ALDER_STATUS_SUCCESS) {
                    alder_loge(ALDER_ERR, "Fatal - table is full!");
                    assert(0);
                    abort();
                }
            }
        }
        
        assert(token == 5);
        
        // Increment the number of input blocks processed.
        int isLastBlock = alder_kmer_counter5_increment_n_block(a, i_np, c_table, counter_id);
        if (isLastBlock) {
            // No code.
        } else {
            // Not the last bock, so won't write to a table file.
            c_table = a->n_ht;
        }
        alder_kmer_counter5_unlock_reader(a, counter_id);
    }
    XFREE(res_key);
    alder_encode_number_destroy(m);
    
    /*************************************************************************/
    /*                           alder_kmer_encode5.c                        */
    /*************************************************************************/
    alder_kmer_binary_buffer_destroy(bs);
    alder_encode_number2_destroy(m2);
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    /*************************************************************************/
    /*                           alder_kmer_encode5.c                        */
    /*************************************************************************/
    
    alder_log("counter(%d): END", counter_id);
    pthread_exit(NULL);
}









