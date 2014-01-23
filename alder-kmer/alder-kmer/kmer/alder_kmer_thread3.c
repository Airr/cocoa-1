/**
 * This file, alder_kmer_thread3.c, is part of alder-kmer.
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

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
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
#include "alder_kmer_thread3.h"

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

#define ALDER_KMER_COUNTER3_INBUFFER_I_NP        1
#define ALDER_KMER_COUNTER3_INBUFFER_LENGTH      9
#define ALDER_KMER_COUNTER3_INBUFFER_BODY        17


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
filename_partition(alder_kmer_thread3_t *o, uint64_t i_np)
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

/**
 *  This function computes the number of buffer blocks in a par file.
 *  This updates n_blockByReader, which denotes the number of input buffer
 *  blocks for a par file. This number is compared with one in n_blockByCounter
 *  to check whether I have reached a final input buffer block.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return SUCCESS or FAIL
 */
static int
compute_number_block(alder_kmer_thread3_t *o, uint64_t i_np)
{
    bstring bfpar = filename_partition(o, i_np);
    if (bfpar == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    /* Compute the number of buffer blocks in the file. */
    size_t file_size = 0;
    alder_file_size(bdata(bfpar), &file_size);
//    size_t size_body = o->size_subinbuf - ALDER_KMER_COUNTER3_INBUFFER_BODY;
//    o->n_blockByReader[i_np] = (int)ALDER_BYTESIZE_KMER(file_size,size_body);
    o->n_blockByReader[i_np] = (int)ALDER_BYTESIZE_KMER(file_size,BUFSIZE);
    
    bdestroy(bfpar);
    return ALDER_STATUS_SUCCESS;
}


static void
alder_kmer_thread3_destroy(alder_kmer_thread3_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
        XFCLOSE(o->fpin);
        for (size_t i = 0; i < o->n_ht; i++) {
            alder_hashtable_mcas_destroy(o->ht[i]);
        }
        XFREE(o->ht);
        XFREE(o->inbuf);
        XFREE(o->next_inbuf);
        XFREE(o->n_blockByReader);
        XFREE(o->n_blockByCounter);
        bdestroy(o->boutdir);
        bdestroy(o->boutfile);
        XFREE(o);
    }
}

static alder_kmer_thread3_t *
alder_kmer_thread3_create(FILE *fpout,
                          int n_counter,
                          uint64_t i_ni,
                          int kmer_size,
                          long memory_available,
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
    alder_log5("Creating alder_kmer_thread3_t...");
    alder_kmer_thread3_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create alder_kmer_thread3_t");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->k = kmer_size;
    o->b = alder_encode_number2_bytesize(kmer_size);
    o->i_ni = i_ni;
    o->n_ni = n_ni;
    o->n_np = n_np;
    o->n_counter = n_counter;
    o->lock_reader = ALDER_KMER_COUNTER3_READER_UNLOCK;
    o->lock_writer = ALDER_KMER_COUNTER3_WRITER_UNLOCK;
    o->next_lenbuf = 0;
    o->reader_lenbuf = 0;
    o->reader_i_parfile = 0;
//    o->size_inbuf = (size_t)(memory_available << 19);
    o->size_subinbuf = ALDER_KMER_COUNTER3_INBUFFER_BODY + BUFSIZE + o->b;
    o->size_inbuf = o->size_subinbuf * n_counter;
    o->n_subbuf = n_counter;
    o->inbuf = NULL;
    o->n_blockByReader = NULL;
    o->n_blockByCounter = NULL;
    assert(n_counter > 0);
    if (n_counter == 1) {
        o->n_ht = 1;
    } else {
        o->n_ht = 2;
    }
    o->main_i_np = 0;
    o->main_table = 0;
    o->ht = NULL;
    o->infile = infile;
    o->fpout = fpout;
    o->boutfile = NULL;
    o->boutdir = NULL;
    o->fpin = NULL;
    o->n_byte = n_byte;
    o->n_kmer = 0;
    o->n_hash = 0;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->nopack_flag = nopack_flag;
    o->isMultithreaded = (n_counter > 1);
    o->status = 0;
    
    /* adjust size inbuf */
//    if (o->size_subinbuf < ALDER_KMER_COUNTER3_INBUFFER_BODY) {
//        alder_loge(ALDER_ERR_MEMORY, "failed to create threads for counters.");
//        alder_loge(ALDER_ERR_MEMORY, "Too small inbuf size.");
//        alder_kmer_thread3_destroy(o);
//        return NULL;
//    }
//    o->size_subinbuf -= ALDER_KMER_COUNTER3_INBUFFER_BODY;
//    o->size_subinbuf = alder_encode_number2_adjustBufferSizeForKmer(kmer_size, o->size_subinbuf);
//    o->size_subinbuf += ALDER_KMER_COUNTER3_INBUFFER_BODY;
//    o->size_inbuf = o->size_subinbuf * o->n_subbuf;
    
    
    /* memory */
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->n_blockByCounter = malloc(sizeof(*o->n_blockByCounter) * n_np);
    o->n_blockByReader = malloc(sizeof(*o->n_blockByReader) * n_np);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf); /* bigmem */
    o->next_inbuf = malloc(sizeof(*o->inbuf) * o->b); /* bigmem */
    o->ht = malloc(sizeof(*o->ht) * o->n_ht);
    if (o->ht == NULL || o->boutdir == NULL || o->inbuf == NULL ||
        o->next_inbuf == NULL ||
        o->n_blockByReader == NULL || o->n_blockByCounter == NULL) {
        alder_kmer_thread3_destroy(o);
        return NULL;
    }
    memset(o->inbuf,0,sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->next_inbuf,0,sizeof(*o->next_inbuf) * o->b);
    memset(o->ht,0,sizeof(*o->ht) * o->n_ht);
    memset(o->n_blockByCounter, 0, sizeof(*o->n_blockByCounter) * n_np);
    memset(o->n_blockByReader, 0, sizeof(*o->n_blockByReader) * n_np);
    assert(o->fpin == NULL);
    
    for (size_t i = 0; i < o->n_ht; i++) {
        o->ht[i] = alder_hashtable_mcas_create(kmer_size, n_nh); /* bigmem */
        o->ht[i]->i_np = i;
        o->ht[i]->state = ALDER_HASHTABLE_MCAS_STATE_UNFINISHED;
    }
    
    /* Compute the number of input buffer blocks. */
    for (uint64_t i_np = 0; i_np < o->n_np; i_np++) {
        int s = compute_number_block(o, i_np);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR_THREAD,
                       "faild to open a partition file");
            
        }
    }
    
    alder_log5("Finish - Creating alder_kmer_kmerrw_t...");
    return o;
}

#pragma mark local

static int close_parfile (alder_kmer_thread3_t *o)
{
    assert(o != NULL);
    XFCLOSE(o->fpin);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function opens a partition file. It also computes the number of
 *  input buffer blocks for identifying a final buffer block.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return SUCCESS or FAIL
 */
static int
open_parfile(alder_kmer_thread3_t *o, uint64_t i_np)
{
    bstring bfpar = filename_partition(o, i_np);
    if (bfpar == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    /* Open a file */
    assert(o->fpin == NULL);
    alder_log2("reader(): openning a partition file %s", bdata(bfpar));
    o->fpin = fopen(bdata(bfpar), "rb");
    if (o->fpin == NULL) {
        alder_loge(ALDER_ERR_FILE, "cannot open the input file %s - %s",
                   bdata(bfpar), strerror(errno));
        bdestroy(bfpar);
        return ALDER_STATUS_ERROR;
    }
    bdestroy(bfpar);
    return ALDER_STATUS_SUCCESS;
}


#pragma mark main

/**
 *  This function implements a 2nd version of counters.
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
int alder_kmer_count_iteration3(FILE *fpout,
                                int n_counter,
                                int i_ni,
                                int kmer_size,
                                long memory_available,
                                long sizeInbuffer,
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
    alder_log5("preparing counting Kmers with version 2...");
    counter_id_counter = 0;
    int s = 0;
    if (n_counter > 1 && kmer_size > 31) {
        mcas_init(n_counter);
    }
    int n_thread = n_counter + 0;
    
    alder_kmer_thread3_t *data = alder_kmer_thread3_create(fpout,
                                                           n_counter,
                                                           i_ni,
                                                           kmer_size,
                                                           memory_available,
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
    alder_log5("creating %d threads: one for reader, and one for writer",
               n_thread);
    
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create threads for counting");
        XFREE(threads);
        alder_kmer_thread3_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    memset(threads,0,sizeof(*threads)*n_thread);
    
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
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
    XFREE(threads);
    alder_kmer_thread3_destroy(data);
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
write_tabfile(alder_kmer_thread3_t *a, size_t c_table, uint64_t i_np)
{
    // c_table of main i_np must enter first.
    int s;
//    assert(a->ht[c_table]->i_np == a->main_i_np);
    
    alder_kmer_counter_lock_writer(a, i_np);
    assert(a->main_i_np == i_np);
    
    // Use counter id to find the table from the input buffer.
    int64_t s_count = alder_hashtable_mcas_printPackToFILE(a->ht[c_table],
                                                           a->fpout);
    if (s_count < 0) {
        alder_loge(ALDER_ERR_HASH,
                   "cannot save a hash table (%llu-%llu)",
                   a->i_ni, a->main_i_np);
    }
    a->n_hash += s_count;
    /* Print the number of elements in the current partition. */
    s = alder_hashtable_mcas_printCountPerPartition(a->fpout,
                                                    s_count,
                                                    a->i_ni,
                                                    i_np,
                                                    a->n_np);
    assert(s == ALDER_STATUS_SUCCESS);
    
    /* Reset the table. */
    alder_hashtable_mcas_reset(a->ht[c_table]);
    
    /* Change the main i_np. */
    a->main_i_np++;
    alder_kmer_counter_unlock_writer(a);
    return a->n_ht;
}

/**
 *  This function reads a block of partition files.
 *
 *  @param a          counter
 *  @param counter_id counter id
 *
 *  @return counter_id
 */
static __attribute__ ((noinline)) int
read_parfile(alder_kmer_thread3_t *a, int counter_id)
{
    int s;
    alder_kmer_counter_lock_reader(a);
    size_t c_inbuffer = counter_id;
    
    if (a->reader_lenbuf == 0) {
        if (a->reader_i_parfile > 0) {
            close_parfile(a);
        }
        if (a->reader_i_parfile < a->n_np) {
            s = open_parfile(a, a->reader_i_parfile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->reader_i_parfile++;
        } else {
            // No more input file to read.
            alder_kmer_counter_unlock_reader(a);
            return (int)a->n_subbuf;
        }
    }
    
    /* Read a block of input buffer. */
    // Replaced by the next following one: see goshngbuffer.
#if 0
    uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
    uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER3_INBUFFER_BODY;
    size_t size_body = a->size_subinbuf - ALDER_KMER_COUNTER3_INBUFFER_BODY;
    a->reader_lenbuf = fread(inbuf_body, sizeof(*inbuf_body), size_body, a->fpin);
    
    if (a->reader_lenbuf == 0) {
        if (a->reader_i_parfile > 0) {
            close_parfile(a);
        }
        if (a->reader_i_parfile < a->n_np) {
            s = open_parfile(a, a->reader_i_parfile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->reader_i_parfile++;
            a->reader_lenbuf = fread(inbuf_body, sizeof(*inbuf_body),
                                     size_body, a->fpin);
        } else {
            // No more input file to read.
            alder_kmer_counter_unlock_reader(a);
            return (int)a->n_subbuf;
        }
    }
    assert(a->reader_i_parfile <= a->n_np);
    assert(a->reader_lenbuf > 0);
#endif
    
#if 1
    size_t lenbuf = 0;
    // goshngbuffer
    uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
    uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER3_INBUFFER_BODY;
    size_t cur_next_lenbuf = a->next_lenbuf;
    if (a->next_lenbuf > 0) {
        memcpy(inbuf_body, a->next_inbuf, a->next_lenbuf);
        inbuf_body += a->next_lenbuf;
    }
    lenbuf = fread(inbuf_body, sizeof(*inbuf_body), BUFSIZE, a->fpin);
    assert(lenbuf == 0 || lenbuf >= a->b);
    if (lenbuf > 0 && a->b > 1) {
        assert(cur_next_lenbuf + lenbuf >= a->b);
        a->next_lenbuf = (cur_next_lenbuf + lenbuf) % a->b;
        memcpy(a->next_inbuf, inbuf_body + lenbuf - a->next_lenbuf, a->next_lenbuf);
        a->reader_lenbuf = cur_next_lenbuf + lenbuf - a->next_lenbuf;
    } else {
        if (lenbuf == 0) {
            assert(cur_next_lenbuf == 0);
        }
        a->reader_lenbuf = lenbuf;
    }
    
    if (a->reader_lenbuf == 0) {
        assert(a->next_lenbuf == 0);
        if (a->reader_i_parfile > 0) {
            close_parfile(a);
        }
        if (a->reader_i_parfile < a->n_np) {
            s = open_parfile(a, a->reader_i_parfile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->reader_i_parfile++;
            lenbuf = fread(inbuf_body, sizeof(*inbuf_body), BUFSIZE, a->fpin);
            assert(lenbuf == 0 || lenbuf >= a->b);
            if (lenbuf > 0 && a->b > 1) {
                assert(cur_next_lenbuf + lenbuf >= a->b);
                a->next_lenbuf = (cur_next_lenbuf + lenbuf) % a->b;
                memcpy(a->next_inbuf, inbuf_body + lenbuf - a->next_lenbuf, a->next_lenbuf);
                a->reader_lenbuf = cur_next_lenbuf + lenbuf - a->next_lenbuf;
            } else {
                if (lenbuf == 0) {
                    assert(cur_next_lenbuf == 0);
                }
                a->reader_lenbuf = lenbuf;
            }
            
        } else {
            // No more input file to read.
            alder_kmer_counter_unlock_reader(a);
            return (int)a->n_subbuf;
        }
    }
    assert(a->reader_i_parfile <= a->n_np);
    assert(a->reader_lenbuf > 0);
#endif
    
    to_uint64(inbuf,ALDER_KMER_COUNTER3_INBUFFER_I_NP) = a->reader_i_parfile - 1;
    to_size(inbuf,ALDER_KMER_COUNTER3_INBUFFER_LENGTH) = a->reader_lenbuf;
    assert(a->reader_lenbuf % a->b == 0);
    
    /* Progress */
    a->n_byte += lenbuf;
    if (a->progress_flag) {
        alder_progress_step(a->n_byte, a->totalFilesize,
                            a->progressToError_flag);
    }
    
    alder_kmer_counter_unlock_reader(a);
    return counter_id;
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
    alder_kmer_thread3_t *a = (alder_kmer_thread3_t*)t;
    assert(a != NULL);
    
    /* Numbers for decoding kmers. */
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(a->k);
    alder_encode_number2_t *m2 = alder_encode_number2_createWithKmer(a->k);
    uint64_t *res_key = malloc(sizeof(*res_key) * m->s);
    if (m == NULL || m2 == NULL || res_key == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "Fatal - counter(): not enough memory");
        pthread_exit(NULL);
    }
    memset(res_key, 0, sizeof(*res_key) * m->s);
    size_t ib = m2->b / 8;
    size_t jb = m2->b % 8;
    
    /* Let's read inputs, and skip writing table files. */
    size_t c_inbuffer = a->n_subbuf;
    size_t c_table = a->n_ht;
    uint64_t i_np = a->n_np;
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_table < a->n_ht) {
            c_table = write_tabfile(a, c_table, i_np);
        }
        
        /* Check if this is the final. */
        if (a->n_blockByReader[a->n_np-1] == a->n_blockByCounter[a->n_np-1]) {
            /* Exit! */
            break;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
//        assert(c_inbuffer == a->n_subbuf);
        c_inbuffer = read_parfile(a, counter_id);
        if (c_inbuffer == a->n_subbuf) {
            /* Exit! */
//            assert(a->n_blockByReader[a->n_np-1] == a->n_blockByCounter[a->n_np-1]);
#if defined(ALDER_THREAD)
            alder_log("counter(%d): BREAK! lock(%d).", counter_id, a->lock_reader);
#endif
            break;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Counter
        ///////////////////////////////////////////////////////////////////////
        assert(c_inbuffer < a->n_subbuf);
        assert(c_inbuffer == counter_id);
        assert(c_table == a->n_ht);
        
        /* setup of inbuf */
        uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
        i_np = to_uint64(inbuf,ALDER_KMER_COUNTER3_INBUFFER_I_NP);
        size_t lenbuf = to_size(inbuf,ALDER_KMER_COUNTER3_INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER3_INBUFFER_BODY;
        /* setup of table */
        alder_hashtable_mcas_t *c_ht = NULL;
        
        ///////////////////////////////////////////////////////////////////////
        //                    BUG: race condition?
        ///////////////////////////////////////////////////////////////////////
        assert(lenbuf > 0);
        if (a->main_i_np == i_np || a->main_i_np + 1 == i_np) {
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
//                alder_log2("main_i_np: %d, i_np", a->main_i_np, i_np);
                
                alder_loge3(ALDER_ERR_THREAD,
                            "WARN: partition files are too small, "
                            "to many threads, some threads are sleeping...zzz");
//                sleep(1);
            }
            assert(a->main_i_np == i_np || a->main_i_np + 1 == i_np);
            c_table = i_np % 2;
        } else {
            assert(a->n_counter > 1);
            alder_loge(ALDER_ERR, "Fatal! "
                       "The current partition must have been processed.");
            assert(0);
            abort();
        }
        assert(c_table < a->n_ht);
        c_ht = a->ht[c_table];
        assert(lenbuf % m2->b == 0);
        uint64_t n_kmer = lenbuf / m2->b;
        size_t x = 0;
        for (uint64_t i_kmer = 0; i_kmer < n_kmer; i_kmer++) {
            
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            /* Decode a kmer. */
            uint64_t *inbuf_body_uint64 = (uint64_t*)(inbuf_body + x);
            for (size_t i = 0; i < ib; i++) {
                m->n[i] = inbuf_body_uint64[i];
                x += 8;
            }
            for (size_t j = 0; j < jb; j++) {
                m2->n[ib].key8[j] = inbuf_body[x++];
            }
            m->n[ib] = m2->n[ib].key64;
            
//            for (size_t i = 0; i < ib; i++) {
//                for (size_t j = 0; j < 8; j++) {
//                    m2->n[i].key8[j] = inbuf_body[x++];
//                }
//            }
//            for (size_t j = 0; j < jb; j++) {
//                m2->n[ib].key8[j] = inbuf_body[x++];
//            }
//            for (int i = 0; i < m->s; i++) {
//                m->n[i] = m2->n[i].key64;
//            }
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            
            s = alder_hashtable_mcas_increment(c_ht, m->n, res_key,
                                               a->isMultithreaded);
            
            if (s != ALDER_STATUS_SUCCESS) {
                alder_loge(ALDER_ERR, "Fatal - table is full!");
                assert(0);
                abort();
            }
        }
        assert(x == lenbuf);
        
        if (lenbuf > 0) {
            alder_kmer_counter_increment_n_block(a, i_np);
        }
        
        // Will need a new block of input buffer.
        //            c_inbuffer = a->n_subbuf;
        
        if (a->n_blockByCounter[i_np] == a->n_blockByReader[i_np]) {
            // No code.
        } else {
            // Not the last bock, so won't write to a table file.
            c_table = a->n_ht;
        }
        
    }
    XFREE(res_key);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(m2);
    alder_log("counter(%d): END", counter_id);
    pthread_exit(NULL);
}


