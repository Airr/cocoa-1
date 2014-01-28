/**
 * This file, alder_kmer.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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

/* 
 * READWRITER - kmerrw
 * -------------------
 * All of the counter threads works on a single partition file. Only after they
 * finished a partition file, can they go to a next partition file. Counter
 * threads would not exit until all of the partition files are finished.
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
#include "alder_hashtable.h"
#include "alder_fileseq.h"
#include "alder_kmer.h"
#include "alder_mcas_wf.h"

enum {
    ALDER_KMER_THREAD_SUCCESS,
    ALDER_KMER_THREAD_ERROR
};

enum {
    ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY,
    ALDER_KMER_THREAD_KRWC_INBUFFER_FULL,
    ALDER_KMER_THREAD_KRWC_SHOULD_WAIT,
    ALDER_KMER_THREAD_KRWC_CANNOT_EXIT,
    ALDER_KMER_THREAD_KRWC_CAN_EXIT
};

typedef struct alder_kmer_counter_struct
alder_kmer_counter_t;

typedef struct alder_kmer_kmerrw_struct
alder_kmer_kmerrw_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_kmerrw_struct {
    bstring boutfile;         /* outfile name                                */
    bstring boutdir;          /* out directory                               */
    FILE *fpout;              /* (not owned) count table output file pointer */
    FILE *fpin;               /* partition input file pointer                */
    int n_counter;            /* n_counter - number of counter threads       */
    int k;                    /* k - kmer size                               */
    uint64_t i_ni;            /* i-th iteration                              */
    uint64_t n_ni;            /* n_ni - number of iteration                  */
    uint64_t n_np;            /* n_np - number of partition                  */
    size_t size_inbuf;        /* size_inbuf - size of the input buffer       */
    size_t end_inbuf;         /* ending position of input buffer             */
    char *inbuf;              /* inbuf - input buffer                        */
    alder_hashtable_mcas_t *ht; /* (per partition) hash table                */
    alder_kmer_counter_t **counter;
    struct bstrList *infile;  /* (not own) input partition file names        */
    /* stat */
    size_t n_byte;
    size_t n_kmer;
    size_t n_hash;
    size_t totalFilesize;     /* total file size                             */
    int progress_flag;        /* flag for progress bar                       */
    int progressToError_flag; /* flag for progress bar                       */
    int nopack_flag;          /* flag for no pack operation                  */
};

/* Counter thread accesses this type.
 * Hash table can be shared by multiple threads.
 * Hash table would be created by krwc.
 */
struct alder_kmer_counter_struct {
    int i;                    /* counter ID                                  */
    int k;                    /* kmer size                                   */
    int status;               /* status of counter's buffers                 */
    bool isMultithreaded;     /* are there other counters?                   */
    size_t size_inbuf;        /* size of the input buffer                    */
    size_t end_inbuf;         /* ending position of input buffer             */
    char *inbuf;              /* in buffer (size_inbuf)                      */
    alder_hashtable_mcas_t *ht;   /* (not own) hash table                    */
    /* stat */
    size_t n_byte;
    size_t n_kmer;
};

static void
alder_kmer_counter_init(alder_kmer_counter_t *o,
                               int i_counter,
                               bool isMultithreaded,
                               int kmer_size,
                               size_t size_inbuf);

static void
alder_kmer_kmerrw_init(alder_kmer_kmerrw_t *o,
                       FILE *fpout,
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
                       size_t size_inbuf);

static alder_kmer_counter_t *
alder_kmer_counter_create(int i_counter,
                                 bool isMultithreaded,
                                 int kmer_size,
                                 size_t size_inbuf);

static void
alder_kmer_counter_destroy(alder_kmer_counter_t *o);


static alder_kmer_kmerrw_t *
alder_kmer_kmerrw_create(FILE *fpout,
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
                         const char *outfile,
                         size_t size_inbuf);
static void
alder_kmer_kmerrw_destroy(alder_kmer_kmerrw_t *o);

static void
alder_kmer_kmerrw_destroy_with_error
(alder_kmer_kmerrw_t *o, int s);

static int
alder_kmer_kmrwc_open_partition(alder_kmer_kmerrw_t *o,
                                       uint64_t i_np);

static int
kmerrw_streamer(alder_kmer_kmerrw_t* o);

//static size_t
//adjust_buffer_size_for_packed_kmer(int kmer_size, size_t size_buf);


static void
let_all_counters_exit(alder_kmer_kmerrw_t *a,
                      uint64_t *n_byte_p,
                      uint64_t *n_kmer_p);

static void
let_all_counters_wait(alder_kmer_kmerrw_t *a);

#pragma mark count

/* The thread model of readwriter and worker needs a mutex and a conditional
 * variable. During the handshaking between a controller and a worker, the
 * controller can tell using a global variable which worker is selected. The
 * selected_counter is used as this global variable. I could see these
 * variables in this file only thanks to the static qualifier. The two threads
 * need function to run on. kmerrw is a function for the controller, and
 * counter is a function for the workder. Each function takes a pointer that I
 * use to pass parameters for those threads. In the counterpart header file, I
 * could see a few C structs. These structs will be passed to the functions
 * that the two threads run on.
 */
static pthread_mutex_t counter2kmerrw_mutex;
static pthread_cond_t counter2kmerrw_cv;
static pthread_mutex_t *kmerrw2counter_mutex;
static pthread_cond_t *kmerrw2counter_cv;
static int selected_counter;
static void *kmerrw(void *t);
static void *counter(void *t);

#pragma mark main

/* This function creates `n_thread' threads to read `n_np' partitioned files in
 * `outdir' with k-mers of size being `kmer_size' and to create a count table.
 * This is called with the order index of `i_ni' iteration as one of `n_ni'
 * iterations. The size of a hash table is determined by the available memory
 * memory_available. A complete hash table is written in the output file
 * fpout.
 * returns
 * 0 on success
 * 1 if an error occurs.
 *
 * Create a hash table.
 * Add kmers to the hash table.
 * Write the hash table in a file.
 * A rewriter thread reads from and writes to files.
 * Encoder threads converts sequnces to kmers.
 *
 * memory
 * . kmerrw
 * . threads
 */
int alder_kmer_count_iteration(FILE *fpout,
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
                               size_t *n_hash,
                               int progress_flag,
                               int progressToError_flag,
                               int nopack_flag,
                               uint8_t *inbuf,
                               size_t size_data,
                               struct bstrList *infile,
                               const char *outdir,
                               const char *outfile)
{
    alder_log5("preparing counting Kmers...");
    int s = 0;
    selected_counter = 0;
    if (n_counter > 1 && kmer_size > 31) {
        mcas_init(n_counter);
    }
    int n_thread = n_counter + 1;
    
    int size_inbuf;
    if (sizeInbuffer == 0) {
        size_inbuf = (int) buffer_size_for_packed_kmer(n_counter, kmer_size,
                                                       memory_available,
                                                       n_np,
                                                       n_nh,
                                                       totalfilesize);
    } else {
        size_inbuf = (int)
        alder_encode_number_adjust_buffer_size_for_packed_kmer
        (kmer_size, (size_t)sizeInbuffer);
    }
    alder_log("main(): buffer: %d MB, %d B", size_inbuf >> 20, size_inbuf);
    alder_kmer_kmerrw_t *krwc =
    alder_kmer_kmerrw_create(fpout,
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
                             outfile,
                             size_inbuf);
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    kmerrw2counter_mutex = malloc(sizeof(*kmerrw2counter_mutex) * n_counter);
    kmerrw2counter_cv = malloc(sizeof(*kmerrw2counter_cv) * n_counter);
    if (krwc == NULL || threads == NULL ||
        kmerrw2counter_mutex == NULL || kmerrw2counter_cv == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create threads for counting");
        XFREE(threads);
        XFREE(kmerrw2counter_mutex);
        XFREE(kmerrw2counter_cv);
        alder_kmer_kmerrw_destroy(krwc);
        return ALDER_KMER_THREAD_ERROR;
    }
    memset(threads,0,sizeof(*threads)*n_thread);
    memset(kmerrw2counter_mutex,0,sizeof(*kmerrw2counter_mutex) * n_counter);
    memset(kmerrw2counter_cv,0,sizeof(*kmerrw2counter_cv) * n_counter);
    
    for (int i = 0; i < n_counter; i++) {
        s += pthread_mutex_init(kmerrw2counter_mutex + i, NULL);
        s += pthread_cond_init (kmerrw2counter_cv + i, NULL);
    }
    s += pthread_mutex_init(&counter2kmerrw_mutex, NULL);
    s += pthread_cond_init (&counter2kmerrw_cv, NULL);
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    s += pthread_create(&threads[0], &attr, kmerrw, (void *)krwc);
    for (int i = 0; i < n_counter; i++) {
        s += pthread_create(&threads[i+1], &attr, counter, (void *)krwc->counter[i]);
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
    pthread_mutex_destroy(&counter2kmerrw_mutex);
    pthread_cond_destroy(&counter2kmerrw_cv);
    for (int i = 0; i < n_counter; i++) {
        s += pthread_mutex_destroy(kmerrw2counter_mutex + i);
        s += pthread_cond_destroy(kmerrw2counter_cv + i);
    }
    XFREE(kmerrw2counter_cv);
    XFREE(kmerrw2counter_mutex);
    XFREE(threads);
    *n_byte = krwc->n_byte;
    *n_hash = krwc->n_hash;
    alder_kmer_kmerrw_destroy(krwc);
    alder_log5("Counting Kmers has been finished with %d threads.", n_thread);
    return 0;
}

/* This function runs as a thread for the controller or readwriter.
 * It keeps reading from a k-mer file to send a sequence of k-mers to worker 
 * counter threads.
 * procedure
 * . when one partition is done, the hash table should be written out.
 * . when all of partition are done, we let the threads exit.
 * . So, I need to loop through partitions. I do not have a single table for
 *   all of the partition. Each partition would create a count table.
 *
 * 1. Reads a chunk of data from the input file.
 *    fread, read, and mmap. I might have to try these.
 *
 * Open a partition file.
 * Create a hash table.
 * Write out the hash table.
 * Close the partition file.
 */
static void *kmerrw(void *t)
{
    alder_log("kmerrw(): START.");
    int s = 0;
    int64_t s_status = 0;
    int64_t s_count = 0;
    alder_kmer_kmerrw_t *a = (alder_kmer_kmerrw_t*)t;
    assert(a != NULL);
    assert(t != NULL);
    uint64_t n_kmer = 0;
    uint64_t n_byte = 0;
    
    for (uint64_t i_np = 0; i_np < a->n_np; i_np++) {
        alder_log2("kmerrw(): iteration(%llu) - partition(%llu)",
                   a->i_ni, i_np);
        s = alder_kmer_kmrwc_open_partition(a, i_np);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR_THREAD,
                       "kmerrw(): faild to open partition files");
            goto cleanup;
        }
        alder_log5("kmerrw(): Resetting the key and value of a hash table.");
        s = alder_hashtable_mcas_reset(a->ht);
        
        // 2. Repeat until there is no counter thread.
        assert(a->end_inbuf == 0);
        assert(s == ALDER_STATUS_SUCCESS);
        int remaining_counter = a->n_counter;
        while (remaining_counter > 0) {
            // 1
            if (a->end_inbuf == 0 && s == ALDER_STATUS_SUCCESS) {
                s = kmerrw_streamer(a);
            }
            
            // 3. Wait for a call from an Encoder.
            ///////////////////////////////////////////////////////////////////
            // Critical Section: BEGIN
            ///////////////////////////////////////////////////////////////////
            pthread_mutex_lock(&counter2kmerrw_mutex);
            alder_log4("kmerrw(): waiting for a call from a counter...");
            selected_counter = -1;
            pthread_cond_wait(&counter2kmerrw_cv, &counter2kmerrw_mutex);
            alder_log3("kmerrw(): wake-up by counter(%d)", selected_counter);
            
            // 4. Locate the Encoder in idle.
            alder_kmer_counter_t *ec = a->counter[selected_counter];
            assert(selected_counter < a->n_counter);
            assert(ec != NULL);
            assert(ec->i == selected_counter);
            
            if (ec->end_inbuf == ec->size_inbuf + 1) {
                alder_log2("kmerrw(): empty wakeup counter(%d)",
                           selected_counter);
            } else if (a->end_inbuf > 0) {
                assert(ec != NULL);
                memcpy(ec->inbuf, a->inbuf, a->end_inbuf);
                alder_log2("kmerrw(): sending data to counter(%d) "
                           "%zu MB or %zu B",
                           selected_counter,
                           (a->end_inbuf >> 20), a->end_inbuf);
                a->n_byte += a->end_inbuf;
                if (a->end_inbuf == a->size_inbuf) {
                    assert(4 * a->end_inbuf % a->k == 0);
                }
                a->n_kmer += 4 * a->end_inbuf / a->k;
                if (a->progress_flag) {
                    alder_progress_step(a->n_byte, a->totalFilesize,
                                        a->progressToError_flag);
                }
                ec->end_inbuf = a->end_inbuf;
                a->end_inbuf = 0;
                /* 7. Let Encoder work on data. */
                ec->status = ALDER_KMER_THREAD_KRWC_INBUFFER_FULL;
                alder_log2("kmerrw(): full wakeup counter(%d)", selected_counter);
            } else {
                remaining_counter--;
                ec->end_inbuf = ec->size_inbuf + 1;
                ec->status = ALDER_KMER_THREAD_KRWC_CANNOT_EXIT;
                alder_log2("kmerrw(): final wakeup counter(%d)", selected_counter);
            }
            alder_log2("kmerrw(): [rw --------> ct(%d)] let counter(%d) work "
                       "remaining working counters are %d",
                       ec->i, ec->i, remaining_counter);
            pthread_mutex_unlock(&counter2kmerrw_mutex);
        }
        alder_log5("kmerrw(): close the input partition file");
        XFCLOSE(a->fpin);
        alder_log5("kmerrw(): saving the hash table (partition:%d)", i_np);
        
        /* Print the elements in the current partition. */
        s_status = alder_hashtable_mcas_printPackToFILE(a->ht, a->fpout);
        if (s_status < 0) {
            alder_loge(ALDER_ERR_HASH, "cannot save the hash table");
        }
        s_count += s_status;
        /* Print the number of elements in the current partition. */
        s = alder_hashtable_mcas_printCountPerPartition(a->fpout,
                                                        s_status,
                                                        a->i_ni,
                                                        i_np,
                                                        a->n_np);
        assert(s == ALDER_STATUS_SUCCESS);
        
        let_all_counters_wait(a);
    }
    let_all_counters_exit(a, &n_byte, &n_kmer);

    a->n_hash = (size_t)s_count;
    alder_log("kmerrw(): bytes - %llu (from encoder)", n_byte);
    alder_log("kmerrw(): bytes - %llu", a->n_byte);
    alder_log("kmerrw(): Kmers - %llu (from encoder)", n_kmer);
    alder_log("kmerrw(): Kmers - %llu", a->n_kmer);
    alder_log("kmerrw(): Kmers(written) - %lld", s_count);
    alder_log("kmerrw(): END");
cleanup:
    pthread_exit(NULL);
}

/* This function encodes and saves kmers in multiple output buffers.
 *
 * I use CAS or compare-and-swap to increase the count of kmer m in the hash
 * table. I would use hash table update with Marcais and Kingsford (2011),
 * where I need an EMPTY key. I could use 0x44...44 as an EMPTY key.
 *
 * memory
 * . m - a large number representing a kmer
 */
static void *counter(void *t)
{
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_counter_t *b = (alder_kmer_counter_t*)t;
    assert(t != NULL);
    assert(b != NULL);
    alder_log("counter(%d): START", b->i);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(b->k);
    if (m == NULL) {
        alder_loge(1, "Fatal - counter(%d): not enough memory", b->i);
        pthread_exit(NULL);
    }
    uint64_t *res_key = malloc(sizeof(*res_key) * ALDER_BYTESIZE_KMER(b->k,ALDER_NUMKMER_8BYTE));
    assert(res_key != NULL);
    memset(res_key, 0, sizeof(*res_key) * ALDER_BYTESIZE_KMER(b->k,ALDER_NUMKMER_8BYTE));
    bstring bm = bfromcstralloc(b->k + 1, "");
    if (bm == NULL) {
        alder_encode_number_destroy(m);
        alder_loge(1, "Fatal - counter(%d): not enough memory", b->i);
        pthread_exit(NULL);
    }
    assert(b->end_inbuf == 1); // Just for entering while loop.
    assert(b->status == ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY);
    
    /* Main Loop */
    while (b->status != ALDER_KMER_THREAD_KRWC_CAN_EXIT) {
        alder_log4("counter(%d): start calling the kmerrw.", b->i);
        while (b->status == ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY) {
            pthread_mutex_lock(&counter2kmerrw_mutex);
            if (selected_counter == -1) {
                alder_log2("counter(%d): [rw <-------- ct(%d)] called kmerrw.",
                           b->i, b->i);
                selected_counter = b->i;
                pthread_cond_signal(&counter2kmerrw_cv);
            } else {
                alder_log5("counter(%d): zzz", b->i);
            }
            pthread_mutex_unlock(&counter2kmerrw_mutex);
            sleep(1);
        }
        alder_log3("counter(%d): wake-up with %d", b->i, b->end_inbuf);
        alder_log2("counter(%d): .................................", b->i);
        
#if !defined(NDEBUG)
        if (b->status == ALDER_KMER_THREAD_KRWC_INBUFFER_FULL) {
            assert(b->end_inbuf > 0);
        }
#endif
        // 2. Work on counting kmer.
        if (b->end_inbuf == 0) {
            // Let it exit.
            alder_log2("counter(%d): no input buffer.", b->i);
            b->status = ALDER_KMER_THREAD_KRWC_CAN_EXIT;
        } else if (b->end_inbuf <= b->size_inbuf) {
            // Any content in inbuf.
            uint8_t *inbuf = (uint8_t*)b->inbuf;
            uint8_t *cbuf = inbuf;
            int i_8bit = 7;
            if (b->end_inbuf == b->size_inbuf) {
                assert(4 * b->end_inbuf % b->k == 0);
            }
            uint64_t n_kmer = 4 * b->end_inbuf / b->k;
            for (uint64_t i = 0; i < n_kmer; i++) {
                cbuf = alder_encode_number_kmer_with_packed(m, cbuf, &i_8bit);
                if (b->k < 4 && m->n[0] == 0) {
                    break;
                }
                s = alder_hashtable_mcas_increment(b->ht, m->n, res_key,
                                                   b->isMultithreaded);
                if (s != ALDER_STATUS_SUCCESS) {
#if !defined(NDEBUG)
                    // alder_log5("index [%03llu] added at [%03llu] ", index, b->ht->index);
                    alder_encode_number_copy_in_DNA((char*)bm->data, m);
                    alder_log3("counter(%d): %s", b->i, bdata(bm));
#endif
                    assert(0);
                }
            }
            b->n_byte += b->end_inbuf;
            b->n_kmer += n_kmer;
            alder_log2("counter(%d): counted %llu kmers.", b->i, n_kmer);
            b->end_inbuf = 0;
            b->status = ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY;
        } else {
            assert(b->end_inbuf == b->size_inbuf + 1);
            // make it wait
            pthread_mutex_lock(kmerrw2counter_mutex + b->i);
            alder_log2("counter(%d): no input so waiting...", b->i);
            b->status = ALDER_KMER_THREAD_KRWC_SHOULD_WAIT;
            pthread_cond_wait(kmerrw2counter_cv + b->i,
                              kmerrw2counter_mutex + b->i);
            alder_log2("counter(%d): resuming...", b->i);
            b->end_inbuf = 0;
            b->status = ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY;
            pthread_mutex_unlock(kmerrw2counter_mutex + b->i);
        }
    }
    alder_log("counter(%d): total number of bytes - %llu", b->i, b->n_byte);
    alder_log("counter(%d): total number of Kmers - %llu", b->i, b->n_kmer);
    alder_log("counter(%d): END", b->i);
    b->i = -1;
    XFREE(res_key);
    alder_encode_number_destroy(m);
    bdestroy(bm);
    pthread_exit(NULL);
}

#pragma mark thread argument

void
alder_kmer_counter_init(alder_kmer_counter_t *o,
                               int i_counter,
                               bool isMultithreaded,
                               int kmer_size,
                               size_t size_inbuf)
{
    o->i = i_counter;
    o->isMultithreaded = isMultithreaded;
    o->k = kmer_size;
    o->status = ALDER_KMER_THREAD_KRWC_INBUFFER_EMPTY;
    o->size_inbuf = size_inbuf;
    o->end_inbuf = 1; // set by counter - for entering while loop in counter.
    o->inbuf = NULL;
    o->ht = NULL;
    o->n_byte = 0;
    o->n_kmer = 0;
}

/* This function prepares a data structure for counter thread. The counter 
 * thread reads the content in the inbuf, and updates the hash table.
 * The counter is given a counter ID (i_counter), kmer_size, and the buffers
 * size.
 * returns
 * alder_kmer_counter_t on success
 * nil if an error occurs.
 * memory
 * . inbuf
 */
alder_kmer_counter_t *
alder_kmer_counter_create(int i_counter,
                                 bool isMultithreaded,
                                 int kmer_size,
                                 size_t size_inbuf)
{
    alder_log5("Creating [%d] alder_kmer_counter_t", i_counter);
    /* initialization */
    alder_kmer_counter_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create kmer_thread_counter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    alder_kmer_counter_init(o, i_counter, isMultithreaded,
                                   kmer_size, size_inbuf);
    /* memory allocation */
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    if (o->inbuf == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create kmer_thread_counter");
        alder_kmer_counter_destroy(o);
        return NULL;
    }
    memset(o->inbuf, 0, sizeof(*o->inbuf) * size_inbuf);
    alder_log5("Finish - Creating alder_kmer_counter_t...");
    return o;
}

/* This function destroys a given counter thread.
 */
void alder_kmer_counter_destroy(alder_kmer_counter_t *o)
{
    if (o != NULL) {
        XFREE(o->inbuf);
        XFREE(o);
    }
}

void
alder_kmer_kmerrw_init(alder_kmer_kmerrw_t *o,
                       FILE *fpout,
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
                       size_t size_inbuf)
{
    o->boutdir = NULL;
    o->fpout = fpout;
    o->fpin = NULL;           /* set by kmerrw */
    o->n_counter = n_counter;
    o->k = kmer_size;
    o->i_ni = i_ni;
    o->n_ni = n_ni;
    o->n_np = n_np;
    o->size_inbuf = size_inbuf;
    o->end_inbuf = 0;
    o->inbuf = NULL;
    o->ht = NULL;
    o->counter = NULL;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->nopack_flag = nopack_flag;
    o->infile = infile;
    o->n_kmer = 0;
    o->n_byte = n_byte;
}

/* This function prepares a data structure for kmerrw thread. This also follows
 * almost the same arguments as alder_kmer_kmerrw_create.
 * returns
 * object on success
 * nil if an error occurs
 * procedure
 * . create a hash table.
    // Create a hash table here. The hash table is once for all the partitions.
    // There are multiple ways of creating a hash table.
    // 1. Create one before all the iteration.
    // 2. Create, use, and destory one in each iteration.
    // 3. Create, use, and destroy one in each partition and each iteration.
    // For now, I use the 2nd one. Here is the part of hash table creation.
//    o->ht
    // where is hashtable_size? where is empty key?
    // hashtable_size is from the command line.
    // empty_key should be found using k-mer size, and the iteration.
    // boutdir/0.par is the current partition.
    // How to find an empty key?
    // 1. If kmer_size % 31 != 0, then use an array of 0xFF as an empty key.
    // 2. Otherwise, try to generate a key of 64 possible ones where
    //    each byte is something where the first two bits are 0. And see if ...
    // ?. I think that the empty key could be created in each partition.
    // For now, we could set it to 0xFF by passing NULL in the empty_key.
    // 3. If I could not find an empty key from the above, I need to
    //    borrow one from other iteration and other partition. Then,
 //    initialize the key with the value using for-loop.
 */
alder_kmer_kmerrw_t *
alder_kmer_kmerrw_create(FILE *fpout,
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
                         const char *outfile,
                         size_t size_inbuf)
{
    alder_log5("Creating alder_kmer_kmerrw_t...");
    alder_kmer_kmerrw_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create alder_kmer_kmerrw_t");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    alder_kmer_kmerrw_init(o,
                           fpout,
                           n_counter,
                           i_ni,
                           kmer_size,
                           memory_available,
                           n_ni,
                           n_np,
                           n_nh,
                           totalfilesize,
                           n_byte,
                           progress_flag,
                           progressToError_flag,
                           nopack_flag,
                           infile,
                           size_inbuf);
    /* Memory */
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    o->ht = alder_hashtable_mcas_create(kmer_size, n_nh);
    if (o->ht == NULL || o->boutdir == NULL || o->inbuf == NULL) {
        alder_kmer_kmerrw_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->inbuf,0,sizeof(*o->inbuf) * size_inbuf);
    
    o->counter = malloc(sizeof(*o->counter)*n_counter);
    if (o->counter == NULL) {
        alder_kmer_kmerrw_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->counter, 0, sizeof(*o->counter)*n_counter);
    for (int i = 0; i < n_counter; i++) {
        o->counter[i] = alder_kmer_counter_create(i,
                                                  1 < n_counter,
                                                  kmer_size,
                                                  size_inbuf);
        if (o->counter[i] == NULL) {
            alder_kmer_kmerrw_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
        o->counter[i]->ht = o->ht;
    }
    
    alder_log5("Finish - Creating alder_kmer_kmerrw_t...");
    return o;
}

void alder_kmer_kmerrw_destroy_with_error
(alder_kmer_kmerrw_t *o, int s)
{
    if (s != 0) {
        alder_loge(s, "cannot create alder_kmer_kmerrw_t");
    }
    alder_kmer_kmerrw_destroy(o);
}

/* fpout is not deallocated because it is just a copy of pointer.
 * In other words, alder_kmer_kmerrw_t does not own fpout.
 * See alder_kmer_kmerrw_create to see that.
 */
void alder_kmer_kmerrw_destroy(alder_kmer_kmerrw_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
        XFCLOSE(o->fpin);
        alder_hashtable_mcas_destroy(o->ht);
        XFREE(o->inbuf);
        if (o->counter != NULL) {
            for (int i = 0; i < o->n_counter; i++) {
                alder_kmer_counter_destroy(o->counter[i]);
            }
            XFREE(o->counter);
        }
        bdestroy(o->boutdir);
        bdestroy(o->boutfile);
        XFREE(o);
    }
}



/* This function opens a new partition file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_kmer_kmrwc_open_partition(alder_kmer_kmerrw_t *o, uint64_t i_np)
{
    bstring bfpar = NULL;
    if (o->infile == NULL) {
        bfpar = bformat("%s/%s-%llu-%llu.par",
                        bdata(o->boutdir), bdata(o->boutfile),
                        o->i_ni, i_np);
        if (bfpar == NULL) return ALDER_STATUS_ERROR;
    } else {
//        assert(o->i_ni == -1);
        bfpar = bstrcpy(o->infile->entry[i_np]);
        if (bfpar == NULL) return ALDER_STATUS_ERROR;
    }
    assert(o->fpin == NULL);
    alder_log2("kmerrw(): openning a partition file %s", bdata(bfpar));
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

//size_t adjust_buffer_size_for_packed_kmer(int kmer_size,
//                                          size_t size_buf)
//{
//    if (kmer_size % 4 == 0) {
//        // No code.
//    } else if (kmer_size % 4 == 1 || kmer_size % 4 == 3) {
//        size_buf = size_buf / kmer_size * kmer_size;
//    } else if (kmer_size % 4 == 2) {
//        size_buf = size_buf / (kmer_size/2) * (kmer_size/2);
//    }
//
//    return size_buf;
//}

/* This function
 * adjusts the input buffer size so that kmers can be packed
 * without kmers that are partially saved for processing.
 *
 * M ~ (8s + 4)x + (t+1)y.
 * y = (M - (8s + 4)x) / (t+1)
 * s - stride of a Kmer in 64-bit numbers
 * x - #_elements in a hash table.
 * t - #_threads
 * y - inbuf size in bytes
 */
size_t buffer_size_for_packed_kmer(int n_thread,
                                   int kmer_size,
                                   long memory_available,
                                   uint64_t np,
                                   size_t nh,
                                   size_t parfilesize)
{
//    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
//    size_t z_memory_available = memory_available * ALDER_MB2BYTE;
//    size_t z_hashtable_size = (sizeof(uint64_t) * stride + sizeof(uint16_t)) * nh;
//    
//    size_t size_buf = (z_memory_available - z_hashtable_size) / (n_thread + 1);

    size_t size_buf = parfilesize/np/n_thread*2;
    if (size_buf > (1 << 26)) {
        size_buf = 1 << 26;
    }
    size_buf = alder_encode_number_adjust_buffer_size_for_packed_kmer(kmer_size, size_buf);
//    size_buf = adjust_buffer_size_for_packed_kmer(kmer_size, size_buf);
    
    return size_buf;
}

/* This function reads partition files.
 * returns
 * 0 on success
 * 1 if an error occurs or it reached the end of the file.
 */
int kmerrw_streamer(alder_kmer_kmerrw_t* o)
{
    o->end_inbuf = fread(o->inbuf, 1, o->size_inbuf, o->fpin);
    if (o->end_inbuf == o->size_inbuf) {
        alder_log2("kmerrw(): reading full %zu B.", o->end_inbuf);
        return 0;
    } else {
        alder_log2("kmerrw(): reading partial %zu B.", o->end_inbuf);
        return 1;
    }
}

/* This function waits for all counters to exit after collecting n_kmer and
 * n_byte.
 */
void
let_all_counters_exit(alder_kmer_kmerrw_t *a,
                      uint64_t *n_byte_p,
                      uint64_t *n_kmer_p)
{
    alder_log2("kmerrw(): let all of the counters exit.");
    for (int i_counter = 0; i_counter < a->n_counter; i_counter++) {
        alder_kmer_counter_t *ec = a->counter[i_counter];
        *n_kmer_p += a->counter[i_counter]->n_kmer;
        *n_byte_p += a->counter[i_counter]->n_byte;
        ec->end_inbuf = 0;
        ec->status = ALDER_KMER_THREAD_KRWC_CAN_EXIT;
    }
    int remaining_counter = a->n_counter;
    while (remaining_counter > 0) {
        remaining_counter = a->n_counter;
        pthread_mutex_lock(&counter2kmerrw_mutex);
        selected_counter = -1;
        for (int i = 0; i < a->n_counter; i++) {
            alder_kmer_counter_t *ec = a->counter[i];
            ec->end_inbuf = 0;
            ec->status = ALDER_KMER_THREAD_KRWC_CAN_EXIT;
            if (ec->i == -1) {
                remaining_counter--;
            }
        }
        pthread_mutex_unlock(&counter2kmerrw_mutex);
    }
}

/* This function waits for all counters are in the waiting-stage.
 */
void
let_all_counters_wait(alder_kmer_kmerrw_t *a)
{
    // Make sure that all counters are waiting.
    alder_log2("kmerrw(): let all of the counters be in the waiting-stage.");
    bool needToWait = true;
    while (needToWait) {
        needToWait = false;
        for (int i = 0; i < a->n_counter; i++) {
            alder_kmer_counter_t *ec = a->counter[i];
            pthread_mutex_lock(kmerrw2counter_mutex + i);
            if (ec->status != ALDER_KMER_THREAD_KRWC_SHOULD_WAIT) {
                needToWait = true;
            }
            pthread_mutex_unlock(kmerrw2counter_mutex + i);
        }
    }
    for (int i = 0; i < a->n_counter; i++) {
        pthread_mutex_lock(kmerrw2counter_mutex + i);
        pthread_cond_signal(kmerrw2counter_cv + i);
        pthread_mutex_unlock(kmerrw2counter_mutex + i);
    }
}