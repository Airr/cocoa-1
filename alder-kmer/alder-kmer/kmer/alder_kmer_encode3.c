/**
 * This file, alder_kmer_encode3.c, is part of alder-kmer.
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
#include "alder_fileseq_streamer.h"
#include "alder_progress.h"
#include "alder_dna.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_encode.h"
#include "alder_kmer_encode3.h"

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
 *
 *  Variables
 *  ---------
 *  c_inbuffer and c_outbuffer are indices for sub buffers. The size of each
 *  sub buffer is denoted by size_subinbuf and size_suboutbuf.
 */

static int encoder_id_counter = 0;

#define ALDER_KMER_SECONDARY_BUFFER_SIZE         1000
#define ALDER_KMER_ENCODER3_OUTBUFFER_HEADER     1
#define ALDER_KMER_ENCODER3_OUTSUBBUFFER_HEADER  8

#define ALDER_KMER_ENCODER3_INBUFFER_TYPE_INFILE 1
#define ALDER_KMER_ENCODER3_INBUFFER_CURRENT     2
#define ALDER_KMER_ENCODER3_INBUFFER_LENGTH      10
#define ALDER_KMER_ENCODER3_INBUFFER_BODY        18
#define ALDER_KMER_ENCODER3_FILETYPE_FASTA       1
#define ALDER_KMER_ENCODER3_FILETYPE_FASTQ       2
#define ALDER_KMER_ENCODER3_FILETYPE_SEQ         3

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
alder_kmer_encoder3_destroy (alder_kmer_encoder3_t *o)
{
    if (o != NULL) {
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
                o->fpout[j] = NULL;
            }
            XFREE(o->fpout);
        }
        XFREE(o->outbuf);
        XFREE(o->inbuf2);
        XFREE(o->inbuf);
        bdestroy(o->dout);
        XFREE(o);
    }
}

static alder_kmer_encoder3_t *
alder_kmer_encoder3_create(int n_encoder,
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
    
    alder_kmer_encoder3_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create fileseq_kmer_thread_readwriter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->k = kmer_size;
    o->n_ni = n_iteration;
    o->n_np = n_partition;
    o->i_ni = i_iteration;
    o->n_encoder = n_encoder;
    o->lock_reader = ALDER_KMER_ENCODER3_READER_UNLOCK;
    o->lock_writer = ALDER_KMER_ENCODER3_WRITER_UNLOCK;
    o->reader_lenbuf = 0;
    o->reader_lenbuf2 = 0;
    o->reader_type_infile = 0;
    o->reader_i_infile = 0;
    o->n_subbuf = n_encoder;
    o->size_inbuf2 = ALDER_KMER_SECONDARY_BUFFER_SIZE;
    o->size_subinbuf = ALDER_KMER_ENCODER3_INBUFFER_BODY + BUFSIZE + o->size_inbuf2;
    o->size_inbuf = o->size_subinbuf * o->n_subbuf;
    o->size_outbuf = (size_t)(memory_available << 20);
    
    o->size_suboutbuf = o->size_outbuf / o->n_subbuf;
    o->size_suboutbuf2 = (o->size_suboutbuf -  ALDER_KMER_ENCODER3_OUTBUFFER_HEADER) / n_partition;
    o->size_suboutbuf2_body = o->size_suboutbuf2 - ALDER_KMER_ENCODER3_OUTSUBBUFFER_HEADER;
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
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);    /* bigmem */
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * o->size_inbuf2);
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf); /* bigmem */
    if (o->dout == NULL ||
        o->inbuf == NULL ||
        o->inbuf2 == NULL ||
        o->outbuf == NULL) {
        alder_kmer_encoder3_destroy(o);
        return NULL;
    }
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->inbuf2, 0, sizeof(*o->inbuf2) * o->size_inbuf2);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);
    
    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_encoder3_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    
    o->fpout = malloc(sizeof(*o->fpout)*n_partition);
    if (o->fpout == NULL) {
        alder_kmer_encoder3_destroy(o);
        return NULL;
    }
    memset(o->fpout, 0, sizeof(*o->fpout)*n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfpar = bformat("%s/%s-%d-%d.par",
                                outdir, outfile, i_iteration, i);
        if (bfpar == NULL) {
            alder_kmer_encoder3_destroy(o);
            return NULL;
        }
        o->fpout[i] = fopen(bdata(bfpar), "wb");
        if (o->fpout[i] == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to make a file: %s",
                       bdata(bfpar));
            bdestroy(bfpar);
            alder_kmer_encoder3_destroy(o);
            return NULL;
        }
        bdestroy(bfpar);
    }
    
    /* init */
    //    size_t x = o->size_inbuf / o->n_subbuf;
    /* buffer state */
    
//    for (size_t i = 0; i < o->n_subbuf; i++) {
//        uint8_t *outbuf = o->outbuf + i * o->size_suboutbuf;
//        *outbuf = ALDER_KMER_ENCODER3_EMPTY_OUTBUF;
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
alder_kmer_encoder3_destroy_with_error (alder_kmer_encoder3_t *o, int s)
{
    if (s != 0) {
        alder_loge(s, "cannot create fileseq_kmer_thread_readwriter");
    }
    alder_kmer_encoder3_destroy(o);
}

/**
 *  This function closes a partition file.
 *
 *  @param o        encoder
 *
 *  @return SUCCESS or FAIL
 */
static int close_infile (alder_kmer_encoder3_t *o)
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
static int open_infile (alder_kmer_encoder3_t *o, int i_infile)
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
alder_kmer_encode3_token(size_t *curbuf, uint8_t *inbuf, size_t lenbuf,
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
    if (type_infile == ALDER_KMER_ENCODER3_FILETYPE_FASTA) {
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
    } else if (type_infile == ALDER_KMER_ENCODER3_FILETYPE_FASTQ) {
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
        assert(type_infile == ALDER_KMER_ENCODER3_FILETYPE_SEQ);
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
alder_kmer_encode3(int n_encoder,
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
    alder_kmer_encoder3_t *data =
    alder_kmer_encoder3_create(n_encoder,
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
        alder_kmer_encoder3_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    /* Initialize mutex and condition variable objects */
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
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
    XFREE(threads);
    *n_byte = data->n_byte;
    alder_kmer_encoder3_destroy(data);
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
static uint8_t what_type_infile(alder_kmer_encoder3_t *a)
{
    uint8_t type_infile;
    if (a->type_infile & ALDER_FILETYPE_ISFASTA) {
        type_infile = ALDER_KMER_ENCODER3_FILETYPE_FASTA;
    } else if (a->type_infile & ALDER_FILETYPE_ISFASTQ) {
        type_infile = ALDER_KMER_ENCODER3_FILETYPE_FASTQ;
    } else {
        type_infile = ALDER_KMER_ENCODER3_FILETYPE_SEQ;
    }
    return type_infile;
}

#pragma mark thread

/**
 *  This funciton writes the output buffer to partition files.
 *
 *  @param a          encoder
 *  @param encoder_id encoder id
 */
static int write_to_partition_file(alder_kmer_encoder3_t *a, int encoder_id)
{
    size_t c_outbuffer;
    alder_kmer_encoder_lock_writer(a);
    c_outbuffer = encoder_id;
    uint8_t *outbuf = (a->outbuf + c_outbuffer * a->size_suboutbuf +
                       ALDER_KMER_ENCODER3_OUTBUFFER_HEADER);
    for (int i_np = 0; i_np < a->n_np; i_np++) {
        // Write the content.
        uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
        uint8_t *outbuf2_body = outbuf2 + ALDER_KMER_ENCODER3_OUTSUBBUFFER_HEADER;
        size_t lenbuf = to_size(outbuf2, 0);
        if (lenbuf > 0) {
            size_t s = fwrite(outbuf2_body, sizeof(*outbuf2_body), lenbuf,
                              a->fpout[i_np]);
            if (s != lenbuf) {
                // Error in writing.
                assert(0);
            }
            to_size(outbuf2,0) = 0;
        }
    }
    alder_kmer_encoder_unlock_writer(a);
    return encoder_id;
}

static int read_from_sequence_file(alder_kmer_encoder3_t *a, int encoder_id)
{
    int s;
    alder_kmer_encoder_lock_reader(a, encoder_id);
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
            alder_kmer_encoder_unlock_reader(a, encoder_id);
            return (int)a->n_subbuf;
        }
    }
    assert(a->reader_i_infile <= a->infile->qty);
    
    uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
    inbuf[ALDER_KMER_ENCODER3_INBUFFER_TYPE_INFILE] = a->reader_type_infile;
    uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER3_INBUFFER_BODY;
    size_t size_body = a->size_subinbuf - ALDER_KMER_ENCODER3_INBUFFER_BODY;
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
    to_size(inbuf, ALDER_KMER_ENCODER3_INBUFFER_CURRENT) = (size_t)0;
    to_size(inbuf, ALDER_KMER_ENCODER3_INBUFFER_LENGTH) = a->reader_lenbuf;
    
    alder_log4("encoder(%d): read %zu (MB)", encoder_id, a->reader_lenbuf >> 20);
    
    
    /* Progress */
    a->n_byte += a->reader_lenbuf;
    if (a->progress_flag) {
        alder_progress_step(a->n_byte, a->totalFilesize,
                            a->progressToError_flag);
    }
    
    alder_kmer_encoder_unlock_reader(a, encoder_id);
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
    alder_kmer_encoder3_t *a = (alder_kmer_encoder3_t*)t;
    assert(a != NULL);
    
    alder_encode_number2_t * s1 = NULL;
    alder_encode_number2_t * s2 = NULL;
    int K = a->k;
    s1 = alder_encode_number2_createWithKmer(K);
    s2 = alder_encode_number2_createWithKmer(K);
    size_t *len_outbuf2 = malloc(sizeof(*len_outbuf2) * a->n_np);
    struct libdivide_u64_t fast_ni = libdivide_u64_gen(a->n_ni);
    struct libdivide_u64_t fast_np = libdivide_u64_gen(a->n_np);
    
    
    size_t ib = s1->b / 8;
    size_t jb = s1->b % 8;
    uint64_t ni = a->n_ni;
    uint64_t np = a->n_np;
    size_t c_inbuffer = a->n_subbuf;
    size_t c_outbuffer = encoder_id;
    
    size_t debug_counter = 0;
    
    /* Reader variables */
    
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_outbuffer == a->n_subbuf) {
            c_outbuffer = write_to_partition_file(a, encoder_id);
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
        // If I need a new input buffer, I'd set c_inbuffer to a->n_subbuf,
        // which is essentially n_encoder.
        if (c_inbuffer == a->n_subbuf) {
            c_inbuffer = read_from_sequence_file(a, encoder_id);
            if (c_inbuffer == a->n_subbuf) {
#if defined(ALDER_THREAD)
                alder_log("encoder(%d): BREAK! lock(%d).", encoder_id, a->lock_reader);
#endif
                break;
            }
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Encoder
        ///////////////////////////////////////////////////////////////////////
        assert(c_inbuffer < a->n_subbuf);
        assert(c_outbuffer < a->n_subbuf);
        
        //        if (c_inbuffer < a->n_subbuf && c_outbuffer < a->n_subbuf) {
        /* setup of inbuf */
        uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
//        assert(*inbuf == ALDER_KMER_ENCODER3_ENCODER_USE_INBUF);
        int type_infile = (int)inbuf[ALDER_KMER_ENCODER3_INBUFFER_TYPE_INFILE];
        assert(type_infile == ALDER_KMER_ENCODER3_FILETYPE_FASTA ||
               type_infile == ALDER_KMER_ENCODER3_FILETYPE_FASTQ ||
               type_infile == ALDER_KMER_ENCODER3_FILETYPE_SEQ);
        
        size_t curbuf = to_size(inbuf,ALDER_KMER_ENCODER3_INBUFFER_CURRENT);
        size_t lenbuf = to_size(inbuf,ALDER_KMER_ENCODER3_INBUFFER_LENGTH);
        uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER3_INBUFFER_BODY;
        /* setup of outbuf */
        uint8_t *outbuf = a->outbuf + c_outbuffer * a->size_suboutbuf;
//        assert(*outbuf == ALDER_KMER_ENCODER3_ENCODER_USE_OUTBUF);
        outbuf += ALDER_KMER_ENCODER3_OUTBUFFER_HEADER;
        for (int i_np = 0; i_np < np; i_np++) {
            uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
            len_outbuf2[i_np] = to_size(outbuf2, 0);
        }
        
        bool isFullOutbuf = false;
        int token;
        if (curbuf == 0) {
            alder_log3("encoder(%d): fresh new input buffer");
            token = 4; // Start with a new Kmer.
        } else {
            alder_log3("encoder(%d): resuming where it stopped");
            token = 0; // Continue from where I stopped.
        }
        
        /* 3. While either input is not empty or output is not full: */
        while (token < 5 && isFullOutbuf == false) {
            if (token == 4) {
                // Create a number.
                token = 0;
                alder_log5("A starting Kmer is being created.");
                for (int i = 0; i < a->k - 1; i++) {
                    debug_counter++;
//                    if (debug_counter == 127367427) {
//                        for (int ix = 0; ix < 1000; ix++) {
//                            fputc(inbuf_body[ix], stdout);
//                        }
//                    }
                    token = alder_kmer_encode3_token(&curbuf, inbuf_body,
                                                     lenbuf, type_infile);
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
            debug_counter++;
            token = alder_kmer_encode3_token(&curbuf, inbuf_body,
                                             lenbuf, type_infile);
            if (token >= 4) continue;
            alder_log5("token: %d", token);
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
            
            
            
#if defined(ALDERDEBUGENCODE)
            alder_log4("Kmer");
            alder_encode_number2_log(s1);
            alder_encode_number2_log(s2);
            alder_encode_number2_log(ss);
#endif
            /* 6. Save a chosen one in the output buffers. */
            if (i_ni == a->i_ni) {
//                uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
                uint8_t *outbuf2_body = (outbuf + i_np * a->size_suboutbuf2 +
                                         ALDER_KMER_ENCODER3_OUTSUBBUFFER_HEADER);
                
                /* Encode the Kmer ss. */
                size_t x = len_outbuf2[i_np];
                uint64_t *outbuf2_uint64 = (uint64_t*)(outbuf2_body + x);
                for (size_t i = 0; i < ib; i++) {
                    outbuf2_uint64[i] = ss->n[i].key64;
                    
//                    for (size_t j = 0; j < 8; j++) {
//                        assert(outbuf2_body[x] == ss->n[i].key8[j]);
//                        x++;
//                    }
                    x += 8;
                    
                }
                for (size_t j = 0; j < jb; j++) {
                    outbuf2_body[x++] = ss->n[ib].key8[j];
                }
                len_outbuf2[i_np] = x;
                
                if (a->size_suboutbuf2_body < len_outbuf2[i_np] + ss->b) {
                    isFullOutbuf = true;
                    break;
                }
                assert(a->size_suboutbuf2_body > len_outbuf2[i_np]);
            }
            
#if defined(ALDERDEBUGENCODE)
            for (size_t i_outbuf = 0; i_outbuf < a->n_subbuf; i_outbuf++) {
                uint8_t x = a->outbuf[i_outbuf * a->size_suboutbuf];
                assert(x == ALDER_KMER_ENCODER3_FULL_OUTBUF ||
                       x == ALDER_KMER_ENCODER3_WRITER_USE_OUTBUF ||
                       x == ALDER_KMER_ENCODER3_EMPTY_OUTBUF ||
                       x == ALDER_KMER_ENCODER3_ENCODER_USE_OUTBUF);
            }
#endif
        }
        
        // outbuf size
        for (int i_np = 0; i_np < np; i_np++) {
            uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
            to_size(outbuf2, 0) = len_outbuf2[i_np];
        }
        
        /* 9. Mark the status if inbuf. */
        if (token == 5) {
            // empty inbuf
            c_inbuffer = a->n_subbuf;
            alder_log2("encoder(): inbuf is empty.");
        } else {
            // partial inbuf
            // Save the current buffer position for later use.
            to_size(inbuf,ALDER_KMER_ENCODER3_INBUFFER_CURRENT) = curbuf;
            alder_log2("encoder(): inbuf is not yet empty, more inbuf.");
        }
        if (isFullOutbuf == true) {
            // full outbuf
            c_outbuffer = a->n_subbuf;
            alder_log2("encoder(): outbuf is full.");
        } else {
            // partial outbuf
            alder_log2("encoder(): outbuf is not yet full, more space.");
        }
    }
    /* Flush any remaining output buffre. */
    write_to_partition_file(a, encoder_id);
    
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    XFREE(len_outbuf2);
    alder_log("encoder(%d): END", encoder_id);
    pthread_exit(NULL);
}


