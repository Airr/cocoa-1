/**
 * This file, alder_kmer_encode2.c, is part of alder-kmer.
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

/**
 *  The state of buffer
 *  -------------------
 *  Initially, inbuf and outbuf are EMPTY. 
 *  1. Reader can change the input buffer state:
 *     EMPTY -> USE        by select_empty_inbuf
 *     USE   -> FULL       by deselect_full_inbuf
 *     USE   -> EMPTY      by deselect_empty_inbuf
 *     EMPTY -> FINISHED   by finish_empty_inbuf
 *  2. Writer can change the output buffer state:
 *     FULL  -> USE        by select_full_output
 *     USE   -> EMPTY      by deselect_empty_output
 *  3. Encoder can change both input and output buffers:
 *     FULL_INBUF   -> USE by retain_inbuf
 *     EMPTY_OUTBUF -> USE by retain_outbuf
 *     USE  -> EMPTY_INBUF by release_inbuf
 *     USE  -> FULL_OUTBUF by release_outbuf
 *
 *  Buffer states by encoder
 *  ------------------------
 *  1. At the start, both inbuf and outbuf are not retained. It tries to retain
 *     both.
 *  2. After a run, it'd be likely that either inbuf or outbuf is done; three
 *     possible scenarios.
 *  3. Inbuf is done (inbuf is empty), and outbuf is still remaining (partial).
 *     I retain a new inbuf.
 *  4. Inbuf is remaining (partial), and outbuf is done (full).
 *     I retain a new outbuf.
 *  5. This would be rare, but possible. Inbuf and outbuf are both done.
 *     This is the same state as the start.
 *
 *  Structure of buffers
 *  --------------------
 *  inbuf: There are [n_subbuf]-many sub buffers. The main inbuf does not have
 *         any header. But, each sub buffer does;
 *         state (1B) type_infile (1B), curbuf (8B), and length (8B).
 *         So, the body start at 18B. The number of sub buffers is determined
 *         by the number of threads.
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

#define ALDER_KMER_ENCODER2_OUTBUFFER_HEADER     1
#define ALDER_KMER_ENCODER2_OUTSUBBUFFER_HEADER  8

#define ALDER_KMER_ENCODER2_INBUFFER_TYPE_INFILE 1
#define ALDER_KMER_ENCODER2_INBUFFER_CURRENT     2
#define ALDER_KMER_ENCODER2_INBUFFER_LENGTH      10
#define ALDER_KMER_ENCODER2_INBUFFER_BODY        18
#define ALDER_KMER_ENCODER2_FILETYPE_FASTA       1
#define ALDER_KMER_ENCODER2_FILETYPE_FASTQ       2
#define ALDER_KMER_ENCODER2_FILETYPE_SEQ         3

enum {
    ALDER_KMER_ENCODER2_EMPTY_INBUF        = 0,
    ALDER_KMER_ENCODER2_READER_USE_INBUF   = 1,
    ALDER_KMER_ENCODER2_FULL_INBUF         = 2,
    ALDER_KMER_ENCODER2_FULL_OUTBUF        = 3,
    ALDER_KMER_ENCODER2_WRITER_USE_OUTBUF  = 4,
    ALDER_KMER_ENCODER2_EMPTY_OUTBUF       = 5,
    ALDER_KMER_ENCODER2_ENCODER_USE_INBUF  = 6,
    ALDER_KMER_ENCODER2_ENCODER_USE_OUTBUF = 7,
    ALDER_KMER_ENCODER2_FINISHED_INBUF     = 8
};

typedef struct alder_kmer_encoder2_struct alder_kmer_encoder2_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_encoder2_struct {
    /* info */
    int k;                     /* k - kmer size                              */
    uint64_t n_ni;             /* n_ni - number of iteration                 */
    uint64_t n_np;             /* number of iteration                        */
    uint64_t i_ni;             /* iteration index                            */
    int n_encoder;             /* n_encoder - number of encoder threads      */
    
    /* buffer */
    size_t size_inbuf;         /* size_inbuf - size of the inbuf             */
    size_t size_inbuf2;        /* size_inbuf2 - size of the inbuf2           */
    size_t size_outbuf;        /* size_outbuf - size of the output buffer    */
    size_t n_subbuf;           /* number of divided buffers in inbuf         */
    size_t size_subinbuf;      /* size of each divided buffer                */
    size_t size_suboutbuf;     /* per thread: size of each sub out buffer    */
    size_t size_suboutbuf2;    /* per partition: size of each sub out buffer */
    uint8_t *inbuf;            /* [size_inbuf] inbuf - input buffer          */
    uint8_t *inbuf2;           /* [size_inbuf2] inbuf2 - input buffer        */
    uint8_t *outbuf;           /* [size_outbuf] outbuf - output buff         */
    /* body size */
    size_t size_suboutbuf2_body;
    
    /* file */
    struct bstrList *infile;   /* (not own) input files                      */
    void *fx;                  /* fx - input file pointer                    */
    int type_infile;           /* type of input file                         */
    FILE **fpout;              /* n_np: fpout - output file pointers         */
    bstring dout;              /* output directory                           */
    
    /* progress */
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;
    int progressToError_flag;
    
    /* stat */
    uint64_t n_byte;           /* number of bytes sent                       */
    uint64_t n_kmer;           /* number of Kmers written to an out file     */
    uint64_t n_letter;         /* number of DNA letters processed            */
    
    /* flag */
    int status;                /* status of encoder's buffers                */
    int flag;                  /* status for readwriter                      */
};

static void *encoder(void *t);
static void *reader(void *t);
static void *writer(void *t);

static void
alder_kmer_encoder2_destroy (alder_kmer_encoder2_t *o)
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

static alder_kmer_encoder2_t *
alder_kmer_encoder2_create(int n_encoder,
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
    
    alder_kmer_encoder2_t *o = malloc(sizeof(*o));
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
    o->size_inbuf = (size_t)(memory_available << 19);
    o->size_inbuf2 = ALDER_KMER_SECONDARY_BUFFER_SIZE;
    o->size_outbuf = (size_t)(memory_available << 19);
    o->n_subbuf = n_encoder * 2;
    o->size_subinbuf = o->size_inbuf / o->n_subbuf;
    o->size_suboutbuf = o->size_outbuf / o->n_subbuf;
    o->size_suboutbuf2 = (o->size_suboutbuf -  ALDER_KMER_ENCODER2_OUTBUFFER_HEADER) / n_partition;
    o->size_suboutbuf2_body = o->size_suboutbuf2 - ALDER_KMER_ENCODER2_OUTSUBBUFFER_HEADER;
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
    o->n_byte = 0;
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
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * o->size_inbuf2);
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf);
    if (o->dout == NULL ||
        o->inbuf == NULL ||
        o->inbuf2 == NULL ||
        o->outbuf == NULL) {
        alder_kmer_encoder2_destroy(o);
        return NULL;
    }
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->inbuf2, 0, sizeof(*o->inbuf2) * o->size_inbuf2);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);

    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_encoder2_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    
    o->fpout = malloc(sizeof(*o->fpout)*n_partition);
    if (o->fpout == NULL) {
        alder_kmer_encoder2_destroy(o);
        return NULL;
    }
    memset(o->fpout, 0, sizeof(*o->fpout)*n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfpar = bformat("%s/%s-%d-%d.par",
                                outdir, outfile, i_iteration, i);
        if (bfpar == NULL) {
            alder_kmer_encoder2_destroy(o);
            return NULL;
        }
        o->fpout[i] = fopen(bdata(bfpar), "wb");
        if (o->fpout[i] == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to make a file: %s",
                       bdata(bfpar));
            bdestroy(bfpar);
            alder_kmer_encoder2_destroy(o);
            return NULL;
        }
        bdestroy(bfpar);
    }
    
    /* init */
//    size_t x = o->size_inbuf / o->n_subbuf;
    /* buffer state */
    for (size_t i = 0; i < o->n_subbuf; i++) {
        uint8_t *outbuf = o->outbuf + i * o->size_suboutbuf;
        *outbuf = ALDER_KMER_ENCODER2_EMPTY_OUTBUF;
    }
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
alder_kmer_encoder2_destroy_with_error (alder_kmer_encoder2_t *o, int s)
{
    if (s != 0) {
        alder_loge(s, "cannot create fileseq_kmer_thread_readwriter");
    }
    alder_kmer_encoder2_destroy(o);
}

/**
 *  This function closes a partition file.
 *
 *  @param o        encoder
 *
 *  @return SUCCESS or FAIL
 */
static int close_infile (alder_kmer_encoder2_t *o)
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
static int open_infile (alder_kmer_encoder2_t *o, int i_infile)
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
alder_kmer_encode2_token(size_t *curbuf, uint8_t *inbuf, size_t lenbuf,
                         int type_infile)
{
    char c;
    if (*curbuf % 10000 == 0) {
        alder_log4("i_inbuf(%zu): %zu", lenbuf, *curbuf);
    }
    if (*curbuf < lenbuf) {
        c = inbuf[(*curbuf)++];
    } else {
//        assert(0);
        return 5;
    }
    if (type_infile == ALDER_KMER_ENCODER2_FILETYPE_FASTA) {
        if (c == '>' || c == '\n') {
            while (c != '\n') {
                c = inbuf[(*curbuf)++];
            }
            if (*curbuf < lenbuf) {
                c = inbuf[(*curbuf)++];
            } else {
//                e->n_byte += *curbuf;
                return 5;
            }
        }
    } else if (type_infile == ALDER_KMER_ENCODER2_FILETYPE_FASTQ) {
        if (c == '@') {
            // header
            while (c != '\n') {
                c = inbuf[(*curbuf)++];
            }
            c = inbuf[(*curbuf)++];
            assert(c != '\n');
        } else if (c == '\n') {
            // +
            c = inbuf[(*curbuf)++];
            assert(c == '+');
            while (c != '\n') {
                c = inbuf[(*curbuf)++];
            }
            // quality score
            c = inbuf[(*curbuf)++];
            while (c != '\n') {
                c = inbuf[(*curbuf)++];
            }
            if (*curbuf < lenbuf) {
                c = 'X';
            } else {
//                e->n_byte += e->i_inbuf;
                return 5;
            }
        }
    } else {
        assert(type_infile == ALDER_KMER_ENCODER2_FILETYPE_SEQ);
        if (c == '\n') {
            if (*curbuf < lenbuf) {
                // no code.
            } else {
//                e->n_byte += e->i_inbuf;
                return 5;
            }
        }
    }
    int ci = alder_dna_char2int(c);
//    if (ci < 4) {
//        e->n_letter++;
//    }
    return ci;
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
alder_kmer_encode2(int n_encoder,
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
    int s = ALDER_STATUS_SUCCESS;
    int n_thread = n_encoder + 2;
    /* Create variables for the threads. */
    alder_kmer_encoder2_t *data =
    alder_kmer_encoder2_create(n_encoder,
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
        alder_kmer_encoder2_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    /* Initialize mutex and condition variable objects */
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    s += pthread_create(&threads[0], &attr, reader, (void *)data);
    s += pthread_create(&threads[1], &attr, writer, (void *)data);
    for (int i = 0; i < n_encoder; i++) {
        s += pthread_create(&threads[i+2], &attr, encoder, (void *)data);
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
    alder_kmer_encoder2_destroy(data);
    alder_log5("Encoding Kmer has been finished with %d threads.", n_thread);
    return ALDER_STATUS_SUCCESS;
}

#pragma mark buffer

/**
 *  This function chooses a input buffer.
 *
 *  @param a          encoder
 *  @param c_inbuffer input buffer id
 */
static void
select_empty_inbuf(alder_kmer_encoder2_t *a, size_t *c_inbuffer)
{
    *c_inbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        // Check the first int in the buffer.
        uint8_t oval = *(a->inbuf + j * a->size_subinbuf);
        if (oval == ALDER_KMER_ENCODER2_EMPTY_INBUF) {
            uint8_t cval = ALDER_KMER_ENCODER2_READER_USE_INBUF;
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
 *  This function changes the state of input buffer from empty to full.
 *
 *  @param a        encoder
 *  @param c_buffer buffer id
 *
 *  @return void
 */
static void
deselect_full_inbuf(alder_kmer_encoder2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_ENCODER2_READER_USE_INBUF);
    uint8_t cval = ALDER_KMER_ENCODER2_FULL_INBUF;
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
 *  @param a          encoder
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
deselect_empty_inbuf(alder_kmer_encoder2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_ENCODER2_READER_USE_INBUF);
    uint8_t cval = ALDER_KMER_ENCODER2_EMPTY_INBUF;
    cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function changes the state of input buffer; READER_USE to FINISHED.
 *
 *  @param a          encoder
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
finish_empty_inbuf(alder_kmer_encoder2_t *a)
{
    int isFinishedInbuf = ALDER_NO;
    while (isFinishedInbuf == ALDER_NO) {
        isFinishedInbuf = ALDER_YES;
        sleep(1);
        for (size_t c_inbuffer = 0; c_inbuffer < a->n_subbuf; c_inbuffer++) {
            uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
            
            if (oval == ALDER_KMER_ENCODER2_EMPTY_INBUF) {
                uint8_t cval = ALDER_KMER_ENCODER2_FINISHED_INBUF;
                cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                                   (uint8_t)oval,
                                                   (uint8_t)cval);
                assert(cval == oval);
//                if (cval == oval) {
//                    // No code: this inbuf is now in the finished state.
//                } else {
//                    // Is this possible?
//                    //
//                    isFinishedInbuf = ALDER_NO;
//                }
            } else if (oval == ALDER_KMER_ENCODER2_FINISHED_INBUF) {
                // No code: this inbuf was already in the finished state.
            } else {
                // The inbuf is still used by others.
                isFinishedInbuf = ALDER_NO;
            }
        }
    }
}

/**
 *  This function selects a full output buffer.
 *
 *  @param a        encoder
 *  @param c_buffer return value of buffer id
 *
 *  @return void
 */
static void
select_full_outbuf(alder_kmer_encoder2_t *a, size_t *c_outbuffer)
{
    *c_outbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        // Check the first int in the buffer.
        uint8_t oval = *(a->outbuf + j * a->size_suboutbuf);
        if (oval == ALDER_KMER_ENCODER2_FULL_OUTBUF) {
            uint8_t cval = ALDER_KMER_ENCODER2_WRITER_USE_OUTBUF;
            cval = __sync_val_compare_and_swap(a->outbuf + j * a->size_suboutbuf,
                                               (uint8_t)oval,
                                               (uint8_t)cval);
            if (cval == oval) {
                *c_outbuffer = j;
                break;
            }
        }
    }
}



/**
 *  This function leaves the output buffer empty.
 *
 *  @param a        encoder
 *  @param c_buffer buffer id
 *
 *  @return void
 */
static void
deselect_empty_outbuf(alder_kmer_encoder2_t *a, size_t c_outbuffer)
{
    uint8_t oval = *(a->outbuf + c_outbuffer * a->size_suboutbuf);
    assert(oval == ALDER_KMER_ENCODER2_WRITER_USE_OUTBUF);
    uint8_t cval = ALDER_KMER_ENCODER2_EMPTY_OUTBUF;
    cval = __sync_val_compare_and_swap(a->outbuf + c_outbuffer * a->size_suboutbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function changes the buffer state from FULL to USE by ENCODER.
 *
 *  @param a          encoder
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
retain_inbuf(alder_kmer_encoder2_t *a, size_t *c_inbuffer)
{
    *c_inbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        uint8_t oval = *(a->inbuf + j * a->size_subinbuf);
        if (oval == ALDER_KMER_ENCODER2_FULL_INBUF) {
            uint8_t cval = ALDER_KMER_ENCODER2_ENCODER_USE_INBUF;
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
 *  This funciton is called encoder thread, and releases an input buffer.
 *
 *  @param a          encoder
 *  @param c_inbuffer input buffer id
 *
 *  @return void
 */
static void
release_inbuf(alder_kmer_encoder2_t *a, size_t c_inbuffer)
{
    uint8_t oval = *(a->inbuf + c_inbuffer * a->size_subinbuf);
    assert(oval == ALDER_KMER_ENCODER2_ENCODER_USE_INBUF);
    uint8_t cval = ALDER_KMER_ENCODER2_EMPTY_INBUF;
    cval = __sync_val_compare_and_swap(a->inbuf + c_inbuffer * a->size_subinbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function changes the outbuffer state from EMPTY to USE for encoder.
 *
 *  @param a           encoder
 *  @param c_outbuffer output buffer id
 *
 *  @return void
 */
static void
retain_outbuf(alder_kmer_encoder2_t *a, size_t *c_outbuffer)
{
    *c_outbuffer = a->n_subbuf;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        uint8_t oval = *(a->outbuf + j * a->size_suboutbuf);
        if (oval == ALDER_KMER_ENCODER2_EMPTY_OUTBUF) {
            uint8_t cval = ALDER_KMER_ENCODER2_ENCODER_USE_OUTBUF;
            cval = __sync_val_compare_and_swap(a->outbuf + j * a->size_suboutbuf,
                                               (uint8_t)oval,
                                               (uint8_t)cval);
            if (cval == oval) {
                *c_outbuffer = j;
                break;
            }
        }
    }
}

/**
 *  This function releases the out buffer.
 *
 *  @param a           encoder
 *  @param c_outbuffer out buffer id
 *
 *  @return void
 */
static void
release_outbuf(alder_kmer_encoder2_t *a, size_t c_outbuffer)
{
    uint8_t oval = *(a->outbuf + c_outbuffer * a->size_suboutbuf);
    assert(oval == ALDER_KMER_ENCODER2_ENCODER_USE_OUTBUF);
    uint8_t cval = ALDER_KMER_ENCODER2_FULL_OUTBUF;
    cval = __sync_val_compare_and_swap(a->outbuf + c_outbuffer * a->size_suboutbuf,
                                       (uint8_t)oval,
                                       (uint8_t)cval);
    assert(cval == oval);
}

/**
 *  This function tests whether all of input buffers were finished.
 *
 *  @param a encoder
 *
 *  @return YES or NO
 */
static int
check_inbuf_finished(alder_kmer_encoder2_t *a)
{
    int s = ALDER_YES;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        uint8_t oval = *(a->inbuf + j * a->size_subinbuf);
        if (oval != ALDER_KMER_ENCODER2_FINISHED_INBUF) {
            s = ALDER_NO;
        }
    }
    return s;
}

static int
check_outbuf_empty(alder_kmer_encoder2_t *a)
{
    int s = ALDER_YES;
    for (size_t j = 0; j < a->n_subbuf; j++) {
        uint8_t oval = *(a->outbuf + j * a->size_subinbuf);
        if (oval != ALDER_KMER_ENCODER2_EMPTY_OUTBUF) {
            s = ALDER_NO;
        }
    }
    return s;
}

/**
 *  This function returns 0,1,2 depending on the type of an input file.
 *
 *  @param a encoder
 *
 *  @return 0 for FASTA, 1 for FASTQ, 2 otherwise.
 */
static uint8_t what_type_infile(alder_kmer_encoder2_t *a)
{
    uint8_t type_infile;
    if (a->type_infile & ALDER_FILETYPE_ISFASTA) {
        type_infile = ALDER_KMER_ENCODER2_FILETYPE_FASTA;
    } else if (a->type_infile & ALDER_FILETYPE_ISFASTQ) {
        type_infile = ALDER_KMER_ENCODER2_FILETYPE_FASTQ;
    } else {
        type_infile = ALDER_KMER_ENCODER2_FILETYPE_SEQ;
    }
    return type_infile;
}

#pragma mark thread

/**
 *  This function reads files.
 *
 *  @param t data
 *
 *  @return SUCCESS or FAIL
 *
 *  State of buffer:
 *  0 - reader needs to supply the input buffer
 *  1 - reader is supplying input
 *  2 - reader is finished with the supply, and encoder can use the buffer
 *  3 - encoder is using the input buffer
 *  4 - encoder is finished with input buffer, and reader can use the buffer
 *
 */
static void *reader(void *t)
{
    alder_log("reader(): START");
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_encoder2_t *a = (alder_kmer_encoder2_t*)t;
    assert(a != NULL);
    
    /* Read each file until no file is left. */
    /* Exit if no file is left to read.      */
    for (int i = 0; i < a->infile->qty; i++) {
        /* Open, read, and exit if nothing left to read. */
        s = open_infile(a, i);
        assert(s == ALDER_STATUS_SUCCESS);
        /* The second element in a buffer is a sequence file type. */
        uint8_t type_infile = what_type_infile(a);
        /* The secondary buffer is reset. */
        size_t lenbuf2 = 0;
        /* The initial state for entering the while-loop */
        size_t lenbuf = 1;
        while (lenbuf > 0) {
            /* A reader needs an empty input buffer. */
            size_t c_inbuffer = a->n_subbuf;
            while (c_inbuffer == a->n_subbuf) {
                select_empty_inbuf(a, &c_inbuffer);
//                sleep(1);
            }
            
            uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
            inbuf[ALDER_KMER_ENCODER2_INBUFFER_TYPE_INFILE] = type_infile;
            uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER2_INBUFFER_BODY;
            size_t size_body = a->size_subinbuf - ALDER_KMER_ENCODER2_INBUFFER_BODY;
            s = alder_fileseq_chunk(&lenbuf,
                                    (char*)inbuf_body,
                                    size_body,
                                    &lenbuf2,
                                    (char*)a->inbuf2,
                                    a->size_inbuf2,
                                    a->k, NULL, a->fx, a->type_infile);
            assert(s == ALDER_STATUS_SUCCESS);
            to_size(inbuf, ALDER_KMER_ENCODER2_INBUFFER_CURRENT) = (size_t)0;
            to_size(inbuf, ALDER_KMER_ENCODER2_INBUFFER_LENGTH) = lenbuf;
            
            /* Progress */
            a->n_byte += lenbuf;
            if (a->progress_flag) {
                alder_progress_step(a->n_byte, a->totalFilesize,
                                    a->progressToError_flag);
            }
            
            if (lenbuf > 0) {
                deselect_full_inbuf(a, c_inbuffer);
            } else {
                deselect_empty_inbuf(a, c_inbuffer);
            }
        }
        assert(lenbuf2 == 0);
        close_infile(a);
    }
    finish_empty_inbuf(a);
    
    pthread_exit(NULL);
}

/**
 *  This function runs on the writer thread.
 *
 *  @param t data
 *
 *  @return void
 */
static void *writer(void *t)
{
    alder_log("writer(): START");
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_encoder2_t *a = (alder_kmer_encoder2_t*)t;
    assert(a != NULL);
    
    /* Read each file until no file is left. */
    /* Exit if no file is left to read.      */
    while (1) {
        //
        size_t c_outbuffer = a->n_subbuf;
        s = ALDER_NO;
        while (c_outbuffer == a->n_subbuf && s == ALDER_NO) {
            select_full_outbuf(a, &c_outbuffer);
            sleep(1);
            s = check_inbuf_finished(a);
        }
        
        if (c_outbuffer < a->n_subbuf) {
            uint8_t *outbuf = (a->outbuf + c_outbuffer * a->size_suboutbuf +
                               ALDER_KMER_ENCODER2_OUTBUFFER_HEADER);
            for (int i_np = 0; i_np < a->n_np; i_np++) {
                // Write the content.
                uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
                uint8_t *outbuf2_body = outbuf2 + ALDER_KMER_ENCODER2_OUTSUBBUFFER_HEADER;
                size_t lenbuf = to_size(outbuf2, 0);
                size_t s = fwrite(outbuf2_body, sizeof(*outbuf2_body), lenbuf,
                                  a->fpout[i_np]);
                if (s != lenbuf) {
                    // Error in writing.
                    assert(0);
                }
                to_size(outbuf2,0) = 0;
            }
            deselect_empty_outbuf(a, c_outbuffer);
        } else {
            // No full outbuffer.
            assert(s == ALDER_YES);
        }
        
        if (s == ALDER_YES) {
            // Check all of the out buffer.
            s = check_outbuf_empty(a);
            if (s == ALDER_YES) {
                // It is time to exit.
                break;
            }
        }
    }
    // WARN: I should combine out buffers for the same partition.
    // encoder_combine_before_flush(a);
    // Make sure that all of the input buffer is finished,
    // and all of the output buffer is empty.
//    encoder_flush(a);
    
    
    pthread_exit(NULL);
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
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_encoder2_t *a = (alder_kmer_encoder2_t*)t;
    assert(a != NULL);
    
    alder_encode_number2_t * s1 = NULL;
    alder_encode_number2_t * s2 = NULL;
    int K = a->k;
    alder_log("encoder(): START");
    s1 = alder_encode_number2_createWithKmer(K);
    s2 = alder_encode_number2_createWithKmer(K);
    size_t *len_outbuf2 = malloc(sizeof(*len_outbuf2) * a->n_np);
    
    size_t ib = s1->b / 8;
    size_t jb = s1->b % 8;
    uint64_t ni = a->n_ni;
    uint64_t np = a->n_np;
    size_t c_inbuffer = a->n_subbuf;
    size_t c_outbuffer = a->n_subbuf;
    while (1) {
        
        s = ALDER_NO;
        while (c_inbuffer == a->n_subbuf && s == ALDER_NO) {
            retain_inbuf(a, &c_inbuffer);
//            sleep(1);
            s = check_inbuf_finished(a);
        }
        if (s == ALDER_YES) {
            // No input available.
            assert(c_inbuffer == a->n_subbuf);
        }
        
        if (c_inbuffer < a->n_subbuf) {
            while (c_outbuffer == a->n_subbuf) {
                retain_outbuf(a, &c_outbuffer);
//                sleep(1);
            }
        }
        
        
        if (c_inbuffer < a->n_subbuf && c_outbuffer < a->n_subbuf) {
            /* setup of inbuf */
            uint8_t *inbuf = a->inbuf + c_inbuffer * a->size_subinbuf;
            assert(*inbuf == ALDER_KMER_ENCODER2_ENCODER_USE_INBUF);
            int type_infile = (int)inbuf[ALDER_KMER_ENCODER2_INBUFFER_TYPE_INFILE];
            assert(type_infile == ALDER_KMER_ENCODER2_FILETYPE_FASTA ||
                   type_infile == ALDER_KMER_ENCODER2_FILETYPE_FASTQ ||
                   type_infile == ALDER_KMER_ENCODER2_FILETYPE_SEQ);
            
            size_t curbuf = to_size(inbuf,ALDER_KMER_ENCODER2_INBUFFER_CURRENT);
            size_t lenbuf = to_size(inbuf,ALDER_KMER_ENCODER2_INBUFFER_LENGTH);
            uint8_t *inbuf_body = inbuf + ALDER_KMER_ENCODER2_INBUFFER_BODY;
            /* setup of outbuf */
            uint8_t *outbuf = a->outbuf + c_outbuffer * a->size_suboutbuf;
            assert(*outbuf == ALDER_KMER_ENCODER2_ENCODER_USE_OUTBUF);
            outbuf += ALDER_KMER_ENCODER2_OUTBUFFER_HEADER;
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
                        token = alder_kmer_encode2_token(&curbuf, inbuf_body,
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
                token = alder_kmer_encode2_token(&curbuf, inbuf_body,
                                                 lenbuf, type_infile);
                if (token >= 4) continue;
                alder_log5("token: %d", token);
                int b1 = token;
                int b2 = (b1 + 2) % 4;
                alder_encode_number2_shiftLeftWith(s1,b1);
                alder_encode_number2_shiftRightWith(s2,b2);
                
//#define ALDERDEBUGENCODE
                
                /* 5. Select partition, iteration, and kmer of the two. */
                alder_encode_number2_t *ss = NULL;
                uint64_t i_ni = 0;
                uint64_t i_np = 0;
                alder_hashtable_mcas_select2(&ss, &i_ni, &i_np, s1, s2,
                                             ni, np);
                
#if defined(ALDERDEBUGENCODE)
                alder_log4("Kmer");
                alder_encode_number2_log(s1);
                alder_encode_number2_log(s2);
                alder_encode_number2_log(ss);
#endif
                /* 6. Save a chosen one in the output buffers. */
                if (i_ni == a->i_ni) {
                    uint8_t *outbuf2 = outbuf + i_np * a->size_suboutbuf2;
                    uint8_t *outbuf2_body = outbuf2 + ALDER_KMER_ENCODER2_OUTSUBBUFFER_HEADER;
                    
                    /* Encode the Kmer ss. */
                    size_t x = len_outbuf2[i_np];
                    for (size_t i = 0; i < ib; i++) {
                        for (size_t j = 0; j < 8; j++) {
                            outbuf2_body[x++] = ss->n[i].key8[j];
                        }
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
                    assert(x == ALDER_KMER_ENCODER2_FULL_OUTBUF ||
                           x == ALDER_KMER_ENCODER2_WRITER_USE_OUTBUF ||
                           x == ALDER_KMER_ENCODER2_EMPTY_OUTBUF ||
                           x == ALDER_KMER_ENCODER2_ENCODER_USE_OUTBUF);
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
                release_inbuf(a, c_inbuffer);
                c_inbuffer = a->n_subbuf;
                alder_log2("encoder(): inbuf is empty.");
            } else {
                // partial inbuf
                // Save the current buffer position for later use.
                to_size(inbuf,ALDER_KMER_ENCODER2_INBUFFER_CURRENT) = curbuf;
                alder_log2("encoder(): inbuf is not yet empty, more inbuf.");
            }
            if (isFullOutbuf == true) {
                // full outbuf
                release_outbuf(a, c_outbuffer);
                c_outbuffer = a->n_subbuf;
                alder_log2("encoder(): outbuf is full.");
            } else {
                // partial outbuf
                alder_log2("encoder(): outbuf is not yet full, more space.");
            }
        } else if (c_inbuffer == a->n_subbuf && c_outbuffer < a->n_subbuf) {
            // Release the output buffer because I do not have any input.
            release_outbuf(a, c_outbuffer);
            c_outbuffer = a->n_subbuf;
            alder_log2("encoder(): force outbuf full because no input.");
        } else {
            /* No input! Exit. */
            alder_log2("encoder(): exiting...");
            break;
        }
    }
    
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    XFREE(len_outbuf2);
    alder_log("encoder(): END");
    pthread_exit(NULL);
}


