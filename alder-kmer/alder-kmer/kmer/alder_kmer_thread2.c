/**
 * This file, alder_kmer_thread2.c, is part of alder-kmer.
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
#include "alder_kmer_thread2.h"

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

#define ALDER_KMER_COUNTER2_NO_INPUT  2
#define ALDER_KMER_COUNTER2_ONE_INPUT 3


#define ALDER_KMER_COUNTER2_INBUFFER_I_NP        1
#define ALDER_KMER_COUNTER2_INBUFFER_LENGTH      9
#define ALDER_KMER_COUNTER2_INBUFFER_BODY        17


static int compute_offset_parfile(size_t *offset_parfile, const char *fn, alder_kmer_thread2_t *o, int counter_id);
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
filename_partition(alder_kmer_thread2_t *o, uint64_t i_np)
{
    /* File name setup */
    bstring bfpar = NULL;
    if (o->infile == NULL) {
        bfpar = bformat("%s/%s-%llu-%llu.par",
                        bdata(o->boutdir), bdata(o->boutfile),
                        o->i_ni, i_np);
        if (bfpar == NULL) return NULL;
    } else {
        int i_infile = ((int)i_np + (int)(o->i_ni * o->n_np));
        bfpar = bstrcpy(o->infile->entry[i_infile]);
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
compute_number_block(alder_kmer_thread2_t *o, uint64_t i_np)
{
    o->n_blockByReader[i_np] = 0;
    bstring bfpar = filename_partition(o, i_np);
    if (bfpar == NULL) {
        return ALDER_STATUS_ERROR;
    }
    /* Compute the number of buffer blocks in the file. */
    size_t file_size = 0;
    alder_file_size(bdata(bfpar), &file_size);
    
//    assert(file_size % o->size_readbuffer == 0);
//    size_t n_block = ALDER_BYTESIZE_KMER(file_size,o->size_readbuffer);
//    size_t n_i_block = ALDER_BYTESIZE_KMER(n_block,o->n_counter);
//    o->n_t_byte_not_last = n_i_block * o->size_readbuffer;
//    o->n_t_byte_last = file_size - (n_i_block * (o->n_counter - 1)) * o->size_readbuffer;
//    alder_log3("parfile size: %zu", file_size);
//    alder_log3("n block:      %zu", n_block);
//    alder_log3("n_i_block:    %zu", n_i_block);
//    for (int i = 0; i < o->n_counter; i++) {
//        size_t offset_binfile = i * n_i_block * o->size_readbuffer;
//        alder_log3("offset (%d): %zu", i, offset_binfile);
//    }
    
    o->n_blockByReader[i_np] = (int)ALDER_BYTESIZE_KMER(file_size,o->size_readbuffer);
    bdestroy(bfpar);
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function computes the file size that the first thread should deal with.
 *
 *  @param o counter
 *
 *  @return SUCCESS or FAIL
 */
//static int
//compute_parfilesize_first(alder_kmer_thread2_t *o)
//{
//    int s;
//    o->totalFilesize = 0;
//    for (int i_np = 0; i_np < o->n_np; i_np++) {
//        bstring bfpar = filename_partition(o, i_np);
//        if (bfpar == NULL) {
//            return ALDER_STATUS_ERROR;
//        }
//        /* Compute the number of buffer blocks in the file. */
//        size_t file_size = 0;
//        if (o->n_counter == 1) {
//            alder_file_size(bdata(bfpar), &file_size);
//        } else {
//            s = compute_offset_parfile(&file_size, bdata(bfpar), o, 1);
//            if (s != ALDER_STATUS_SUCCESS) {
//                alder_loge(ALDER_ERR, "failed to compute parfile: %s",
//                           bdata(bfpar));
//                bdestroy(bfpar);
//                return s;
//            }
//        }
//        o->totalFilesize += file_size;
//        bdestroy(bfpar);
//    }
//    return ALDER_STATUS_SUCCESS;
//}



static void
alder_kmer_thread2_destroy(alder_kmer_thread2_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
        for (int i = 0; i < o->n_counter; i++) {
            //            XFCLOSE(o->fpin[i]);
            XFREE(o->next_inbuf[i]);
        }
        XFREE(o->next_inbuf);
        for (size_t i = 0; i < o->n_ht; i++) {
            alder_hashtable_mcas_destroy(o->ht[i]);
        }
        XFREE(o->ht);
        XFREE(o->inbuf);
        XFREE(o->n_blockByReader);
        XFREE(o->n_blockByCounter);
        XFREE(o->next_lenbuf);
        XFREE(o->n_i_byte);
        XFREE(o->n_t_byte);
        XFREE(o->reader_i_parfile);
        XFREE(o->reader_lenbuf);
        bdestroy(o->boutdir);
        bdestroy(o->boutfile);
        XFREE(o);
    }
}

static alder_kmer_thread2_t *
alder_kmer_thread2_create(FILE *fpout,
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
                          size_t n_total_kmer,
                          size_t n_current_kmer,
                          int progress_flag,
                          int progressToError_flag,
                          int nopack_flag,
                          struct bstrList *infile,
                          const char *outdir,
                          const char *outfile)
{
    alder_log5("Creating alder_kmer_thread2_t...");
    alder_kmer_thread2_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create alder_kmer_thread2_t");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    /* Adjust read buffer size to fit k-mer bytes. */
    int b_kmer = alder_encode_number2_bytesize(kmer_size);
    o->size_readbuffer = sizeInbuffer / b_kmer * b_kmer;
    o->size_writebuffer = sizeOutbuffer;
    o->k = kmer_size;
    o->b = b_kmer;
    o->i_ni = i_ni;
    o->n_ni = n_ni;
    o->n_np = n_np;
    o->n_counter = n_counter;
    o->next_inbuf = NULL;
    o->next_lenbuf = NULL;
    o->reader_lenbuf = NULL;
    o->reader_i_parfile = NULL;
    o->size_subinbuf = ALDER_KMER_COUNTER2_INBUFFER_BODY + o->size_readbuffer + o->b;
    o->size_inbuf = o->size_subinbuf * n_counter;
    o->inbuf = NULL;
    o->n_blockByReader = NULL;
    o->n_blockByCounter = NULL;
    assert(n_counter > 0);
    o->n_ht = 1;
    
    o->main_i_np = 0;
    o->main_table = 0;
    o->ht = NULL;
    o->infile = infile;
    o->fpout = fpout;
    o->boutfile = NULL;
    o->boutdir = NULL;
    o->fpin = NULL;
    o->n_t_byte = NULL;
    o->n_i_byte = NULL;
    o->n_i_kmer = NULL;
    o->n_byte = n_byte;
    o->n_kmer = 0;
    o->n_hash = 0;
    o->n_total_kmer = n_total_kmer;
    o->n_current_kmer = n_current_kmer;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->nopack_flag = nopack_flag;
    o->isMultithreaded = (n_counter > 1);
    o->status = 0;
    
    /* memory */
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->fpin = malloc(sizeof(*o->fpin) * n_counter);
    o->n_t_byte = malloc(sizeof(*o->n_t_byte) * n_counter);
    o->n_i_byte = malloc(sizeof(*o->n_i_byte) * n_counter);
    o->n_i_kmer = malloc(sizeof(*o->n_i_kmer) * n_counter);
    o->next_lenbuf = malloc(sizeof(*o->next_lenbuf) * n_counter);
    o->reader_lenbuf = malloc(sizeof(*o->reader_lenbuf) * n_counter);
    o->reader_i_parfile = malloc(sizeof(*o->reader_i_parfile) * n_counter);
    o->n_blockByReader = malloc(sizeof(*o->n_blockByReader) * (n_np+1));
    o->n_blockByCounter = malloc(sizeof(*o->n_blockByCounter) * (n_np+1));
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf); /* bigmem */
    o->next_inbuf = malloc(sizeof(*o->next_inbuf) * n_counter);// o->b);
    o->ht = malloc(sizeof(*o->ht) * o->n_ht);
    if (o->boutfile == NULL || o->boutdir == NULL ||
        o->n_i_kmer == NULL || o->n_i_byte == NULL || o->n_t_byte == NULL ||
        o->next_lenbuf == NULL || o->fpin == NULL ||
        o->reader_lenbuf == NULL || o->reader_i_parfile == NULL ||
        o->n_blockByReader == NULL || o->n_blockByCounter == NULL ||
        o->inbuf == NULL || o->next_inbuf == NULL || o->ht == NULL) {
        alder_kmer_thread2_destroy(o);
        return NULL;
    }
    memset(o->fpin, 0, sizeof(*o->fpin) * n_counter);
    memset(o->n_t_byte, 0, sizeof(*o->n_t_byte) * n_counter);
    memset(o->n_i_byte, 0, sizeof(*o->n_i_byte) * n_counter);
    memset(o->n_i_kmer, 0, sizeof(*o->n_i_kmer) * n_counter);
    memset(o->next_lenbuf, 0, sizeof(*o->next_lenbuf) * n_counter);
    memset(o->reader_lenbuf, 0, sizeof(*o->reader_lenbuf) * n_counter);
    memset(o->reader_i_parfile, 0, sizeof(*o->reader_i_parfile) * n_counter);
    memset(o->n_blockByReader, 0, sizeof(*o->n_blockByReader) * (n_np+1));
    memset(o->n_blockByCounter, 0, sizeof(*o->n_blockByCounter) * (n_np+1));
    memset(o->inbuf,0,sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->next_inbuf,0,sizeof(*o->next_inbuf) * n_counter); // * o->b);
    memset(o->ht,0,sizeof(*o->ht) * o->n_ht);
    
    for (int i = 0; i < n_counter; i++) {
        o->next_inbuf[i] = malloc(sizeof(*(o->next_inbuf[i])) * o->b);
        if (o->next_inbuf[i] == NULL) {
            alder_kmer_thread2_destroy(o);
            return NULL;
        }
        memset(o->next_inbuf[i], 0, sizeof(*(o->next_inbuf[i])) * o->b);
    }
    
    for (size_t i = 0; i < o->n_ht; i++) {
        o->ht[i] = alder_hashtable_mcas_create(kmer_size, n_nh); /* bigmem */
        o->ht[i]->i_np = i;
        o->ht[i]->state = ALDER_HASHTABLE_MCAS_STATE_UNFINISHED;
    }
    
    /* Compute the number of input buffer blocks. */
    o->n_blockByReader[n_np] = 0;
    for (uint64_t i_np = 0; i_np < o->n_np; i_np++) {
        int s = compute_number_block(o, i_np);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR_THREAD,
                       "faild to open a partition file");
        }
        o->n_blockByReader[n_np] += o->n_blockByReader[i_np];
    }
//    compute_parfilesize_first(o);
    
    alder_log5("Finish - Creating alder_kmer_kmerrw_t...");
    return o;
}

#pragma mark local

static int close_parfile (alder_kmer_thread2_t *o, int counter_id)
{
    assert(o != NULL);
    XFCLOSE(o->fpin[counter_id]);
//    alder_file_rm();
    
    
    
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function find the offset in a partition file for each counter.
 *
 *  @param offset_parfile [return] offset
 *  @param fn             partition file name
 *  @param o              counter
 *  @param counter_id     counter id
 *
 *  @return SUCCESS if each counter has input.
 *          ALDER_KMER_COUNTER2_NO_INPUT if parfile is zero.
 *          ALDER_KMER_COUNTER2_ONE_INPUT if only one counter will have input.
 */
static int compute_offset_parfile(size_t *offset_parfile, const char *fn, alder_kmer_thread2_t *o, int counter_id)
{
    // Find the offset, and move the file pointer to the offset.
    int s = ALDER_STATUS_SUCCESS;
    size_t parfilesize = 0;
    alder_file_size(fn, &parfilesize);
    size_t n_block = ALDER_BYTESIZE_KMER(parfilesize,o->size_readbuffer);
    size_t n_i_block = n_block / o->n_counter;  // ALDER_BYTESIZE_KMER(n_block,o->n_counter);
//    if (n_i_block == 0) {
//        alder_loge(ALDER_ERR, "Too small data; use larger data or use a single thread");
//        return ALDER_STATUS_ERROR;
//    }
    
    o->n_t_byte_not_last = n_i_block * o->size_readbuffer;
    o->n_t_byte_last = parfilesize - (n_i_block * (o->n_counter - 1)) * o->size_readbuffer;
    
    if (n_i_block > 0) {
        assert(parfilesize > (n_i_block * (o->n_counter - 1)) * o->size_readbuffer);
        *offset_parfile = counter_id * n_i_block * o->size_readbuffer;
        s = ALDER_STATUS_SUCCESS;
    } else if (parfilesize == 0) {
        assert(o->n_t_byte_not_last == 0);
        assert(o->n_t_byte_last == 0);
        *offset_parfile = SIZE_T_MAX;
        s = ALDER_KMER_COUNTER2_NO_INPUT;
    } else {
        assert(o->n_t_byte_not_last == 0);
        assert(o->n_t_byte_last > 0);
        *offset_parfile = 0;
        s = ALDER_KMER_COUNTER2_ONE_INPUT;
    }
    return s;
}

/**
 *  This function opens a partition file. It also computes the number of
 *  input buffer blocks for identifying a final buffer block.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return SUCCESS or FAIL or ALDER_KMER_COUNTER2_NOINPUT (2)
 */
static int
open_parfile(alder_kmer_thread2_t *o, int counter_id)
{
    uint64_t i_np = o->reader_i_parfile[counter_id];
    
    bstring bfpar = filename_partition(o, i_np);
    if (bfpar == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    /* Open a file */
    assert(o->fpin[counter_id] == NULL);
    alder_log3("reader(): openning a partition file %s", bdata(bfpar));
    o->fpin[counter_id] = fopen(bdata(bfpar), "rb");
//    fcntl(fileno(o->fpin[counter_id]), F_RDAHEAD, 1); // FIXME: profile it.
    if (o->fpin[counter_id] == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to open the input file %s - %s",
                   bdata(bfpar), strerror(errno));
        bdestroy(bfpar);
        return ALDER_STATUS_ERROR;
    }
    /* Find the offset and move to the offset. */
    size_t offset_parfile = 0;
    int s = compute_offset_parfile(&offset_parfile, bdata(bfpar), o, counter_id);
//    if (s != ALDER_STATUS_SUCCESS) {
//        alder_loge(ALDER_ERR, "failed to open parfile: %s",
//                   bdata(bfpar));
//        bdestroy(bfpar);
//        return s;
//    }
    if (s == ALDER_STATUS_SUCCESS) {
        lseek(fileno(o->fpin[counter_id]), (off_t)offset_parfile, SEEK_SET);
    } else if (s == ALDER_KMER_COUNTER2_NO_INPUT) {
        assert(offset_parfile == SIZE_T_MAX);
        // No code.
    } else if (s == ALDER_KMER_COUNTER2_ONE_INPUT && counter_id == o->n_counter - 1) {
        assert(offset_parfile == 0);
        lseek(fileno(o->fpin[counter_id]), (off_t)offset_parfile, SEEK_SET);
    } else if (s == ALDER_KMER_COUNTER2_ONE_INPUT && counter_id < o->n_counter - 1) {
        assert(offset_parfile == 0);
        // No code.
    } else {
        assert(0);
        abort();
    }
    bdestroy(bfpar);
    return s;
}


#pragma mark main

/**
 *  This function implements a version of counter with separte partition files
 *  for each thread.
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
int alder_kmer_count_iteration2(FILE *fpout,
                                int n_counter,
                                int i_ni,
                                int kmer_size,
                                long memory_available,
                                long sizeInbuffer,
                                long sizeOutbuffer,
                                uint64_t n_ni,
                                uint64_t n_np,
                                size_t n_nh,
                                size_t totalfilesize,
                                size_t *n_byte,
                                size_t *n_kmer,
                                size_t *n_hash,
                                size_t n_total_kmer,
                                size_t *n_current_kmer,
                                int progress_flag,
                                int progressToError_flag,
                                int nopack_flag,
                                uint8_t *inbuf,
                                size_t size_data,
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
    
    alder_kmer_thread2_t *data = alder_kmer_thread2_create(fpout,
                                                           n_counter,
                                                           i_ni,
                                                           kmer_size,
                                                           memory_available,
                                                           sizeInbuffer,
                                                           sizeOutbuffer,
                                                           n_ni,
                                                           n_np,
                                                           n_nh,
                                                           totalfilesize,
                                                           *n_byte,
                                                           n_total_kmer,
                                                           *n_current_kmer,
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
        alder_kmer_thread2_destroy(data);
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
    
    /* Cleanup */
    pthread_attr_destroy(&attr);
    XFREE(threads);
    
    data->n_byte  = 0;
    for (int i = 0; i < n_counter; i++) {
        data->n_byte += data->n_i_byte[i];
    }
    data->n_kmer = 0;
    for (int i = 0; i < n_counter; i++) {
        data->n_kmer += data->n_i_kmer[i];
    }
    alder_log2("counter()[%d] Kmers: %zu", i_ni, data->n_kmer);
    alder_log2("counter()[%d) Bytes: %zu", i_ni, data->n_byte);
    *n_byte += data->n_byte;
    *n_kmer += data->n_kmer;
    *n_hash = data->n_hash;
    *n_current_kmer += data->n_kmer;
    alder_kmer_thread2_destroy(data);
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
write_tabfile(alder_kmer_thread2_t *a, size_t c_table, uint64_t i_np)
{
    int s;
    assert(a->main_i_np == i_np);
    assert(c_table == 0);
    
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
    /* Print the number of elements in the current partition. */
    s = alder_hashtable_mcas_printCountPerPartitionFD(fileno(a->fpout),
                                                      s_count,
                                                      a->i_ni,
                                                      i_np,
                                                      a->n_np);
    assert(s == ALDER_STATUS_SUCCESS);
    
    /* Reset the table. */
    alder_hashtable_mcas_reset(a->ht[c_table]);
    
    /* Change the main i_np; all of the waiting threads will restart. */
    a->main_i_np++;
    alder_log3("main_i_np++: %zu", a->main_i_np);
    
    if (a->progress_flag) {
        size_t c = a->n_current_kmer;
        for (int i = 0; i < a->n_counter; i++) {
            c += a->n_i_kmer[i];
        }
        alder_progress_step(c, a->n_total_kmer, a->progressToError_flag);
    }
    
    return a->n_ht;
}

/**
 *  This function reads a block of partition files.
 *
 *  @param a          counter
 *  @param counter_id counter id
 *
 *  @return counter_id if there is more to read in a partition, otherise 
 *          n_counter
 */
static __attribute__ ((noinline)) int
read_parfile(alder_kmer_thread2_t *a, int counter_id, int is_last_read)
{
    int s;
    //    alder_kmer_counter4_lock_reader(a);
    
    if (a->reader_lenbuf[counter_id] == 0) {
        if (a->reader_i_parfile[counter_id] > 0) {
            close_parfile(a, counter_id);
        }
        if (a->reader_i_parfile[counter_id] < a->n_np) {
            s = open_parfile(a, counter_id);
            assert(s != ALDER_STATUS_ERROR);
            a->reader_i_parfile[counter_id]++;
            
            if ((s == ALDER_KMER_COUNTER2_NO_INPUT) ||
                       (s == ALDER_KMER_COUNTER2_ONE_INPUT && counter_id < a->n_counter - 1)) {
                uint8_t *inbuf = a->inbuf + counter_id * a->size_subinbuf;
                to_uint64(inbuf,ALDER_KMER_COUNTER2_INBUFFER_I_NP) = a->reader_i_parfile[counter_id] - 1;
                to_size(inbuf,ALDER_KMER_COUNTER2_INBUFFER_LENGTH) = 0;
                a->reader_lenbuf[counter_id] = 0;
                return a->n_counter;
            }
        } else {
            // No more input file to read.
            //            alder_kmer_counter4_unlock_reader(a);
            return a->n_counter;
        }
    }
    
    /* Read a block of input buffer. */
    ssize_t lenbuf = 0;
    // goshngbuffer
    uint8_t *inbuf = a->inbuf + counter_id * a->size_subinbuf;
    uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER2_INBUFFER_BODY;
    size_t cur_next_lenbuf = a->next_lenbuf[counter_id];
    if (a->next_lenbuf[counter_id] > 0) {
        memcpy(inbuf_body, a->next_inbuf[counter_id], a->next_lenbuf[counter_id]);
        inbuf_body += a->next_lenbuf[counter_id];
    }
    lenbuf = read(fileno(a->fpin[counter_id]), inbuf_body, a->size_readbuffer);
    assert(lenbuf == 0 || lenbuf >= a->b);
    if (lenbuf > 0 && a->b > 1) {
        assert(cur_next_lenbuf + lenbuf >= a->b);
        a->next_lenbuf[counter_id] = (cur_next_lenbuf + lenbuf) % a->b;
        memcpy(a->next_inbuf[counter_id],
               inbuf_body + lenbuf - a->next_lenbuf[counter_id],
               a->next_lenbuf[counter_id]);
        a->reader_lenbuf[counter_id] = cur_next_lenbuf + lenbuf - a->next_lenbuf[counter_id];
    } else if (lenbuf > 0 && a->b == 1) {
        a->next_lenbuf[counter_id] = 0;
        a->reader_lenbuf[counter_id] = lenbuf;
    } else if (lenbuf == 0) {
        assert(cur_next_lenbuf == 0);
        a->reader_lenbuf[counter_id] = lenbuf;
    } else {
        // Error in reading.
        assert(0);
        abort();
    }
    
    assert(a->reader_i_parfile[counter_id] <= a->n_np);
    
    to_uint64(inbuf,ALDER_KMER_COUNTER2_INBUFFER_I_NP) = a->reader_i_parfile[counter_id] - 1;
    to_size(inbuf,ALDER_KMER_COUNTER2_INBUFFER_LENGTH) = a->reader_lenbuf[counter_id];
    assert(a->reader_lenbuf[counter_id] % a->b == 0);
    
    /* Progress */
    a->n_t_byte[counter_id] += lenbuf;
    if (counter_id < a->n_counter - 1) {
        // No the last one
        if (a->n_t_byte[counter_id] == a->n_t_byte_not_last) {
            // The last read.
            a->n_i_byte[counter_id] += a->n_t_byte[counter_id];
            a->n_t_byte[counter_id] = 0;
            return a->n_counter;
        }
    } else {
        if (a->n_t_byte[counter_id] == a->n_t_byte_last) {
            // The last read.
            a->n_i_byte[counter_id] += a->n_t_byte[counter_id];
            a->n_t_byte[counter_id] = 0;
            return a->n_counter;
        }
    }
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
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_thread2_t *a = (alder_kmer_thread2_t*)t;
    assert(a != NULL);
    alder_log2("counter(%d)[%llu]: START", counter_id, a->i_ni);
    
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
    
//    size_t debug_counter = 0;
    
    /* Let's read inputs, and skip writing table files. */
    size_t c_table = a->n_ht;
    uint64_t i_np = a->n_np;
    int is_last_read = 0;
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_table < a->n_ht) {
            c_table = write_tabfile(a, c_table, i_np);
        }
        
        /* Check if this is the final. */
        if (a->n_blockByReader[a->n_np] == a->n_blockByCounter[a->n_np]) {
            /* Exit! */
            break;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
        // Block if this is the last thread.
        while (is_last_read == 1 && a->main_i_np == i_np) {
//            sleep(1);
            asm("");
        }
        if (is_last_read == 1 && a->main_i_np == a->n_np) {
            break;
        }
        int w_counter = read_parfile(a, counter_id, is_last_read);
        if (w_counter == a->n_counter) {
            is_last_read = 1;
        } else {
            is_last_read = 0;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Counter
        ///////////////////////////////////////////////////////////////////////
        assert(c_table == a->n_ht);
        
        /* setup of inbuf */
        uint8_t *inbuf = a->inbuf + counter_id * a->size_subinbuf;
        i_np = to_uint64(inbuf,ALDER_KMER_COUNTER2_INBUFFER_I_NP);
        size_t lenbuf = to_size(inbuf,ALDER_KMER_COUNTER2_INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER2_INBUFFER_BODY;
        if (lenbuf > 0) {
            assert(a->main_i_np == i_np);
        }
        /* setup of table */
        alder_hashtable_mcas_t *c_ht = NULL;
        c_table = 0;
        
        assert(c_table < a->n_ht);
        c_ht = a->ht[c_table];
        assert(lenbuf % m2->b == 0);
        uint64_t n_kmer = lenbuf / m2->b;
        size_t x = 0;
        
        for (uint64_t i_kmer = 0; i_kmer < n_kmer; i_kmer++) {
            a->n_i_kmer[counter_id]++;
            
//            debug_counter++;
            
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            /* Decode a kmer. */
            uint64_t *inbuf_body_uint64 = (uint64_t*)(inbuf_body + x);
            for (size_t i = 0; i < ib; i++) {
                m->n[i] = inbuf_body_uint64[i];
                x += 8;
            }
            if (jb > 0) {
                for (size_t j = 0; j < jb; j++) {
                    m2->n[ib].key8[j] = inbuf_body[x++];
                }
                m->n[ib] = m2->n[ib].key64;
            }
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
            alder_kmer_counter2_increment_n_block(a, i_np);
            
            if (a->n_blockByCounter[i_np] == a->n_blockByReader[i_np]) {
                // No code.
                a->n_blockByCounter[a->n_np] += a->n_blockByCounter[i_np];
            } else {
                // Not the last bock, so won't write to a table file.
                c_table = a->n_ht;
            }
        } else {
            c_table = a->n_ht;
        }
        if (is_last_read == 1) {
            a->reader_lenbuf[counter_id] = 0;
        }
        
        /* When parfile is zero... */
        if (a->n_blockByReader[i_np] == 0 && counter_id == 0) {
            a->main_i_np++;
        }
    }
    XFREE(res_key);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(m2);
    alder_log2("counter(%d)[%llu]: END", counter_id, a->i_ni);
    
    //    printf("debug counter: %zu\n",debug_counter++);
    pthread_exit(NULL);
}


