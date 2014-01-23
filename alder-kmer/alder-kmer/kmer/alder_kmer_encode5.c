/**
 * This file, alder_kmer_encode5.c, is part of alder-kmer.
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
#include "alder_kmer_encode5.h"

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
 *         18B + BUFSIZE + inbuf2 (ALDER_KMER_SECONDARY_BUFFER_SIZE)
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

static int dna_char2int[128] = {
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,0,4,1,4,4,4,3,4,4,4,4,4,4,4,4,
    4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,4,
    4,0,4,1,4,4,4,3,4,4,4,4,4,4,4,4,
    4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,4
};



static void *encoder(void *t);

static void
alder_kmer_encoder5_destroy (alder_kmer_encoder5_t *o)
{
    if (o != NULL) {
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
                o->fpout[j] = NULL;
            }
            XFREE(o->fpout);
        }
        XFREE(o->encoder_remaining_outbuf);
        XFREE(o->outbuf);
        XFREE(o->inbuf2);
        XFREE(o->inbuf);
        XFREE(o->lock_partition);
        bdestroy(o->dout);
        XFREE(o);
    }
}

static alder_kmer_encoder5_t *
alder_kmer_encoder5_create(int n_encoder,
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
                           int progress_flag,
                           int progressToError_flag,
                           struct bstrList *infile,
                           const char *outdir,
                           const char *outfile)
{
    alder_log5("Creating fileseq_kmer_thread_readwriter...");
    
    alder_kmer_encoder5_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create fileseq_kmer_thread_readwriter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->size_fixedoutbuffer = 1 << 20; /* FIXME: this must be determined before the function call */
    //    o->size_fixedoutbuffer = 7; /* FIXME: this must be determined before the function call */
    /*        Set this just right after command-line parsing.  */
    o->k = kmer_size;
    o->b = alder_encode_number2_bytesize(kmer_size);
    o->n_ni = n_iteration;
    o->n_np = n_partition;
    o->i_ni = i_iteration;
    o->n_encoder = n_encoder;
    o->lock_reader = ALDER_KMER_ENCODER5_READER_UNLOCK;
    o->lock_writer = ALDER_KMER_ENCODER5_WRITER_UNLOCK;
    o->reader_lenbuf = 0;
    o->reader_lenbuf2 = 0;
    o->reader_type_infile = 0;
    o->reader_i_infile = 0;
    o->n_encoder = n_encoder;
    o->size_inbuf2 = ALDER_KMER_SECONDARY_BUFFER_SIZE;
    o->size_subinbuf = ALDER_KMER_ENCODER5_INBUFFER_BODY + BUFSIZE + o->size_inbuf2;
    o->size_inbuf = o->size_subinbuf * o->n_encoder;
    
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
    o->size_suboutbuf2 = sizeof(size_t)*2 + o->b*3 + o->size_fixedoutbuffer;
    o->size_suboutbuf = o->size_suboutbuf2 * n_partition;
    o->size_outbuf = o->size_suboutbuf * n_encoder;
    alder_log("outbuf: %zu (MB)\n", o->size_outbuf >> 20);
    
    o->inbuf = NULL;
    o->inbuf2 = NULL;
    o->outbuf = NULL;
    o->infile = infile;
    o->fx = NULL;
    o->type_infile = 0;
    o->fpout = NULL;
    o->dout = NULL;
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->n_byte = n_byte;
    o->n_kmer = 0;
    o->n_letter = 0;
    o->status = 0;
    o->flag = 0;
    
    
    int s = 0;
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("%*s %d (MB)", width, "inbuf size:", o->size_inbuf >> 20);
    alder_log("%*s %d (MB)", width, "outbuf size:", o->size_outbuf >> 20);
    
    /* Memory */
    o->dout = bfromcstr(outdir);
    o->lock_partition = malloc(sizeof(*o->lock_partition) * n_partition);
    o->encoder_remaining_outbuf = malloc(sizeof(*o->encoder_remaining_outbuf) * n_partition);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);    /* bigmem */
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * o->size_inbuf2);
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf); /* bigmem */
    if (o->dout == NULL ||
        o->lock_partition == NULL ||
        o->encoder_remaining_outbuf == NULL ||
        o->inbuf == NULL ||
        o->inbuf2 == NULL ||
        o->outbuf == NULL) {
        alder_kmer_encoder5_destroy(o);
        return NULL;
    }
    memset(o->lock_partition, 0, sizeof(*o->lock_partition) * n_partition);
    memset(o->encoder_remaining_outbuf, 0, sizeof(*o->encoder_remaining_outbuf) * n_partition);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->inbuf2, 0, sizeof(*o->inbuf2) * o->size_inbuf2);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);
    
    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_encoder5_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    
    o->fpout = malloc(sizeof(*o->fpout)*n_partition);
    if (o->fpout == NULL) {
        alder_kmer_encoder5_destroy(o);
        return NULL;
    }
    memset(o->fpout, 0, sizeof(*o->fpout)*n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfpar = bformat("%s/%s-%d-%d.par",
                                outdir, outfile, i_iteration, i);
        if (bfpar == NULL) {
            alder_kmer_encoder5_destroy(o);
            return NULL;
        }
        o->fpout[i] = fopen(bdata(bfpar), "wb");
        //        int fd = fileno(o->fpout[i]);
        if (o->fpout[i] == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to make a file: %s",
                       bdata(bfpar));
            bdestroy(bfpar);
            alder_kmer_encoder5_destroy(o);
            return NULL;
        }
        bdestroy(bfpar);
    }
    
    /* init */
    //    size_t x = o->size_inbuf / o->n_subbuf;
    /* buffer state */
    
    //    for (size_t i = 0; i < o->n_subbuf; i++) {
    //        uint8_t *outbuf = o->outbuf + i * o->size_suboutbuf;
    //        *outbuf = ALDER_KMER_ENCODER5_EMPTY_OUTBUF;
    //    }
    
    // n_subbuf * x = total size of inbuf
    // 4 + 8 + 8 + 8 = 28
    //
    // i * x = position of a buffer for each inbuf
    // x - 28 = capacity of a buffer for each inbuf
    // We do not need sbuf.
    // i * x + 28 = start position at which a buffer is stored.
    
    alder_log5("Finish - Creating fileseq_kmer_thread_readwriter...");
    return o;
}

void
alder_kmer_encoder5_destroy_with_error (alder_kmer_encoder5_t *o, int s)
{
    if (s != 0) {
        alder_loge(s, "cannot create fileseq_kmer_thread_readwriter");
    }
    alder_kmer_encoder5_destroy(o);
}

/**
 *  This function closes a partition file.
 *
 *  @param o        encoder
 *
 *  @return SUCCESS or FAIL
 */
static int close_infile (alder_kmer_encoder5_t *o)
{
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    
    if (o->fx != NULL) {
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            gzclose(o->fx);
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            BZ2_bzclose(o->fx);
        } else {
            close((int)((intptr_t)o->fx));
        }
    }
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
static int open_infile (alder_kmer_encoder5_t *o, int i_infile)
{
    int fp = -1;
    gzFile fpgz = NULL;
    BZFILE *fpbz = NULL;
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    
    char *fn = bdata(o->infile->entry[i_infile]);
    o->type_infile = alder_fileseq_type(fn);
    
    if (o->type_infile & ALDER_FILETYPE_GZ) {
        fpgz = gzopen(fn, "r");
    } else if (o->type_infile & ALDER_FILETYPE_BZ) {
        fpbz = BZ2_bzopen(fn, "r");
    } else {
        fp = open(fn, O_RDONLY);
    }
    if (fp < 0 && fpgz == Z_NULL && fpbz == NULL) {
        alder_loge(ALDER_ERR_FILE, "cannot open file %s - %s",
                   fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    if (o->type_infile & ALDER_FILETYPE_GZ) {
        o->fx = fpgz;
    } else if (o->type_infile & ALDER_FILETYPE_BZ) {
        o->fx = fpbz;
    } else {
        o->fx = (void *)(intptr_t)fp;
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function returns 0,1,2,3 for DNA.
 *
 *  @param e encoder
 *
 *  @return int
 *  0,1,2,3 for A,C,T,G.
 *  4 for others.
 *  5 for the end of buffer.
 */
static int
alder_kmer_encode5_token(size_t *curbuf, uint8_t *inbuf, size_t lenbuf,
                         int type_infile)
{
    char c;
    size_t vcurbuf = *curbuf;
#if !defined(NDEBUG)
    if (vcurbuf % 10000 == 0) {
        alder_log4("i_inbuf(%zu): %zu", lenbuf, vcurbuf);
    }
#endif
    if (vcurbuf < lenbuf) {
        c = inbuf[vcurbuf++];
    } else {
        //        assert(0);
        *curbuf = vcurbuf;
        return 5;
    }
    if (type_infile == ALDER_KMER_ENCODER5_FILETYPE_FASTA) {
        if (c == '>' || c == '\n') {
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            if (c == '>') {
                uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
                vcurbuf = (size_t)(cinbuf - inbuf + 1);
            }
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            if (vcurbuf < lenbuf) {
                c = inbuf[vcurbuf++];
            } else {
                //                e->n_byte += *curbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    } else if (type_infile == ALDER_KMER_ENCODER5_FILETYPE_FASTQ) {
        if (c == '@') {
            // header
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            c = inbuf[vcurbuf++];
            assert(c != '\n');
        } else if (c == '\n') {
            // +
            c = inbuf[vcurbuf++];
            assert(c == '+');
            
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            // quality score
            //            c = inbuf[vcurbuf++];
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            if (vcurbuf < lenbuf) {
                c = 'X';
            } else {
                //                e->n_byte += e->i_inbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    } else {
        assert(type_infile == ALDER_KMER_ENCODER5_FILETYPE_SEQ);
        if (c == '\n') {
            if (vcurbuf < lenbuf) {
                // no code.
            } else {
                //                e->n_byte += e->i_inbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    }
    
    *curbuf = vcurbuf;
    return dna_char2int[c];
    //    int ci = alder_dna_char2int(c);
    //    return ci;
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
alder_kmer_encode5(int n_encoder,
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
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile,
                   const char *outdir,
                   const char *outfile)
{
    assert(n_encoder >= 1);
    alder_log5("preparing encoding Kmers...");
    encoder_id_counter = 0;
    int s = ALDER_STATUS_SUCCESS;
    int n_thread = n_encoder + 0;
    /* Create variables for the threads. */
    alder_kmer_encoder5_t *data =
    alder_kmer_encoder5_create(n_encoder,
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
                               progress_flag,
                               progressToError_flag,
                               infile,
                               outdir,
                               outfile);
    alder_log5("creating %d threads: one for reader, and one for writer",
               n_thread);
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    memset(threads, 0, sizeof(*threads)*n_thread);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY,
                   "failed to create fileseq_kmer_thread_readwriter");
        XFREE(threads);
        alder_kmer_encoder5_destroy(data);
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
    for (int i = 0; i < n_thread; i++) {
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
    *n_byte = data->n_byte;
    alder_kmer_encoder5_destroy(data);
    alder_log5("Encoding Kmer has been finished with %d threads.", n_thread);
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
 *  This function returns 0,1,2 depending on the type of an input file.
 *
 *  @param a encoder
 *
 *  @return 0 for FASTA, 1 for FASTQ, 2 otherwise.
 */
static uint8_t what_type_infile(alder_kmer_encoder5_t *a)
{
    uint8_t type_infile;
    if (a->type_infile & ALDER_FILETYPE_ISFASTA) {
        type_infile = ALDER_KMER_ENCODER5_FILETYPE_FASTA;
    } else if (a->type_infile & ALDER_FILETYPE_ISFASTQ) {
        type_infile = ALDER_KMER_ENCODER5_FILETYPE_FASTQ;
    } else {
        type_infile = ALDER_KMER_ENCODER5_FILETYPE_BINARY;
//        type_infile = ALDER_KMER_ENCODER5_FILETYPE_SEQ;
    }
    return type_infile;
}

#pragma mark thread

/**
 *  This funciton writes the output buffer to partition files.
 *
 *  @param a          encoder
 *  @param encoder_id encoder id
 *
 *  ---------------------------------------------------------------------------
 *
 *  outbuf: n_encoder x [suboutbuf]
 *
 *  outbuf2: n_np x [suboutbuf2]
 *  This is
 *     A B  C     D         E     F
 *  X: 8 8 [bbb] [fffffff] [ccc] [aaa]
 *  A - size of filled buffer in D + E
 *  B - size of buffer in F
 *  C - potentially prepended buffer from F (could be from other encoder)
 *  D - data filled by the current encoder
 *  E - data overflowed by the current encoder
 *  F - remaining buffer
 *  S - start of buffer to write
 *  Q - (CDE or total buffer size - written size)
 *
 *  lenbuf: 1
 *  x_lenbuf: 2 in the other encoder
 *  lock only one partition not all of them.
 */
//#define VERSION4_DEV
static size_t write_to_partition_file(alder_kmer_encoder5_t *a,
                                      int encoder_id,
                                      int partition_id)
{
#if defined(VERSION4_DEV)
    uint8_t *debug_a = malloc(32);
    for (uint8_t i = 0; i < 32; i++) {
        debug_a[i] = i;
    }
#endif
    pthread_mutex_lock(&mutex_write);
    
    uint8_t *outbuf = a->outbuf + encoder_id * a->size_suboutbuf;
    uint8_t *outbuf2 = outbuf + partition_id * a->size_suboutbuf2;
    uint8_t *A_outbuf2 = outbuf2;
    uint8_t *B_outbuf2 = outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_B;
    uint8_t *C_outbuf2 = outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_C;
    uint8_t *D_outbuf2 = outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_C + a->b;
    uint8_t *E_outbuf2 = outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_C + a->b + a->size_fixedoutbuffer;
    uint8_t *F_outbuf2 = outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_C + a->b + a->size_fixedoutbuffer + a->b;
    size_t DE_lenbuf = to_size(A_outbuf2, 0);
#if defined(VERSION4_DEV)
    alder_loga("I", debug_a, 32);
    alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
    
    //
    int x_encoder = a->encoder_remaining_outbuf[partition_id];
    uint8_t *O_outbuf = a->outbuf + x_encoder * a->size_suboutbuf;
    uint8_t *O_outbuf2 = O_outbuf + partition_id * a->size_suboutbuf2;
    uint8_t *OB_outbuf2 = O_outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_B;
    uint8_t *OF_outbuf2 = O_outbuf2 + ALDER_KMER_ENCODER5_OUTBUFFER_C + a->b + a->size_fixedoutbuffer + a->b;
    size_t OF_lenbuf = to_size(OB_outbuf2, 0);
    
    // Copy any remaining buffer from other encoder or itself.
    if (OF_lenbuf > 0) {
#if defined(VERSION4_DEV)
        memset(C_outbuf2, 0xAA, a->b);
        alder_loga("I", debug_a, 32);
        alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
        memcpy(C_outbuf2, OF_outbuf2, a->b);
        memset(OF_outbuf2, 0, a->b);
#if defined(VERSION4_DEV)
        alder_loga("I", debug_a, 32);
        alder_loga("O", O_outbuf2, a->size_suboutbuf2);
#endif
    }
    
    //
    size_t CDE_lenbuf = OF_lenbuf + DE_lenbuf;
    uint8_t *S_outbuf2 = D_outbuf2 - OF_lenbuf;
    if (CDE_lenbuf > a->size_fixedoutbuffer) {
        //
        int fd = fileno(a->fpout[partition_id]);
        ssize_t s = write(fd, S_outbuf2, a->size_fixedoutbuffer);
        assert(s == a->size_fixedoutbuffer);
#if defined(VERSION4_DEV)
        memset(S_outbuf2, 0xFF, a->size_fixedoutbuffer);
        alder_loga("I", debug_a, 32);
        alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
        //
        size_t Q_lenbuf = CDE_lenbuf - a->size_fixedoutbuffer;
        size_t R_lenbuf = Q_lenbuf;
        uint8_t *R_outbuf = S_outbuf2 + a->size_fixedoutbuffer;
        uint8_t *Q_outbuf = R_outbuf;
        if (Q_lenbuf >= a->b) {
            R_lenbuf = Q_lenbuf - a->b;
        }
        
        if (R_lenbuf > 0) {
            // Copy the remaining ot F.
            uint8_t *FR_outbuf2 = F_outbuf2 + a->b - R_lenbuf;
#if defined(VERSION4_DEV)
            memset(FR_outbuf2, 0xBB, R_lenbuf);
            alder_loga("I", debug_a, 32);
            alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
            memcpy(FR_outbuf2, R_outbuf, R_lenbuf);
            
#if defined(VERSION4_DEV)
            memset(F_outbuf2, 0xCC, a->b - R_lenbuf);
            alder_loga("I", debug_a, 32);
            alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
            
            memset(F_outbuf2, 0, a->b - R_lenbuf);
            
            to_size(B_outbuf2,0) = R_lenbuf;
#if defined(VERSION4_DEV)
            memset(R_outbuf, 0xBB, R_lenbuf);
            alder_loga("I", debug_a, 32);
            alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
            Q_outbuf += R_lenbuf;
        } else {
            to_size(B_outbuf2,0) = 0;
        }
        
        if (Q_lenbuf >= a->b) {
            // Copy the b bytes to the start of the buffer.
#if defined(VERSION4_DEV)
            memset(D_outbuf2, 0xAA, a->b);
            alder_loga("I", debug_a, 32);
            alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
            memcpy(D_outbuf2, Q_outbuf, a->b);
            
            to_size(A_outbuf2,0) = a->b;
#if defined(VERSION4_DEV)
            memset(Q_outbuf, 0xAA, a->b);
            alder_loga("I", debug_a, 32);
            alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
        } else {
            to_size(A_outbuf2,0) = 0;
        }
        
#if defined(VERSION4_DEV)
        alder_loga("I", debug_a, 32);
        alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
        
    } else {
        //
        ssize_t s = write(fileno(a->fpout[partition_id]), S_outbuf2, CDE_lenbuf);
        assert(s == CDE_lenbuf);
        to_size(A_outbuf2,0) = 0;
        to_size(B_outbuf2,0) = 0;
    }
    a->encoder_remaining_outbuf[partition_id] = encoder_id;
    memset(E_outbuf2, 0, a->b);
    
#if defined(VERSION4_DEV)
    alder_loga("I", debug_a, 32);
    alder_loga("A", A_outbuf2, a->size_suboutbuf2);
#endif
    
    pthread_mutex_unlock(&mutex_write);
//    alder_kmer_encoder5_unlock_writer(a, partition_id);
    
#if defined(VERSION4_DEV)
    free(debug_a);
#endif
    return to_size(A_outbuf2,0);
}

static int read_from_sequence_file(alder_kmer_encoder5_t *a, int encoder_id)
{
    int s;
//    alder_kmer_encoder5_lock_reader(a, encoder_id);
    pthread_mutex_lock(&mutex_read);
    
    size_t c_inbuffer = encoder_id;
    
    if (a->reader_lenbuf == 0) {
        if (a->reader_i_infile > 0) {
            close_infile(a);
        }
        if (a->reader_i_infile < a->infile->qty) {
            s = open_infile(a, a->reader_i_infile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->reader_type_infile = what_type_infile(a);
            a->reader_i_infile++;
            //            fprintf(stdout, "read_i_infile: %d\n", a->reader_i_infile);
            a->reader_lenbuf2 = 0;
        } else {
            // No more input file to read.
            // I may still have some outbuf.
            // I would call writer_to_partition_file after while-loop
            // in encoder function.
            pthread_mutex_unlock(&mutex_read);
//            alder_kmer_encoder5_unlock_reader(a, encoder_id);
            return (int)a->n_encoder;
        }
    }
    assert(a->reader_i_infile <= a->infile->qty);
    
    if (a->reader_type_infile == ALDER_KMER_ENCODER5_FILETYPE_BINARY) {
        //
        uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
        inbuf[ALDER_KMER_ENCODER5_INBUFFER_TYPE_INFILE] = a->reader_type_infile;
        uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER5_INBUFFER_BODY;
//        size_t size_body = a->size_subinbuf - ALDER_KMER_ENCODER5_INBUFFER_BODY;
        a->reader_lenbuf = (size_t)read((int)((intptr_t)a->fx), inbuf_body, BUFSIZE);
        to_size(inbuf, ALDER_KMER_ENCODER5_INBUFFER_CURRENT) = (size_t)0;
        to_size(inbuf, ALDER_KMER_ENCODER5_INBUFFER_LENGTH) = a->reader_lenbuf;
        
        alder_log4("encoder(%d): read %zu (MB)", encoder_id, a->reader_lenbuf >> 20);
    } else {
        uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
        inbuf[ALDER_KMER_ENCODER5_INBUFFER_TYPE_INFILE] = a->reader_type_infile;
        uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER5_INBUFFER_BODY;
        size_t size_body = a->size_subinbuf - ALDER_KMER_ENCODER5_INBUFFER_BODY;
        assert(size_body == BUFSIZE + ALDER_KMER_SECONDARY_BUFFER_SIZE);
        s = alder_fileseq_chunk(&a->reader_lenbuf,
                                (char*)inbuf_body,
                                size_body,
                                &a->reader_lenbuf2,
                                (char*)a->inbuf2,
                                a->size_inbuf2,
                                a->k, NULL, a->fx, a->type_infile);
        if (s == ALDER_STATUS_SUCCESS) {
            assert(a->reader_lenbuf > 0);
        } else {
            assert(a->reader_lenbuf == 0);
        }
        to_size(inbuf, ALDER_KMER_ENCODER5_INBUFFER_CURRENT) = (size_t)0;
        to_size(inbuf, ALDER_KMER_ENCODER5_INBUFFER_LENGTH) = a->reader_lenbuf;
        
        alder_log4("encoder(%d): read %zu (MB)", encoder_id, a->reader_lenbuf >> 20);
    }
    
    
    /* Progress */
    a->n_byte += a->reader_lenbuf;
    if (a->progress_flag) {
        alder_progress_step(a->n_byte, a->totalFilesize,
                            a->progressToError_flag);
    }
    
    pthread_mutex_unlock(&mutex_read);
//    alder_kmer_encoder5_unlock_reader(a, encoder_id);
    return encoder_id;
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
    alder_log("encoder(%d): START", encoder_id);
    
    //    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_encoder5_t *a = (alder_kmer_encoder5_t*)t;
    assert(a != NULL);
    
    alder_encode_number2_t * s1 = NULL;
    alder_encode_number2_t * s2 = NULL;
    int K = a->k;
    s1 = alder_encode_number2_createWithKmer(K);
    s2 = alder_encode_number2_createWithKmer(K);
    size_t *len_outbuf2 = malloc(sizeof(*len_outbuf2) * a->n_np);
    memset(len_outbuf2, 0, sizeof(*len_outbuf2) * a->n_np);
    struct libdivide_u64_t fast_ni = libdivide_u64_gen(a->n_ni);
    struct libdivide_u64_t fast_np = libdivide_u64_gen(a->n_np);
    
    alder_kmer_binary_stream_t *bs = alder_kmer_binary_buffer_create();
    
    size_t ib = s1->b / 8;
    size_t jb = s1->b % 8;
    uint64_t ni = a->n_ni;
    uint64_t np = a->n_np;
    size_t c_inbuffer = a->n_encoder;
    size_t c_outbuffer = encoder_id;
    
    size_t debug_counter = 0;
    
    /* Reader variables */
    alder_encode_number2_t * m1 = NULL;
    alder_encode_number2_t * m2 = NULL;
    m1 = alder_encode_number2_createWithKmer(K);
    m2 = alder_encode_number2_createWithKmer(K);
    //    int n_boundary = (a->k / ALDER_NUMKMER_8BYTE);
    
    /* Setup of outbuf */
    uint8_t *outbuf = a->outbuf + encoder_id * a->size_suboutbuf;
    uint64_t full_partition = a->n_np;
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_outbuffer == a->n_encoder) {
            assert(full_partition < a->n_np);
            len_outbuf2[full_partition] = write_to_partition_file(a, encoder_id,
                                                                  (int)full_partition);
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
        uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
        //        assert(*inbuf == ALDER_KMER_ENCODER5_ENCODER_USE_INBUF);
        int type_infile = (int)inbuf[ALDER_KMER_ENCODER5_INBUFFER_TYPE_INFILE];
        assert(type_infile == ALDER_KMER_ENCODER5_FILETYPE_BINARY ||
               type_infile == ALDER_KMER_ENCODER5_FILETYPE_FASTQ ||
               type_infile == ALDER_KMER_ENCODER5_FILETYPE_FASTA ||
               type_infile == ALDER_KMER_ENCODER5_FILETYPE_SEQ);
        
        size_t curbuf = to_size(inbuf,ALDER_KMER_ENCODER5_INBUFFER_CURRENT);
        size_t lenbuf = to_size(inbuf,ALDER_KMER_ENCODER5_INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER5_INBUFFER_BODY;
        /* setup of outbuf */
        
        full_partition = a->n_np;
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
        while (token < 5 && full_partition == a->n_np) {
            
            if (token == 4) {
                // Create a number.
                token = 0;
                alder_log5("A starting Kmer is being created.");
                for (int i = 0; i < a->k - 1; i++) {
                    
//                    token = alder_kmer_encode5_token(&curbuf, inbuf_body,
//                                                     lenbuf, type_infile);
                    token = alder_kmer_binary_buffer_read(bs,NULL);
//debug_counter++;
//printf("[%zu] %d\n", debug_counter++, token);
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
//            token = alder_kmer_encode5_token(&curbuf, inbuf_body,
//                                             lenbuf, type_infile);
            token = alder_kmer_binary_buffer_read(bs,NULL);
//debug_counter++;
//            if (debug_counter == 1472937) {
//                printf("STOP HERE: %zu\n",debug_counter);
//            }
//printf("[%zu] %d\n", debug_counter++, token);
            if (token >= 4) continue;
            alder_log5("token: %d", token);
//            printf("[%zu] %d\n", debug_counter, token);
            int b1 = token;
            int b2 = (b1 + 2) % 4;
            alder_encode_number2_shiftLeftWith(s1,b1);
            alder_encode_number2_shiftRightWith(s2,b2);
            
            
            
            //#define ALDERDEBUGENCODE
            
            /*****************************************************************/
            /*                         OPTIMIZATION                          */
            /*****************************************************************/
            /* 5. Select partition, iteration, and kmer of the two. */
            
            alder_encode_number2_t *ss = NULL;
            uint64_t i_ni = 0;
            uint64_t i_np = 0;
            //            alder_hashtable_mcas_select2(&ss, &i_ni, &i_np, s1, s2,
            //                                         ni, np);
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
                uint8_t *D_outbuf2 = (outbuf + i_np * a->size_suboutbuf2 +
                                      ALDER_KMER_ENCODER5_OUTBUFFER_C + a->b);
                
                
                /* Encode the Kmer ss. */
                size_t x = len_outbuf2[i_np];
                uint64_t *D_outbuf2_uint64 = (uint64_t*)(D_outbuf2 + x);
                for (size_t i = 0; i < ib; i++) {
                    D_outbuf2_uint64[i] = ss->n[i].key64;
                    x += 8;
                    
                }
                for (size_t j = 0; j < jb; j++) {
                    D_outbuf2[x++] = ss->n[ib].key8[j];
                }
                len_outbuf2[i_np] = x;
                //                to_size(D_outbuf2, x);
//                debug_counter++;
//                printf("[%zu] %zu\n", debug_counter, x);
                
                // CHECK THIS
                if (a->size_fixedoutbuffer <= x) {
                    uint8_t *A_outbuf2 = outbuf + i_np * a->size_suboutbuf2;
                    full_partition = i_np;
                    to_size(A_outbuf2,0) = x; // Only this partition.
                    break;
                }
                assert(a->size_fixedoutbuffer > len_outbuf2[i_np]);
            }
        }
        
        //        // outbuf size
        //        for (int i_np = 0; i_np < np; i_np++) {
        //            uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
        //            to_size(outbuf2, 0) = len_outbuf2[i_np];
        //        }
        
        /* 9. Mark the status if inbuf. */
        if (token == 5) {
            // empty inbuf - read more input
            c_inbuffer = a->n_encoder;
            alder_log2("encoder(): inbuf is empty.");
        } else {
            // partial inbuf
            // Save the current buffer position for later use.
            to_size(inbuf,ALDER_KMER_ENCODER5_INBUFFER_CURRENT) = curbuf;
            alder_log2("encoder(): inbuf is not yet empty, more inbuf.");
        }
        if (full_partition < a->n_np) {
            // full outbuf
            c_outbuffer = a->n_encoder;
            alder_log2("encoder(): outbuf is full.");
        } else {
            // partial outbuf
            alder_log2("encoder(): outbuf is not yet full, more space.");
        }
    }
    /* Flush any remaining output buffer. */
    for (uint64_t i_np = 0; i_np < a->n_np; i_np++) {
        uint8_t *A_outbuf2 = outbuf + i_np * a->size_suboutbuf2;
        to_size(A_outbuf2,0) = len_outbuf2[i_np];
        assert(len_outbuf2[i_np] < a->size_fixedoutbuffer);
        len_outbuf2[i_np] = write_to_partition_file(a, encoder_id, (int)i_np);
        assert(len_outbuf2[i_np] == 0);
    }
    
    alder_kmer_binary_buffer_destroy(bs);
    alder_encode_number2_destroy(m1);
    alder_encode_number2_destroy(m2);
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    XFREE(len_outbuf2);
    alder_log("encoder(%d): END", encoder_id);
    pthread_exit(NULL);
}













