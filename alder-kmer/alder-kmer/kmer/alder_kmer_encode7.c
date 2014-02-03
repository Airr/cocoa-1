/**
 * This file, alder_kmer_encode7.c, is part of alder-kmer.
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
 *  The state of buffer
 *  -------------------
 *
 *  Buffer states by encoder
 *  ------------------------
 *
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
 *
 *  Variables
 *  ---------
 *  c_inbuffer and c_outbuffer are indices for sub buffers. The size of each
 *  sub buffer is denoted by size_subinbuf and size_suboutbuf.
 */

static int encoder_id_counter = 0;
static int encoder_full_counter = 0;
static int encoder_exit_counter = 0;

#define ALDER_KMER_SECONDARY_BUFFER_SIZE        1000
#define ALDER_KMER_ENCODE7_OUTBUFFER_HEADER     1
#define ALDER_KMER_ENCODE7_OUTSUBBUFFER_HEADER  8

//
#define ALDER_KMER_ENCODE7_OUTBUFFER_BODY       8

#define ALDER_KMER_ENCODE7_OUTBUFFER_A          0
#define ALDER_KMER_ENCODE7_OUTBUFFER_B          8
#define ALDER_KMER_ENCODE7_OUTBUFFER_C          16

#define ALDER_KMER_ENCODE7_INBUFFER_TYPE_INFILE 1
#define ALDER_KMER_ENCODE7_INBUFFER_CURRENT     2
#define ALDER_KMER_ENCODE7_INBUFFER_LENGTH      10
#define ALDER_KMER_ENCODE7_INBUFFER_BODY        18
#define ALDER_KMER_ENCODE7_FILETYPE_FASTA       1
#define ALDER_KMER_ENCODE7_FILETYPE_FASTQ       2
#define ALDER_KMER_ENCODE7_FILETYPE_SEQ         3
#define ALDER_KMER_ENCODE7_FILETYPE_BINARY      4


static void *encoder(void *t);

/**
 *  This function closes a partition file.
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
 *  This function opens a partition file.
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

static void
alder_kmer_encode7_destroy (alder_kmer_encode7_t *o)
{
    if (o != NULL) {
        close_infile(o);
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
                o->fpout[j] = NULL;
            }
            XFREE(o->fpout);
        }
        XFREE(o->lock_write);
        XFREE(o->n_i_byte);
        XFREE(o->n_i_kmer);
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        bdestroy(o->dout);
        XFREE(o);
    }
}

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
    alder_log5("Creating fileseq_kmer_thread_readwriter...");
    
    alder_kmer_encode7_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create fileseq_kmer_thread_readwriter");
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
    o->reader_lenbuf = 0;
    o->reader_lenbuf2 = 0;
    o->reader_i_infile = 0;
    o->n_encoder = n_encoder;
    o->size_subinbuf = ALDER_KMER_ENCODE7_INBUFFER_BODY + sizeInbuffer + ALDER_KMER_SECONDARY_BUFFER_SIZE;
    o->size_inbuf = o->size_subinbuf * o->n_encoder;
    o->is_full = 0;
    
    /* outbuf */
    // outbuf = n_encoder x suboutbuf = n_encoder x [n_partition x suboutbuf2]
    // suboutbuf2 = [b1 + fixed_buffer + b2].
    // The first b1 is for buffers that I would have to take from other encoder.
    // The second b2 is for preventing buffer overflow.
    // So, here is a scenario:
    // 1. Fill content from pos [b] of suboutbuf2.
    // 2. Stop if the size is fixedbuffer or greater than that.
    // 3. Find the encoder that previously wrote content to the same partition.
    // 4. Copy the previously remained b to b1, and write the fixed buffer.
    // 5. The remaining part could be larger than b.
    // 6. If so, copy the kmer to the position at fixed_buffer_size, and
    //    the other part that does not fit to a kmer to the start of buffer.
    // This might be a little complicated, so let's have an example.
    // This is an imaginary buffer.
    //
    //  s.  A     B     F         C
    //  X: [aaa] [bbb] [fffffff] [ccc]
    //
    //  0: [000] [000] [0000000] [000] : initial
    //  1: [000] [000] [1110000] [000] : fill
    //  2: [000] [000] [1111110] [000] : fill
    //  3: [000] [000] [1111111] [110] : fill and overflow
    //  4: [000] [000] [xxxxxxx] [zz0] : write x's in B and F (B is empty).
    //  5: [0zz] [000] [0000000] [000] : save z's to A (mark encoder)
    //  6: [011] [000] [1110000] [000] : fill
    //  7: [011] [000] [1111110] [000] : fill
    //  8: [011] [000] [1111111] [110] : fill and overflow
    //  9: [000] [011] [1111111] [110] : move A to B (need to find which encoder)
    // 10: [000] [0xx] [xxxxx11] [110] : write x's in B and F
    // 11: [000] [0xx] [xxxxxzy] [yy0] : move y's to F
    // 12: [00z] [000] [yyy0000] [000] : save z's to A
    // 13: [001] [000] [yyy1110] [000] : fill
    //
    size_t size_outbuf = (size_t)memory_available << 20;
    size_t size_suboutbuf = size_outbuf / n_partition;
    size_t size_suboutbuf2 = size_suboutbuf - sizeof(uint64_t);
    o->n_kmer_suboutbuf = size_suboutbuf2 / o->b;
    o->size_suboutbuf2 = o->n_kmer_suboutbuf * o->b;
    o->size_suboutbuf = o->size_suboutbuf2 + sizeof(uint64_t);
    o->size_outbuf = o->size_suboutbuf * n_partition;
    o->maxsize_suboutbuf = o->size_suboutbuf2 - n_encoder * o->b;
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer encode setup ***");
    alder_log("%*s %d", width, "Version:", 7);
    alder_log("%*s %zu (KB)", width, "inbuf:", o->size_inbuf >> 10);
    alder_log("%*s %ld (MB)", width, "Memory space:", memory_available);
    alder_log("%*s %llu", width, "Number of partitions:", n_partition);
    alder_log("%*s %zu (MB)", width, "outbuf:", o->size_outbuf >> 20);
    alder_log("%*s %zu (MB)", width, "suboutbuf:", o->size_suboutbuf >> 20);
    alder_log("%*s %zu", width, "n_kmer_subutbuf:", o->n_kmer_suboutbuf);
    
    o->lock_write = NULL;
    o->inbuf = NULL;
    o->outbuf = NULL;
    o->infile = infile;
    o->fx = NULL;
    o->fpout = NULL;
    o->dout = NULL;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->n_byte = 0;
    o->n_i_byte = NULL;
    o->n_i_kmer = NULL;
    o->n_kmer = 0;
    o->n_total_kmer = n_total_kmer;
    o->n_current_kmer = n_current_kmer;
    
    /* Memory */
    o->dout = bfromcstr(outdir);
    o->lock_write = malloc(sizeof(*o->lock_write) * n_encoder);
    o->n_i_byte = malloc(sizeof(*o->n_i_byte) * n_encoder);
    o->n_i_kmer = malloc(sizeof(*o->n_i_kmer) * n_encoder);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);    /* bigmem */
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf); /* bigmem */
    if (o->dout == NULL ||
        o->lock_write == NULL ||
        o->n_i_byte == NULL ||
        o->n_i_kmer == NULL ||
        o->inbuf == NULL ||
        o->outbuf == NULL) {
        alder_kmer_encode7_destroy(o);
        return NULL;
    }
    memset(o->lock_write, 0, sizeof(*o->lock_write) * n_encoder);
    memset(o->n_i_byte, 0, sizeof(*o->n_i_byte) * n_encoder);
    memset(o->n_i_kmer, 0, sizeof(*o->n_i_kmer) * n_encoder);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);
    
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
        //        int fd = fileno(o->fpout[i]);
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

static pthread_mutex_t mutex_write;
static pthread_mutex_t mutex_read;

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
 *  @param n_byte               return bytes
 *  @param progress_flag        progress
 *  @param progressToError_flag progress std err
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
        s += pthread_create(&threads[i+0], &attr, encoder, (void *)data);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "cannot create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    /* Wait for all threads to complete */
    alder_log5("Waiting for all threads to complete...");
    for (int i = 0; i < n_encoder; i++) {
        s += pthread_join(threads[i], NULL);
        if (s != 0) {
            alder_loge(ALDER_ERR_THREAD, "cannot join threads - %s",
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

static int
assign_encoder_id()
{
    int oval = encoder_id_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = encoder_id_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&encoder_id_counter, (int)oval, (int)cval);
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
        cval = __sync_val_compare_and_swap(&encoder_full_counter, (int)oval, (int)cval);
    }
    return oval + 1;
}

static int
add_encoder_to_exit()
{
    int oval = encoder_exit_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = encoder_exit_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&encoder_exit_counter, (int)oval, (int)cval);
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
    for (int i_np = 0; i_np < a->n_np; i_np++) {
        size_t x = a->size_suboutbuf * i_np;
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
    
    a->is_full = 0;
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
    uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODE7_INBUFFER_BODY;
    
    pthread_mutex_lock(&mutex_read);
    ssize_t len = read((int)((intptr_t)a->fx), inbuf_body,
                       a->size_fixedinbuffer);
    pthread_mutex_unlock(&mutex_read);
    
    assert(len == 0 || len == a->size_fixedinbuffer);
    inbuf[ALDER_KMER_ENCODE7_INBUFFER_TYPE_INFILE] = 0;
    to_size(inbuf, ALDER_KMER_ENCODE7_INBUFFER_CURRENT) = (size_t)0;
    to_size(inbuf, ALDER_KMER_ENCODE7_INBUFFER_LENGTH) = (size_t)len;
    
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
    alder_log("encoder(%d)[%d]: START", encoder_id, a->i_ni);
    
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
    
    /* Setup of outbuf */
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_outbuffer == a->n_encoder) {
            write_to_partition_file(a, encoder_id);
            c_outbuffer = encoder_id;
            for (int i = 0; i < a->n_encoder; i++) {
                a->lock_write[i] = 2;
            }
        }
        while (a->lock_write[encoder_id] != 0) {
            if (a->lock_write[encoder_id] == 2) {
                a->lock_write[encoder_id] = 0;
            }
        }
        assert(a->lock_write[encoder_id] == 0);
        
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
        size_t curbuf = to_size(inbuf,ALDER_KMER_ENCODE7_INBUFFER_CURRENT);
        size_t lenbuf = to_size(inbuf,ALDER_KMER_ENCODE7_INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODE7_INBUFFER_BODY;
        /* setup of outbuf */
        
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
                /* Find the position to encode a kmer. */
                size_t pos_lock = a->size_suboutbuf * i_np;
                uint64_t oval = 0;
                uint64_t cval = 1;
                while (cval != oval) {
                    oval = to_uint64(a->outbuf, pos_lock);
                    cval = oval + a->b;
                    cval = __sync_val_compare_and_swap((uint64_t*)(a->outbuf + pos_lock),
                                                       (uint64_t)oval,
                                                       (uint64_t)cval);
                }
                
                size_t x = a->size_suboutbuf * i_np + 8 + cval;
                uint64_t *outbuf_uint64 = (uint64_t*)(a->outbuf + x);
                for (size_t i = 0; i < ib; i++) {
                    outbuf_uint64[i] = ss->n[i].key64;
                }
                x += (8 * ib);
                for (size_t j = 0; j < jb; j++) {
                    a->outbuf[x++] = ss->n[ib].key8[j];
                }
                
                a->n_i_kmer[encoder_id]++;
                
                if (a->maxsize_suboutbuf <= cval) {
                    c_outbuffer = add_encoder_to_full(a);
                    a->lock_write[encoder_id] = 1;
                    a->is_full = 1;
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
            to_size(inbuf,ALDER_KMER_ENCODE7_INBUFFER_CURRENT) = curbuf;
            alder_log2("encoder(%d): inbuf is not yet empty.",
                       encoder_id);
        }
        if (c_outbuffer < a->n_encoder) {
            // partial outbuf
            if (a->lock_write[encoder_id] == 0) {
                alder_log2("encoder(%d): outbuf is not yet full, more space.",
                           encoder_id);
            } else {
                // full outbuf, but will wait for the writing thread to finish
                alder_log2("encoder(%d): outbuf is full, and wait...",
                           encoder_id);
            }
        } else {
            // full outbuf
            assert(c_outbuffer == a->n_encoder);
            alder_log2("encoder(%d): outbuf is full.", encoder_id);
        }
    }
    /* Flush any remaining output buffer. */
    c_outbuffer = add_encoder_to_exit();
    if (c_outbuffer == a->n_encoder) {
        write_to_partition_file(a, encoder_id);
    }
    
    alder_kmer_binary_buffer_destroy(bs);
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    alder_log("encoder(%d)[%d]: END", encoder_id, a->i_ni);
    pthread_exit(NULL);
}

