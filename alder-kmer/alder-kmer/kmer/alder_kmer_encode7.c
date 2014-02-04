/**
 * This file, alder_kmer_encode7.c, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
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

/**
 *  This file contains a function to encode a binary file to partition files.
 *  The binary file contains blocks of a fixed size, and this encoder should
 *  read them by blocks. The block size is currently 2^16 bytes.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include "bzlib.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_cmacro.h"
#include "libdivide.h"
#include "bstrmore.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_chunk.h"
#include "bstrmore.h"
#include "alder_encode.h"
#include "alder_encode_number.h"
#include "alder_fileseq_streamer.h"
#include "alder_progress.h"
#include "alder_dna.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_uncompress.h"
#include "alder_kmer_encode7.h"

/**
 *  Structure of buffers
 *  --------------------
 *  inbuf: There are [n_subbuf]-many sub buffers. The main inbuf does not have
 *         any header. But, each sub buffer does;
 *         state (1B) type_infile (1B), curbuf (8B), and length (8B).
 *         So, the body start at 18B.
 *         18B + BUFSIZE + ALDER_KMER_SECONDARY_BUFFER_SIZE
 *
 *  outbuf: There are [n_subbuf]-many sub buffers. The main outbuf does not
 *          have any header, but each sub buffer does: state (1B). The number
 *          of sub buffers is determined by the numer of threads. Each sub
 *          buffer is again divided by the number of partitions. This sub of
 *          the sub buffer has length value (8B).
 *          1
 */

static int encoder_id_counter = 0;
static int encoder_full_counter = 0;
static int encoder_exit_counter = 0;

#define SECONDARY_INBUFFER_SIZE 1000
#define INBUFFER_TYPE_INFILE    1
#define INBUFFER_CURRENT        2
#define INBUFFER_LENGTH         10
#define INBUFFER_BODY           18

static void *encoder(void *t);

/**
 *  This function closes a binary file.
 *
 *  @param o        encoder
 *
 *  @return SUCCESS or FAIL
 */
static int close_infile (alder_kmer_encode7_t *o)
{
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    assert(o->fx != NULL);
    close((int)((intptr_t)o->fx));
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function opens a binary file.
 *
 *  @param o        encoder
 *  @param i_infile input file index
 *
 *  @return SUCCESS or FAIL
 */
static int open_infile (alder_kmer_encode7_t *o)
{
    int fp = -1;
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty == 1);
    
    char *fn = bdata(o->infile->entry[0]);
    fp = open(fn, O_RDONLY);
    if (fp < 0) {
        alder_loge(ALDER_ERR_FILE, "cannot open file %s - %s",
                   fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    o->fx = (void *)(intptr_t)fp;
    
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function frees the memory used by an encoder.
 *
 *  Free these variables:
 *  1. lock_write
 *  2. inbuf
 *  3. outbuf
 *  4. fx
 *  5. fpout
 *  6. dout
 *  7. n_i_byte
 *  8. n_i_kmer
 *
 *  @param o encoder
 */
static void
alder_kmer_encode7_destroy (alder_kmer_encode7_t *o)
{
    if (o != NULL) {
        XFREE(o->n_i_kmer);
        XFREE(o->n_i_byte);
        bdestroy(o->dout);
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
                o->fpout[j] = NULL;
            }
            XFREE(o->fpout);
        }
        close_infile(o);
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        XFREE(o->lock_write);
        XFREE(o);
    }
}

/**
 *  This function creates an encoder.
 *
 *  @param n_encoder            number of encoder threads
 *  @param i_iteration          iteration index
 *  @param kmer_size            K
 *  @param disk_space           D
 *  @param memory_available     M
 *  @param sizeInbuffer         inbuf size
 *  @param sizeOutbuffer        outbuf size
 *  @param n_iteration          n_ni
 *  @param n_partition          n_np
 *  @param totalfilesize        ?
 *  @param n_byte               ?
 *  @param n_total_kmer         total number of kmers to process
 *  @param n_current_kmer       current number of kmers
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param infile               infile
 *  @param outdir               out directory
 *  @param outfile              out file
 *
 *  @return encoder
 */
static alder_kmer_encode7_t *
alder_kmer_encode7_create(int n_encoder,
                           int i_iteration,
                           int kmer_size,
                           long disk_space,
                           long memory_available,
                           long sizeInbuffer,
                           long sizeOutbuffer,
                           uint64_t n_iteration,
                           uint64_t n_partition,
                           size_t totalfilesize,
                           size_t n_byte,
                           size_t n_total_kmer,
                           size_t n_current_kmer,
                           int progress_flag,
                           int progressToError_flag,
                           struct bstrList *infile,
                           const char *outdir,
                           const char *outfile)
{
    int s = 0;
    alder_log5("Creating an encoder ...");
    
    alder_kmer_encode7_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "failed to create an encoder data");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->size_fixedoutbuffer = sizeOutbuffer;
    o->size_fixedinbuffer = sizeInbuffer;
    assert(sizeInbuffer == (1 << 16));
    o->k = kmer_size;
    o->b = alder_encode_number2_bytesize(kmer_size);
    o->n_ni = n_iteration;
    o->n_np = n_partition;
    o->i_ni = i_iteration;
    o->n_encoder = n_encoder;
    /* in buffer */
    o->size_subinbuf = INBUFFER_BODY + sizeInbuffer + SECONDARY_INBUFFER_SIZE;
    o->size_inbuf = o->size_subinbuf * o->n_encoder;
    /* out buffer */
    size_t size_outbuf = (size_t)memory_available << 20;
    size_t size_suboutbuf = size_outbuf / n_encoder;
    size_t size_suboutbuf2 = size_suboutbuf / n_partition;
    size_t size_suboutbuf3 = size_suboutbuf2 - sizeof(uint64_t);
    o->n_kmer_suboutbuf = size_suboutbuf3 / o->b;
    o->size_suboutbuf3 = o->n_kmer_suboutbuf * o->b;
    o->size_suboutbuf2 = o->size_suboutbuf3 + sizeof(uint64_t);
    o->size_suboutbuf = o->size_suboutbuf2 * n_partition;
    o->size_outbuf = o->size_suboutbuf * n_encoder;
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log2("*** Kmer encode setup ***");
    alder_log2("%*s %d", width, "Version:", 7);
    alder_log2("%*s %zu (KB)", width, "inbuf:", o->size_inbuf >> 10);
    alder_log2("%*s %ld (MB)", width, "Memory space:", memory_available);
    alder_log2("%*s %llu", width, "Number of partitions:", n_partition);
    alder_log2("%*s %zu (MB)", width, "outbuf:", o->size_outbuf >> 20);
    alder_log2("%*s %zu (MB)", width, "suboutbuf:", o->size_suboutbuf >> 20);
    alder_log2("%*s %zu (MB)", width, "suboutbuf2:", o->size_suboutbuf2 >> 20);
    alder_log2("%*s %zu (B)", width, "suboutbuf3:", o->size_suboutbuf3);
    alder_log2("%*s %zu", width, "n_kmer_subutbuf:", o->n_kmer_suboutbuf);
    o->lock_write = NULL;
    o->inbuf = NULL;
    o->outbuf = NULL;
    o->fx = NULL;
    o->fpout = NULL;
    o->dout = NULL;
    o->n_i_byte = NULL;
    o->n_i_kmer = NULL;
    o->infile = infile;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->n_byte = 0;
    o->n_kmer = 0;
    o->n_total_kmer = n_total_kmer;
    o->n_current_kmer = n_current_kmer;
    
    /* Memory */
    o->lock_write = malloc(sizeof(*o->lock_write) * n_encoder);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf); /* bigmem */
    o->dout = bfromcstr(outdir);
    o->n_i_byte = malloc(sizeof(*o->n_i_byte) * n_encoder);
    o->n_i_kmer = malloc(sizeof(*o->n_i_kmer) * n_encoder);
    if (o->lock_write == NULL || o->inbuf == NULL || o->outbuf == NULL ||
        o->dout == NULL || o->n_i_byte == NULL || o->n_i_kmer == NULL ) {
        alder_kmer_encode7_destroy(o);
        return NULL;
    }
    memset(o->lock_write, 0, sizeof(*o->lock_write) * n_encoder);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);
    memset(o->n_i_byte, 0, sizeof(*o->n_i_byte) * n_encoder);
    memset(o->n_i_kmer, 0, sizeof(*o->n_i_kmer) * n_encoder);
    
    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_encode7_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    /* Open input file: a binary file */
    open_infile(o);
    /* Open output files: partition files */
    o->fpout = malloc(sizeof(*o->fpout) * n_partition);
    if (o->fpout == NULL) {
        alder_kmer_encode7_destroy(o);
        return NULL;
    }
    memset(o->fpout, 0, sizeof(*o->fpout) * n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfpar = bformat("%s/%s-%d-%d.par",
                                outdir, outfile, i_iteration, i);
        if (bfpar == NULL) {
            alder_loge(ALDER_ERR, "failed to create bfpar memory");
            alder_kmer_encode7_destroy(o);
            return NULL;
        }
        o->fpout[i] = fopen(bdata(bfpar), "wb");
        if (o->fpout[i] == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to make a file: %s - %s",
                       bdata(bfpar), strerror(errno));
            bdestroy(bfpar);
            alder_kmer_encode7_destroy(o);
            return NULL;
        }
        bdestroy(bfpar);
    }
    
    alder_log5("Finish - Creating encoder7...");
    return o;
}

#pragma mark main

static pthread_mutex_t mutex_read;
static pthread_mutex_t mutex_write;

/**
 *  This function is the main function.
 *
 *  @param n_encoder            number of encoders (or threads)
 *  @param i_iteration          iteration index
 *  @param kmer_size            kmer size
 *  @param disk_space           disk space
 *  @param memory_available     memory
 *  @param sizeInbuffer         input buffer size
 *  @param sizeOutbuffer        output buffer size
 *  @param n_iteration          iterations
 *  @param n_partition          partitions
 *  @param totalfilesize        file size
 *  @param n_byte               [return] bytes
 *  @param n_kmer               [return] kmer 
 *  @param n_total_kmer         total kmer for displaying progress
 *  @param n_current_kmer       current kmer for displaying progress
 *  @param progress_flag        progress
 *  @param progressToError_flag progress std err
 *  @param binfile_given        ?
 *  @param infile               input files
 *  @param outdir               out directory
 *  @param outfile              out file name
 *
 *  @return SUCCESS or FAIL
 */

int
alder_kmer_encode7(int n_encoder,
                   int i_iteration,
                   int kmer_size,
                   long disk_space,
                   long memory_available,
                   long sizeInbuffer,
                   long sizeOutbuffer,
                   uint64_t n_iteration,
                   uint64_t n_partition,
                   size_t totalfilesize,
                   size_t *n_byte,
                   size_t *n_kmer,/* */
                   size_t n_total_kmer,/* */
                   size_t *n_current_kmer,/* */
                   int progress_flag,
                   int progressToError_flag,
                   unsigned int binfile_given,
                   struct bstrList *infile,
                   const char *outdir,
                   const char *outfile)
{
    alder_log5("preparing encoding Kmers...");
    encoder_id_counter = 0;
    encoder_full_counter = 0;
    encoder_exit_counter = 0;
    int s = ALDER_STATUS_SUCCESS;
    /* Create variables for the threads. */
    alder_kmer_encode7_t *data =
    alder_kmer_encode7_create(n_encoder,
                               i_iteration,
                               kmer_size,
                               disk_space,
                               memory_available,
                               sizeInbuffer,
                               sizeOutbuffer,
                               n_iteration,
                               n_partition,
                               totalfilesize,
                               *n_byte,
                               n_total_kmer,
                               *n_current_kmer,
                               progress_flag,
                               progressToError_flag,
                               infile,
                               outdir,
                               outfile);
    pthread_t *threads = malloc(sizeof(*threads) * n_encoder);
    memset(threads, 0, sizeof(*threads) * n_encoder);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY,
                   "failed to create fileseq_kmer_thread_readwriter");
        XFREE(threads);
        alder_kmer_encode7_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    /* Initialize mutex and condition variable objects */
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_mutex_init(&mutex_read, NULL);
    s += pthread_mutex_init(&mutex_write, NULL);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_encoder; i++) {
        s += pthread_create(&threads[i], &attr, encoder, (void *)data);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "failed to create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    /* Wait for all threads to complete */
    alder_log5("Waiting for all threads to complete...");
    for (int i = 0; i < n_encoder; i++) {
        s += pthread_join(threads[i], NULL);
        if (s != 0) {
            alder_loge(ALDER_ERR_THREAD, "failed to join threads - %s",
                       strerror(errno));
            goto cleanup;
        }
    }
    
cleanup:
    /* Cleanup! */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex_read);
    pthread_mutex_destroy(&mutex_write);
    XFREE(threads);
    data->n_byte = 0;
    data->n_kmer = 0;
    for (int i = 0; i < n_encoder; i++) {
        data->n_byte += data->n_i_byte[i];
        data->n_kmer += data->n_i_kmer[i];
    }
    alder_log2("encoder()[%d] Byte: %zu", i_iteration, data->n_byte);
    alder_log2("encoder()[%d] Kmer: %zu", i_iteration, data->n_kmer);
    *n_byte += data->n_byte;
    *n_kmer += data->n_kmer;
    *n_current_kmer += data->n_kmer;
    
    alder_kmer_encode7_destroy(data);
    alder_log5("Encoding Kmer has been finished with %d threads.", n_encoder);
    return ALDER_STATUS_SUCCESS;
}

#pragma mark buffer

/**
 *  This function assigns an index to an encoder.
 *
 *  @return assigned encoder index
 */
static int
assign_encoder_id()
{
    int oval = encoder_id_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = encoder_id_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&encoder_id_counter,
                                           (int)oval, (int)cval);
    }
    return oval;
}

/**
 *  This function increases the full counter. If the returned value is equal to
 *  the number of encoders, then the thread is selected to write partition
 *  files.
 *
 *  @return number of encoders with outbuffer full.
 */
static int
add_encoder_to_full(alder_kmer_encode7_t *a)
{
    int oval = encoder_full_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = encoder_full_counter;
        if (oval == a->n_encoder - 1) {
            cval = 0;
        } else {
            cval = oval + 1;
        }
        cval = __sync_val_compare_and_swap(&encoder_full_counter,
                                           (int)oval, (int)cval);
    }
    return oval + 1;
}

/**
 *  This function lets the last encoder thread write the remaining partitions.
 *  If the returned value is equal to the number of encoders, then that thread
 *  is selected to write partition files.
 *
 *  @return encoder id
 */
static int
add_encoder_to_exit()
{
    int oval = encoder_exit_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = encoder_exit_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&encoder_exit_counter,
                                           (int)oval, (int)cval);
    }
    return oval + 1;
}

#pragma mark thread

/**
 *  This funciton writes the output buffer to partition files.
 */
static size_t write_to_partition_file(alder_kmer_encode7_t *a,
                                      int encoder_id)
{
    pthread_mutex_lock(&mutex_write);
    
    for (int i_np = 0; i_np < a->n_np; i_np++) {
        size_t x = (a->size_suboutbuf * encoder_id +
                    a->size_suboutbuf2 * i_np);
        size_t n = to_size(a->outbuf, x);
        
        uint8_t *outbuf = a->outbuf + x + sizeof(size_t);
        
        ssize_t left_to_write = (ssize_t)n;
        size_t write_size = a->size_fixedoutbuffer;
        ssize_t written_size = 0;
        while (left_to_write > 0) {
            if (left_to_write < write_size) {
                write_size = left_to_write;
            }
            ssize_t written = write(fileno(a->fpout[i_np]),
                                    outbuf + written_size,
                                    write_size);
            if (written == -1) {
                break;
            } else {
                left_to_write -= written;
                written_size += written;
            }
        }
        assert (left_to_write == 0);
        to_size(a->outbuf, x) = 0;
    }
    
    if (a->progress_flag && encoder_id == 0) {
        size_t c = a->n_current_kmer;
        for (int i = 0; i < a->n_encoder; i++) c += a->n_i_kmer[i];
        alder_progress_step(c, a->n_total_kmer,
                            a->progressToError_flag);
    }
    
    pthread_mutex_unlock(&mutex_write);
    return 0;
}

/**
 *  This function reads a fixed-size buffer from a binary file.
 *
 *  @param a          encoder
 *  @param encoder_id encoder id
 *
 *  @return encoder id if there are more to read, otherwise n_encoder
 */
static int
read_from_sequence_file(alder_kmer_encode7_t *a, int encoder_id)
{
    int s;
    
    uint8_t *inbuf = a->inbuf + encoder_id * a->size_subinbuf;
    uint8_t *inbuf_body = inbuf + INBUFFER_BODY;
    
    pthread_mutex_lock(&mutex_read);
    ssize_t len = read((int)((intptr_t)a->fx), inbuf_body,
                       a->size_fixedinbuffer);
    pthread_mutex_unlock(&mutex_read);
    
    assert(len == 0 || len == a->size_fixedinbuffer);
    inbuf[INBUFFER_TYPE_INFILE] = 0;
    to_size(inbuf, INBUFFER_CURRENT) = (size_t)0;
    to_size(inbuf, INBUFFER_LENGTH) = (size_t)len;
    
    /* Progress */
    a->n_i_byte[encoder_id] += len;
    
    if (len > 0) {
        s = encoder_id;
    } else {
        s = a->n_encoder;
    }
    return s;
}

/**
 *  This function runs on the encoder thread.
 *
 *  @param t data
 *
 *  @return SUCCESS or FAIL
 */
static void *encoder(void *t)
{
    int encoder_id = assign_encoder_id();
    alder_kmer_encode7_t *a = (alder_kmer_encode7_t*)t;
    assert(a != NULL);
    alder_log2("encoder(%d)[%d]: START", encoder_id, a->i_ni);
    
    alder_encode_number2_t * s1 = NULL;
    alder_encode_number2_t * s2 = NULL;
    int K = a->k;
    s1 = alder_encode_number2_createWithKmer(K);
    s2 = alder_encode_number2_createWithKmer(K);
    struct libdivide_u64_t fast_ni = libdivide_u64_gen(a->n_ni);
    struct libdivide_u64_t fast_np = libdivide_u64_gen(a->n_np);
    
    alder_kmer_binary_stream_t *bs = alder_kmer_binary_buffer_create();
    
    size_t ib = s1->b / 8;
    size_t jb = s1->b % 8;
    uint64_t ni = a->n_ni;
    uint64_t np = a->n_np;
    size_t c_inbuffer = a->n_encoder;
    size_t c_outbuffer = encoder_id;
    
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_outbuffer == a->n_encoder) {
            write_to_partition_file(a, encoder_id);
            c_outbuffer = encoder_id;
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
        if (c_inbuffer == a->n_encoder) {
            c_inbuffer = read_from_sequence_file(a, encoder_id);
            if (c_inbuffer == a->n_encoder) {
                break;
            }
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Encoder
        ///////////////////////////////////////////////////////////////////////
        assert(c_inbuffer < a->n_encoder);
        assert(c_outbuffer < a->n_encoder);
        
        /* setup of inbuf */
        uint8_t *inbuf = a->inbuf + encoder_id * a->size_subinbuf;
        size_t curbuf = to_size(inbuf,INBUFFER_CURRENT);
        size_t lenbuf = to_size(inbuf,INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + INBUFFER_BODY;
        
        int token;
        if (curbuf == 0) {
            alder_kmer_binary_buffer_open(bs, inbuf_body, lenbuf);
            alder_log3("encoder(%d): fresh new input buffer");
            token = 4; // Start with a new Kmer.
            curbuf = 1;
        } else {
            alder_log3("encoder(%d): resuming where it stopped");
            token = 0; // Continue from where I stopped.
        }
        
        /* 3. While either input is not empty or output is not full: */
        while (token < 5 && c_outbuffer < a->n_encoder) {
            
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
            //            printf("[%zu] %d\n", debug_counter, token);
            int b1 = token;
            int b2 = (b1 + 2) % 4;
            alder_encode_number2_shiftLeftWith(s1,b1);
            alder_encode_number2_shiftRightWith(s2,b2);
            
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            /* 5. Select partition, iteration, and kmer of the two. */
            alder_encode_number2_t *ss = NULL;
            uint64_t i_ni = 0;
            uint64_t i_np = 0;
            uint64_t hash_s1 = alder_encode_number2_hash(s1);
            uint64_t hash_s2 = alder_encode_number2_hash(s2);
            uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
            ss = (hash_s1 < hash_s2) ? s1 : s2;
            uint64_t h_over_ni = libdivide_u64_do(hash_ss, &fast_ni);
            uint64_t h_over_np = libdivide_u64_do(h_over_ni, &fast_np);
            i_ni = hash_ss - h_over_ni * ni;
            i_np = h_over_ni - h_over_np * np;
            //    *i_ni = hash_ss % number_iteration;
            //    *i_np = hash_ss / number_iteration % number_partition;
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            
            /* 6. Save a chosen one in the output buffers. */
            if (i_ni == a->i_ni) {
                size_t pos_count = (a->size_suboutbuf * encoder_id +
                                    a->size_suboutbuf2 * i_np);
                uint64_t oval = to_uint64(a->outbuf, pos_count);
                size_t x = (pos_count + 8 + oval);
                uint64_t *outbuf_uint64 = (uint64_t*)(a->outbuf + x);
                for (size_t i = 0; i < ib; i++) {
                    outbuf_uint64[i] = ss->n[i].key64;
                }
                x += (8 * ib);
                for (size_t j = 0; j < jb; j++) {
                    a->outbuf[x++] = ss->n[ib].key8[j];
                }
                to_uint64(a->outbuf, pos_count) = oval + a->b;
                
                a->n_i_kmer[encoder_id]++;
                
                assert(a->size_suboutbuf3 >= oval + a->b);
                if (a->size_suboutbuf3 <= oval + a->b) {
                    c_outbuffer = a->n_encoder;
                    break;
                }
            }
        }
        
        /* 9. Mark the status if inbuf. */
        if (token == 5) {
            // empty inbuf - read more input
            c_inbuffer = a->n_encoder;
            alder_log2("encoder(%d): inbuf is empty.", encoder_id);
        } else {
            // partial inbuf
            // Save the current buffer position for later use.
            to_size(inbuf,INBUFFER_CURRENT) = curbuf;
            alder_log2("encoder(%d): inbuf is not yet empty.",
                       encoder_id);
        }
        if (c_outbuffer < a->n_encoder) {
            alder_log2("encoder(%d): outbuf is not yet full.",
                       encoder_id);
        } else {
            // full outbuf
            assert(c_outbuffer == a->n_encoder);
            alder_log2("encoder(%d): outbuf is full.", encoder_id);
        }
    }
    /* Flush any remaining output buffer. */
    write_to_partition_file(a, encoder_id);
    
    alder_kmer_binary_buffer_destroy(bs);
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    alder_log2("encoder(%d)[%d]: END", encoder_id, a->i_ni);
    pthread_exit(NULL);
}

