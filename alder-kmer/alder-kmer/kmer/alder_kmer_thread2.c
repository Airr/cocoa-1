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

/**
 *  The state of buffer
 *  -------------------
 *  Initially, inbuf is EMPTY.
 *  1. Reader can change the input buffer state:
 *     EMPTY -> USE           by select_empty_inbuf
 *     USE   -> FULL          by deselect_full_inbuf
 *     USE   -> EMPTY         by deselect_empty_inbuf
 *     EMPTY -> FINISHED      by finish_empty_inbuf
 *  2. Writer can change the talbe state:
 *     FINISHED -> LOCKED     by select_finished_table
 *     LOCKED   -> UNFINISHED by deselect_empty_table
 *  3. Counter can change both input buffer and output table:
 *     FULL_INBUF   -> USE    by retain_inbuf
 *     USE  -> EMPTY_INBUF    by release_inbuf
 *     UNFINISHED -> FINISHED by finish_table
 *
 *  Structure of buffers
 *  --------------------
 *  inbuf: There are [n_subbuf]-many sub buffers. The main inbuf does not have
 *         any header. But, each sub buffer does;
 *         state (1B) partition index (8B), and length (8B).
 *         So, the body start at 17B. The number of sub buffers is determined
 *         by the number of threads.
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
 *  2. First, it attempts to choose an input buffer from the main partition.
 *  3. Second, if the firty attempt failed, it would choose one from secondary.
 *  4. Use n_blockByReader and n_blockByCounter to check the final buffer.
 */

#define ALDER_KMER_COUNTER2_INBUFFER_I_NP        1
#define ALDER_KMER_COUNTER2_INBUFFER_LENGTH      9
#define ALDER_KMER_COUNTER2_INBUFFER_BODY        17

enum {
    ALDER_KMER_COUNTER2_EMPTY_INBUF        = 0,
    ALDER_KMER_COUNTER2_READER_USE_INBUF   = 1,
    ALDER_KMER_COUNTER2_FULL_INBUF         = 2,
    ALDER_KMER_COUNTER2_FINISHED_TABLE     = 3,
    ALDER_KMER_COUNTER2_WRITER_USE_TABLE   = 4,
    ALDER_KMER_COUNTER2_EMPTY_TABLE        = 5,
    ALDER_KMER_COUNTER2_COUNTER_USE_INBUF  = 6,
    ALDER_KMER_COUNTER2_FINISHED_INBUF     = 8
};

typedef struct alder_kmer_thread2_struct
alder_kmer_thread2_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_thread2_struct {
    /* info */
    int k;                       /* k - kmer size                            */
    uint64_t i_ni;               /* i-th iteration                           */
    uint64_t n_ni;               /* n_ni - number of iteration               */
    uint64_t n_np;               /* n_np - number of partition               */
    int n_counter;               /* n_counter - number of counter threads    */
    
    /* buffer */
    size_t size_inbuf;           /* size_inbuf - size of the input buffer    */
    size_t n_subbuf;             /* number of divided buffers in inbuf       */
    size_t size_subinbuf;        /* size of each divided buffer              */
    uint8_t *inbuf;              /* inbuf - input buffer                     */
    int *n_blockByReader;        /* [n_np] input buffer blocks               */
    int *n_blockByCounter;       /* [n_np] input buffer blocks               */
    /* outbuf: hash table */
    size_t n_ht;                 /* number of hash tables                    */
    size_t main_i_np;            /* main table                               */
    alder_hashtable_mcas_t **ht; /* (per partition) hash table               */
    
    /* file */
    struct bstrList *infile;     /* (not own) input partition file names     */
    FILE *fpout;                 /* (not own) count table output file pointer*/
    bstring boutfile;            /* outfile name                             */
    bstring boutdir;             /* out directory                            */
    FILE *fpin;                  /* partition input file pointer             */
    
    /* stat */
    size_t n_byte;
    size_t n_kmer;
    size_t n_hash;
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;         /* flag for progress bar                      */
    int progressToError_flag;  /* flag for progress bar                      */
    int nopack_flag;           /* flag for no pack operation                 */
   
    /* flag */
    bool isMultithreaded;      /* are there other counters?                  */
    int status;                /* status of counter's buffers                */
};

static void *reader(void *t);
static void *writer(void *t);
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
compute_number_block(alder_kmer_thread2_t *o, uint64_t i_np)
{
    bstring bfpar = filename_partition(o, i_np);
    if (bfpar == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    /* Compute the number of buffer blocks in the file. */
    size_t file_size = 0;
    alder_file_size(bdata(bfpar), &file_size);
    size_t size_body = o->size_subinbuf - ALDER_KMER_COUNTER2_INBUFFER_BODY;
    o->n_blockByReader[i_np] = (int)ALDER_BYTESIZE_KMER(file_size,size_body);
    
    bdestroy(bfpar);
    return ALDER_STATUS_SUCCESS;
}


static void
alder_kmer_thread2_destroy(alder_kmer_thread2_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
        XFCLOSE(o->fpin);
        for (size_t i = 0; i < o->n_ht; i++) {
            alder_hashtable_mcas_destroy(o->ht[i]);
        }
        XFREE(o->ht);
        XFREE(o->inbuf);
        XFREE(o->n_blockByReader);
        XFREE(o->n_blockByCounter);
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
    alder_log5("Creating alder_kmer_thread2_t...");
    alder_kmer_thread2_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create alder_kmer_thread2_t");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->k = kmer_size;
    o->i_ni = i_ni;
    o->n_ni = n_ni;
    o->n_np = n_np;
    o->n_counter = n_counter;
    o->size_inbuf = o->size_inbuf = (size_t)(memory_available << 19);
    o->n_subbuf = n_counter * 2;
    o->size_subinbuf = o->size_inbuf / o->n_subbuf;
    o->inbuf = NULL;
    o->n_blockByReader = NULL;
    o->n_blockByCounter = NULL;
    o->n_ht = 2;
    o->main_i_np = 0;
    o->ht = NULL;
    o->infile = infile;
    o->fpout = fpout;
    o->boutfile = NULL;
    o->boutdir = NULL;
    o->fpin = NULL;
    o->n_byte = 0;
    o->n_kmer = 0;
    o->n_hash = 0;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->nopack_flag = nopack_flag;
    o->isMultithreaded = (n_counter > 1);
    o->status = 0;
    
    /* adjust size inbuf */
    if (o->size_subinbuf < ALDER_KMER_COUNTER2_INBUFFER_BODY) {
        alder_loge(ALDER_ERR_MEMORY, "failed to create threads for counters.");
        alder_loge(ALDER_ERR_MEMORY, "Too small inbuf size.");
        alder_kmer_thread2_destroy(o);
        return NULL;
    }
    o->size_subinbuf -= ALDER_KMER_COUNTER2_INBUFFER_BODY;
    o->size_subinbuf = alder_encode_number2_adjustBufferSizeForKmer(kmer_size, o->size_subinbuf);
    o->size_subinbuf += ALDER_KMER_COUNTER2_INBUFFER_BODY;
    o->size_inbuf = o->size_subinbuf * o->n_subbuf;
    
    
    /* memory */
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->n_blockByCounter = malloc(sizeof(*o->n_blockByCounter) * n_np);
    o->n_blockByReader = malloc(sizeof(*o->n_blockByReader) * n_np);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);
    o->ht = malloc(sizeof(*o->ht) * o->n_ht);
    if (o->ht == NULL || o->boutdir == NULL || o->inbuf == NULL ||
        o->n_blockByReader == NULL || o->n_blockByCounter == NULL) {
        alder_kmer_thread2_destroy(o);
        return NULL;
    }
    memset(o->inbuf,0,sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->ht,0,sizeof(*o->ht) * o->n_ht);
    memset(o->n_blockByCounter, 0, sizeof(*o->n_blockByCounter) * n_np);
    memset(o->n_blockByReader, 0, sizeof(*o->n_blockByReader) * n_np);
    assert(o->fpin == NULL);
    
    for (size_t i = 0; i < o->n_ht; i++) {
        o->ht[i] = alder_hashtable_mcas_create(kmer_size, n_nh);
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
open_partition(alder_kmer_thread2_t *o, uint64_t i_np)
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
int alder_kmer_count_iteration2(FILE *fpout,
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
    int s = 0;
    if (n_counter > 1 && kmer_size > 31) {
        mcas_init(n_counter);
    }
    int n_thread = n_counter + 2;
    
    alder_kmer_thread2_t *data = alder_kmer_thread2_create(fpout,
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
        alder_kmer_thread2_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    memset(threads,0,sizeof(*threads)*n_thread);
    
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    s += pthread_create(&threads[0], &attr, reader, (void *)data);
    s += pthread_create(&threads[1], &attr, writer, (void *)data);
    for (int i = 0; i < n_counter; i++) {
        s += pthread_create(&threads[i+2], &attr, counter, (void *)data);
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
    alder_kmer_thread2_destroy(data);
    alder_log5("Counting Kmers has been finished with %d threads.", n_thread);
    return 0;
}

#pragma mark buffer

/**
 *  This function attempt to choose an empty input buffer.
 *
 *  @param a          counter
 *  @param c_inbuffer input buffer id
 */
static void
select_empty_inbuf(alder_kmer_thread2_t *a, size_t *c_inbuffer)
{
    *c_inbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        // Check the first int in the buffer.
        uint8_t oval = *(a->inbuf + j * a->size_subinbuf);
        if (oval == ALDER_KMER_COUNTER2_EMPTY_INBUF) {
            uint8_t cval = ALDER_KMER_COUNTER2_READER_USE_INBUF;
            cval = __sync_val_compare_and_swap(a->inbuf + j * a->size_subinbuf,
                                               (uint8_t)oval,
                                               (uint8_t)cval);
            if (cval == oval) {
                *c_inbuffer = j;
                break;
            }
        }
    }
}

/**
 *  This function changes the state of input buffer from USE to FULL.
 *
 *  @param a        counter
 *  @param c_buffer buffer id
 *
 *  @return void
 */
static void
deselect_full_inbuf(alder_kmer_thread2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_COUNTER2_READER_USE_INBUF);
    uint8_t cval = ALDER_KMER_COUNTER2_FULL_INBUF;
    cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function changes the input buffer state from USE to EMPTY. It is 
 *  called by reader when input buffer failed to be filled with any more 
 *  content.
 *
 *  @param a          counter
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
deselect_empty_inbuf(alder_kmer_thread2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_COUNTER2_READER_USE_INBUF);
    uint8_t cval = ALDER_KMER_COUNTER2_EMPTY_INBUF;
    cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function finishes the input buffer.
 *
 *  @param a counter
 *
 *  @return void
 */
static void
finish_empty_inbuf(alder_kmer_thread2_t *a)
{
    int isFinishedInbuf = ALDER_NO;
    while (isFinishedInbuf == ALDER_NO) {
        isFinishedInbuf = ALDER_YES;
        sleep(1);
        for (size_t c_inbuffer = 0; c_inbuffer < a->n_subbuf; c_inbuffer++) {
            size_t pos_subinbuf = c_inbuffer * a->size_subinbuf;
            uint8_t oval = *(a->inbuf + pos_subinbuf);
            
            if (oval == ALDER_KMER_COUNTER2_EMPTY_INBUF) {
                uint8_t cval = ALDER_KMER_COUNTER2_FINISHED_INBUF;
                cval = __sync_val_compare_and_swap(a->inbuf + pos_subinbuf,
                                                   (uint8_t)oval,
                                                   (uint8_t)cval);
                assert(cval == oval);
            } else if (oval == ALDER_KMER_COUNTER2_FINISHED_INBUF) {
                // No code: this inbuf was already in the finished state.
            } else {
                // The inbuf is still used by others.
                isFinishedInbuf = ALDER_NO;
            }
        }
    }
}


/**
 *  This function tests whether all of input buffers were finished.
 *
 *  @param a counter
 *
 *  @return YES or NO
 */
//static int
//check_inbuf_finished(alder_kmer_thread2_t *a)
//{
//    int s = ALDER_YES;
//    for (size_t j = 0; j < a->n_subbuf; j++) {
//        uint8_t oval = *(a->inbuf + j * a->size_subinbuf);
//        if (oval != ALDER_KMER_COUNTER2_FINISHED_INBUF) {
//            s = ALDER_NO;
//        }
//    }
//    return s;
//}

/**
 *  This function selects the finished main table, and locks it.
 *
 *  @param a       counter
 *  @param c_table table id
 *
 *  @return void
 */
static void
select_finished_table(alder_kmer_thread2_t *a, size_t *c_table)
{
    *c_table = a->n_ht;
    for (size_t j = 0; j < a->n_ht; j++) {
        // Check the first int in the buffer.
        int oval = alder_hashtable_mcas_state(a->ht[j]);
        int cval = ALDER_HASHTABLE_MCAS_STATE_LOCKED;
        if (oval == ALDER_HASHTABLE_MCAS_STATE_FINISHED &&
            a->main_i_np == a->ht[j]->i_np) {
            cval = __sync_val_compare_and_swap(&a->ht[j]->state,
                                               (int)oval,
                                               (int)cval);
            if (oval == cval) {
                *c_table = j;
                break;
            }
        }
    }
}

/**
 *  This function changes the table state from LOCKED to UNFINISHED so that 
 *  counter can use it. It also increase the main_i_np by 1.
 *
 *  @param a           counter
 *  @param c_outbuffer table id
 *
 *  @return void
 */
static void
deselect_empty_table(alder_kmer_thread2_t *a, size_t c_table)
{
    int oval = alder_hashtable_mcas_state(a->ht[c_table]);
    assert(oval == ALDER_HASHTABLE_MCAS_STATE_LOCKED);
    assert(a->main_i_np == a->ht[c_table]->i_np);
    int cval = ALDER_HASHTABLE_MCAS_STATE_UNFINISHED;
    cval = __sync_val_compare_and_swap(&a->ht[c_table]->state,
                                       (int)oval,
                                       (int)cval);
    assert(oval == cval);
}

/**
 *  This function check if all of the tables are empty or unfinished.
 *
 *  @param a counter
 *
 *  @return YES or NO
 */
static int
check_table_empty(alder_kmer_thread2_t *a)
{
    int s = ALDER_YES;
    for (size_t j = 0; j < a->n_ht; j++) {
        int oval = alder_hashtable_mcas_state(a->ht[j]);
        if (oval != ALDER_HASHTABLE_MCAS_STATE_UNFINISHED ) {
            s = ALDER_NO;
            break;
        }
    }
    return s;
}

/**
 *  This function changes the buffer state from FULL to USE by ENCODER.
 *
 *  @param a          counter
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
retain_inbuf(alder_kmer_thread2_t *a, size_t *c_inbuffer)
{
    /* Fetch the main partition. */
    uint64_t main_i_np = a->main_i_np;
    uint64_t secondary_i_np = main_i_np + 1;
    
    /* Pick a buffer with the main partition. */
    *c_inbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        size_t pos_subinbuf = j * a->size_subinbuf;
        uint8_t oval = *(a->inbuf + pos_subinbuf);
        if (oval == ALDER_KMER_COUNTER2_FULL_INBUF) {
            uint8_t cval = ALDER_KMER_COUNTER2_COUNTER_USE_INBUF;
            cval = __sync_val_compare_and_swap(a->inbuf + pos_subinbuf,
                                               (uint8_t)oval,
                                               (uint8_t)cval);
            if (cval == oval) {
                uint64_t i_np = to_uint64(a->inbuf + j * a->size_subinbuf,
                                          ALDER_KMER_COUNTER2_INBUFFER_I_NP);
                if (main_i_np == i_np) {
                    *c_inbuffer = j;
                    break;
                } else {
                    oval = ALDER_KMER_COUNTER2_COUNTER_USE_INBUF;
                    cval = ALDER_KMER_COUNTER2_FULL_INBUF;
                    cval = __sync_val_compare_and_swap(a->inbuf + pos_subinbuf,
                                                       (uint8_t)oval,
                                                       (uint8_t)cval);
                    assert(cval == oval);
                }
                
            }
        }
    }
    if (*c_inbuffer < a->n_subbuf) {
        return;
    }
    
    /* Failed to get the primary partition.        */
    /* Pick a buffer with the secondary partition. */
    if (secondary_i_np < a->n_np) {
        *c_inbuffer = a->n_subbuf;
        for (size_t j = 0; j < a->n_subbuf; j++) {
            size_t pos_subinbuf = j * a->size_subinbuf;
            uint8_t oval = *(a->inbuf + pos_subinbuf);
            if (oval == ALDER_KMER_COUNTER2_FULL_INBUF) {
                uint8_t cval = ALDER_KMER_COUNTER2_COUNTER_USE_INBUF;
                cval = __sync_val_compare_and_swap(a->inbuf + pos_subinbuf,
                                                   (uint8_t)oval,
                                                   (uint8_t)cval);
                if (cval == oval) {
                    uint64_t i_np = to_uint64(a->inbuf + pos_subinbuf,
                                              ALDER_KMER_COUNTER2_INBUFFER_I_NP);
                    if (secondary_i_np == i_np) {
                        *c_inbuffer = j;
                        break;
                    }
                } else {
                    oval = ALDER_KMER_COUNTER2_COUNTER_USE_INBUF;
                    cval = ALDER_KMER_COUNTER2_FULL_INBUF;
                    cval = __sync_val_compare_and_swap(a->inbuf + pos_subinbuf,
                                                       (uint8_t)oval,
                                                       (uint8_t)cval);
                    assert(cval == oval);
                    
                }
            }
        }
    } else {
        /* No secondary: main partition is the last one in the itearation. */
        // No code.
    }
}

/**
 *  This funciton is called encoder thread, and releases an input buffer.
 *
 *  @param a          counter
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
release_inbuf(alder_kmer_thread2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_COUNTER2_COUNTER_USE_INBUF);
    uint8_t cval = ALDER_KMER_COUNTER2_EMPTY_INBUF;
    cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function increases the count of blocks by 1 atomically.
 *
 *  @param a    counter
 *  @param i_np partition index
 *
 *  @return void
 */
static void increment_n_block(alder_kmer_thread2_t *a, uint64_t i_np)
{
    int oval = a->n_blockByCounter[i_np];
    int cval = oval + 1;
    while (oval != cval) {
        oval = a->n_blockByCounter[i_np];
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(a->n_blockByCounter + i_np,
                                           (int)oval,
                                           (int)cval);
    }
}

static void finish_table(alder_hashtable_mcas_t *c_ht)
{
    int oval = alder_hashtable_mcas_state(c_ht);
    assert(oval == ALDER_HASHTABLE_MCAS_STATE_UNFINISHED);
    int cval = ALDER_HASHTABLE_MCAS_STATE_FINISHED;
    cval = __sync_val_compare_and_swap(&c_ht->state,
                                       (int)oval,
                                       (int)cval);
    assert(oval == cval);
}

#pragma mark thread

/**
 *  This function is for the reader thread.
 *
 *  @param t data
 *
 *  @return SUCCESS or FAIL
 *
 */
static void *reader(void *t)
{
    alder_log("reader(): START.");
    int s = 0;
    alder_kmer_thread2_t *a = (alder_kmer_thread2_t*)t;
    assert(a != NULL);
    
    alder_encode_number2_t *m2 = alder_encode_number2_createWithKmer(a->k);
    
    /* Read par files until no files are left to read. */
    /* Exit if reading a set of par files.             */
    for (uint64_t i_np = 0; i_np < a->n_np; i_np++) {
        alder_log2("reader(): iteration(%llu) - partition(%llu)",
                   a->i_ni, i_np);
        
        s = open_partition(a, i_np);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR_THREAD,
                       "reader(): faild to open a partition - "
                       "force all threads to exit");
            goto cleanup;
        }
        
        size_t lenbuf = 1;
        while (lenbuf > 0) {
            /* Select an empty input buffer. */
            size_t c_inbuffer = a->n_subbuf;
            while (c_inbuffer == a->n_subbuf) {
                select_empty_inbuf(a, &c_inbuffer);
//                sleep(1);
            }
            
            /* Read a block of input buffer. */
            uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
            uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER2_INBUFFER_BODY;
            size_t size_body = a->size_subinbuf - ALDER_KMER_COUNTER2_INBUFFER_BODY;
            lenbuf = fread(inbuf_body, sizeof(*inbuf_body), size_body, a->fpin);
            to_uint64(inbuf,ALDER_KMER_COUNTER2_INBUFFER_I_NP) = i_np;
            to_size(inbuf,ALDER_KMER_COUNTER2_INBUFFER_LENGTH) = lenbuf;
            
            assert(lenbuf % m2->b == 0);
            
            /* Progress */
            a->n_byte += lenbuf;
            if (a->progress_flag) {
                alder_progress_step(a->n_byte, a->totalFilesize,
                                    a->progressToError_flag);
            }
            
            /* Finish the input buffer. */
            if (lenbuf > 0) {
                deselect_full_inbuf(a, c_inbuffer);
            } else {
                deselect_empty_inbuf(a, c_inbuffer);
            }
        }
        XFCLOSE(a->fpin);
    }
cleanup:
    alder_encode_number2_destroy(m2);
    finish_empty_inbuf(a);
    pthread_exit(NULL);
}

/**
 *  This function runs on the writer's thread.
 *
 *  @param t data
 *
 *  @return void
 */
static void *writer(void *t)
{
    alder_log("writer(): START.");
    int s = 0;
    int isFinishedInput = 0;
    int64_t s_count = 0;
    int64_t s_total = 0;
    alder_kmer_thread2_t *a = (alder_kmer_thread2_t*)t;
    assert(a != NULL);
    
    /* Check if all of the partitions are done.        */
    /* Exit if all of the partition files are done.    */
    while (1) {
        
        size_t c_table = a->n_ht;
        isFinishedInput = ALDER_NO;
        while (c_table == a->n_ht && isFinishedInput == ALDER_NO) {
            select_finished_table(a, &c_table);
            sleep(1);
            if (a->n_blockByReader[a->n_np-1] ==
                a->n_blockByCounter[a->n_np-1]) {
                isFinishedInput = ALDER_YES;
            }
        }
        
        if (c_table < a->n_ht) {
            assert(a->ht[c_table]->i_np == a->main_i_np);
            s_count = alder_hashtable_mcas_printPackToFILE(a->ht[c_table],
                                                            a->fpout);
            if (s_count < 0) {
                alder_loge(ALDER_ERR_HASH,
                           "cannot save a hash table (%llu-%llu)",
                           a->i_ni, a->main_i_np);
            }
            s_total += s_count;
            /* Print the number of elements in the current partition. */
            s = alder_hashtable_mcas_printCountPerPartition(a->fpout,
                                                            s_count,
                                                            a->i_ni,
                                                            a->ht[c_table]->i_np,
                                                            a->n_np);
            assert(s == ALDER_STATUS_SUCCESS);
            /* Reset the table. */
            alder_hashtable_mcas_reset(a->ht[c_table]);
            
            /* Change the main i_np. */
            deselect_empty_table(a, c_table);
            a->main_i_np++;
        } else {
            // No full outbuffer.
            assert(isFinishedInput == ALDER_YES);
        }
        
        if (isFinishedInput == ALDER_YES) {
            // Check all of the out buffer.
            isFinishedInput = check_table_empty(a);
            if (isFinishedInput == ALDER_YES) {
                // It is time to exit.
                break;
            }
        }
    }
    a->n_hash = s_total;
cleanup:
    pthread_exit(NULL);
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
    alder_log("counter(): START.");
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_thread2_t *a = (alder_kmer_thread2_t*)t;
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
    
    /* main loop */
    while (1) {
        /* Pick an input buffer. */
        size_t c_inbuffer = a->n_subbuf;
        int isFinishedInput = ALDER_NO;
        while (c_inbuffer == a->n_subbuf && isFinishedInput == ALDER_NO) {
            retain_inbuf(a, &c_inbuffer);
//            sleep(1); // FIXME: this can be adjusted or larger.
//            isFinishedInput = check_inbuf_finished(a);
            if (a->n_blockByReader[a->n_np-1] ==
                a->n_blockByCounter[a->n_np-1]) {
                isFinishedInput = ALDER_YES;
            }
        }
        if (isFinishedInput == ALDER_YES) {
            // No input available.
            assert(c_inbuffer == a->n_subbuf);
        }
        
        if (c_inbuffer < a->n_subbuf) {
            /* setup of inbuf */
            uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
            uint64_t i_np = to_uint64(inbuf,ALDER_KMER_COUNTER2_INBUFFER_I_NP);
            size_t lenbuf = to_size(inbuf,ALDER_KMER_COUNTER2_INBUFFER_LENGTH);
            uint8_t *inbuf_body = inbuf + ALDER_KMER_COUNTER2_INBUFFER_BODY;
            /* setup of table */
            alder_hashtable_mcas_t *c_ht = NULL;
            if (alder_hashtable_mcas_i_np(a->ht[0]) == i_np) {
                c_ht = a->ht[0];
            } else if (alder_hashtable_mcas_i_np(a->ht[1]) == i_np) {
                c_ht = a->ht[1];
            } else {
                alder_loge(ALDER_ERR, "Fatal!");
                assert(0);
                abort();
            }
            
            assert(lenbuf % m2->b == 0);
            uint64_t n_kmer = lenbuf / m2->b;
            size_t x = 0;
            for (uint64_t i_kmer = 0; i_kmer < n_kmer; i_kmer++) {
                
                /* Decode a kmer. */
                for (size_t i = 0; i < ib; i++) {
                    for (size_t j = 0; j < 8; j++) {
                        m2->n[i].key8[j] = inbuf_body[x++];
                    }
                }
                for (size_t j = 0; j < jb; j++) {
                    m2->n[ib].key8[j] = inbuf_body[x++];
                }
                for (int i = 0; i < m->s; i++) {
                    m->n[i] = m2->n[i].key64;
                }
                
                s = alder_hashtable_mcas_increment(c_ht, m->n, res_key,
                                                   a->isMultithreaded);
            
                if (s != ALDER_STATUS_SUCCESS) {
#if !defined(NDEBUG)
                    // alder_log5("index [%03llu] added at [%03llu] ", index, b->ht->index);
//                    alder_encode_number_copy_in_DNA((char*)bm->data, m);
//                    alder_log3("counter(%d): %s", b->i, bdata(bm));
#endif
                    alder_loge(ALDER_ERR, "Fatal!");
                    assert(0);
                    abort();
                }
            }
            assert(x == lenbuf);
            
            increment_n_block(a, i_np);
            release_inbuf(a, c_inbuffer);
            c_inbuffer = a->n_subbuf; // FIXME: redundant? did in retain_inbuf
            
            if (a->n_blockByCounter[i_np] == a->n_blockByReader[i_np]) {
                finish_table(c_ht);
                if (i_np == a->n_np - 1) {
                    isFinishedInput = ALDER_YES;
                    alder_log("counter(): reached the final block ...");
                }
            }
        }
        
        if (isFinishedInput == ALDER_YES) {
            alder_log("counter(): exiting ...");
            break;
        }
    }
    XFREE(res_key);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(m2);
    alder_log("counter(): END");
    pthread_exit(NULL);
}


