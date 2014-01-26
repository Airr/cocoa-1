/**
 * This file, alder_kmer_binary.c, is part of alder-kmer.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
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

#include "alder_kmer_binary.h"

#define ALDER_KMER_SECONDARY_BUFFER_SIZE       1000
#define ALDER_KMER_BINARY_OUTBUFFER_HEADER     1
#define ALDER_KMER_BINARY_OUTSUBBUFFER_HEADER  8

#define ALDER_KMER_BINARY_INBUFFER_TYPE_INFILE 1
#define ALDER_KMER_BINARY_INBUFFER_CURRENT     2
#define ALDER_KMER_BINARY_INBUFFER_LENGTH      10
#define ALDER_KMER_BINARY_INBUFFER_BODY        18

#define ALDER_KMER_BINARY_OUTBUFFER_CURRENT    0
#define ALDER_KMER_BINARY_OUTBUFFER_LENGTH     8
#define ALDER_KMER_BINARY_OUTBUFFER_BODY       16

//#define ALDER_KMER_BINARY_READBLOCK_LEN        4
//#define ALDER_KMER_BINARY_READBLOCK_BODY       12

#define ALDER_KMER_BINARY_FILETYPE_FASTA       1
#define ALDER_KMER_BINARY_FILETYPE_FASTQ       2
#define ALDER_KMER_BINARY_FILETYPE_SEQ         3

#define ALDER_KMER_BINARY_INBUFFER_STATE_0     0
#define ALDER_KMER_BINARY_INBUFFER_STATE_1     1
#define ALDER_KMER_BINARY_INBUFFER_STATE_2     2
#define ALDER_KMER_BINARY_INBUFFER_STATE_3     3
#define ALDER_KMER_BINARY_INBUFFER_STATE_4     4 // AAANNAA or AAANNNN


typedef int (*binary_t)(
void *ptr, size_t size, size_t subsize,
uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
long version,
int K, long D,
int n_nt,
size_t totalfilesize,
size_t *n_byte,
int progress_flag,
int progressToError_flag,
struct bstrList *infile, const char *outdir,
const char *outfile);


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

/**
 *  dna2byte[DNA][POS].
 */
static uint8_t dna2byte[4][4] = {
    {0x00,0x00,0x00,0x00},
    {0x40,0x10,0x04,0x01},
    {0x80,0x20,0x08,0x02},
    {0xC0,0x30,0x0C,0x03}
};

//
///* This function converts four numbers to a unsiged int8 or char or byte.
// * e.g., 1 2 3 0 -> 01 10 11 00 = 0x6C
// */
//uint8_t alder_bit_four2uc(int i1, int i2, int i3, int i4)
//{
//    assert(0 <= i1 && i1 < 4);
//    assert(0 <= i2 && i2 < 4);
//    assert(0 <= i3 && i3 < 4);
//    assert(0 <= i4 && i4 < 4);
//    uint8_t x = 0;
//    x |= alder_bit_four2uc_matrix[i1][0];
//    x |= alder_bit_four2uc_matrix[i2][1];
//    x |= alder_bit_four2uc_matrix[i3][2];
//    x |= alder_bit_four2uc_matrix[i4][3];
//    return x;
//}

static int binary_id_counter = 0;

static void *binary(void *t);

static void
alder_kmer_binary_destroy (alder_kmer_binary_t *o)
{
    if (o != NULL) {
        XFCLOSE(o->fpout);
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        bdestroy(o->dout);
        XFREE(o);
    }
}

static alder_kmer_binary_t *
alder_kmer_binary_create(int K,
                         long disk_space,
                         int n_nt,
                         size_t totalfilesize,
                         size_t n_byte,
                         int progress_flag,
                         int progressToError_flag,
                         struct bstrList *infile,
                         const char *outdir,
                         const char *outfile)
{
    alder_log5("Creating fileseq_kmer_thread_readwriter...");
    int s;
    
    alder_kmer_binary_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create fileseq_kmer_thread_readwriter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    /* init */
    o->k = K;
    o->n_binary = 1;
    o->len_read = 0;
    o->i_infile = 0;
//    o->size_fixed_inbuf = (1 << 16);  // FIXME: must be determined.
//    o->size_fixed_outbuf = (1 << 20); // FIXME: must be determined.
    o->size_fixed_inbuf = 21;  // FIXME: must be determined.
    o->size_fixed_outbuf = 21 << 2; // FIXME: must be determined.
    o->size_inbuf = ALDER_KMER_BINARY_INBUFFER_BODY + o->size_fixed_inbuf;
    o->size_outbuf = (ALDER_KMER_BINARY_OUTBUFFER_BODY + o->size_fixed_outbuf +
                      o->size_fixed_inbuf); // for safe spare buffer
    o->inbuf = NULL;
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
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("%*s %d (MB)", width, "inbuf size:", o->size_inbuf >> 20);
    alder_log("%*s %d (MB)", width, "outbuf size:", o->size_outbuf >> 20);
    
    /* Memory */
    o->dout = bfromcstr(outdir);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);    /* bigmem */
    o->outbuf = malloc(sizeof(*o->outbuf) * o->size_outbuf); /* bigmem */
    if (o->dout == NULL ||
        o->inbuf == NULL ||
        o->outbuf == NULL) {
        alder_kmer_binary_destroy(o);
        return NULL;
    }
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->outbuf, 0, sizeof(*o->outbuf) * o->size_outbuf);
    to_size(o->outbuf,ALDER_KMER_BINARY_OUTBUFFER_CURRENT) = 0;
    to_size(o->outbuf,ALDER_KMER_BINARY_OUTBUFFER_LENGTH) = o->size_fixed_outbuf;
    
    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_binary_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    
    bstring bfpar = bformat("%s/%s.bin", outdir, outfile);
    if (bfpar == NULL) {
        alder_kmer_binary_destroy(o);
        return NULL;
    }
    o->fpout = fopen(bdata(bfpar), "wb");
    if (o->fpout == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to make a file: %s",
                   bdata(bfpar));
        bdestroy(bfpar);
        alder_kmer_binary_destroy(o);
        return NULL;
    }
    bdestroy(bfpar);
    
    alder_log5("Finish - Creating fileseq_kmer_thread_readwriter...");
    return o;
}

/**
 *  This function closes a partition file.
 *
 *  @param o        binary
 *
 *  @return SUCCESS or FAIL
 */
static int close_infile (alder_kmer_binary_t *o)
{
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    
    if (o->fx != NULL) {
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            gzclose(o->fx);
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            BZ2_bzclose(o->fx);
        } else if (o->type_infile & ALDER_FILETYPE_LZ) {
            assert(0);
//            LZ2_bzclose(o->fx);
        } else {
            close((int)((intptr_t)o->fx));
        }
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function opens a partition file.
 *
 *  @param o        binary
 *  @param i_infile input file index
 *
 *  @return SUCCESS or FAIL
 */
static int open_infile (alder_kmer_binary_t *o, int i_infile)
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

///**
// *  This function returns 0,1,2,3 for DNA.
// *
// *  @param e binary
// *
// *  @return int
// *  0,1,2,3 for A,C,T,G.
// *  4 for others.
// *  5 for the end of buffer.
// */
//static int
//alder_kmer_binary_token(size_t *curbuf, uint8_t *inbuf, size_t lenbuf,
//                        int type_infile)
//{
//    char c;
//    size_t vcurbuf = *curbuf;
//#if !defined(NDEBUG)
//    if (vcurbuf % 10000 == 0) {
//        alder_log4("i_inbuf(%zu): %zu", lenbuf, vcurbuf);
//    }
//#endif
//    if (vcurbuf < lenbuf) {
//        c = inbuf[vcurbuf++];
//    } else {
//        //        assert(0);
//        *curbuf = vcurbuf;
//        return 5;
//    }
//    if (type_infile == ALDER_KMER_BINARY_FILETYPE_FASTA) {
//        if (c == '>' || c == '\n') {
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            if (c == '>') {
//                uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
//                vcurbuf = (size_t)(cinbuf - inbuf + 1);
//            }
//            //            while (c != '\n') {
//            //                c = inbuf[vcurbuf++];
//            //            }
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            
//            if (vcurbuf < lenbuf) {
//                c = inbuf[vcurbuf++];
//            } else {
//                //                e->n_byte += *curbuf;
//                *curbuf = vcurbuf;
//                return 5;
//            }
//        }
//    } else if (type_infile == ALDER_KMER_BINARY_FILETYPE_FASTQ) {
//        if (c == '@') {
//            // header
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
//            vcurbuf = (size_t)(cinbuf - inbuf + 1);
//            //            while (c != '\n') {
//            //                c = inbuf[vcurbuf++];
//            //            }
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            
//            c = inbuf[vcurbuf++];
//            assert(c != '\n');
//        } else if (c == '\n') {
//            // +
//            c = inbuf[vcurbuf++];
//            assert(c == '+');
//            
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
//            vcurbuf = (size_t)(cinbuf - inbuf + 1);
//            cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
//            vcurbuf = (size_t)(cinbuf - inbuf + 1);
//            //            while (c != '\n') {
//            //                c = inbuf[vcurbuf++];
//            //            }
//            // quality score
//            //            c = inbuf[vcurbuf++];
//            //            while (c != '\n') {
//            //                c = inbuf[vcurbuf++];
//            //            }
//            /*****************************************************************/
//            /*                          OPTIMIZATION                         */
//            /*****************************************************************/
//            
//            if (vcurbuf < lenbuf) {
//                c = 'X';
//            } else {
//                //                e->n_byte += e->i_inbuf;
//                *curbuf = vcurbuf;
//                return 5;
//            }
//        }
//    } else {
//        assert(type_infile == ALDER_KMER_BINARY_FILETYPE_SEQ);
//        if (c == '\n') {
//            if (vcurbuf < lenbuf) {
//                // no code.
//            } else {
//                //                e->n_byte += e->i_inbuf;
//                *curbuf = vcurbuf;
//                return 5;
//            }
//        }
//    }
//    
//    *curbuf = vcurbuf;
//    return dna_char2int[c];
//    //    int ci = alder_dna_char2int(c);
//    //    return ci;
//}

#pragma mark main

/**
 *  This function converts a set of sequence data to a binary file before
 *  starting the procedure of counting Kmers.
 *
 *  @param version             version
 *  @param K                   kmer size
 *  @param D                   disk space
 *  @param n_nt                number of threads available
 *  @param progress_flag       progress
 *  @param progressToErro_flag progress to stderr
 *  @param infile              input files
 *  @param outdir              out directory
 *  @param outfile             output file name prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_binary(void *ptr, size_t size, size_t subsize,
                  uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                  long version,
                  int K, long D,
                  int n_nt,
                  size_t totalfilesize,
                  size_t *n_byte,
                  int progress_flag,
                  int progressToError_flag,
                  struct bstrList *infile, const char *outdir,
                  const char *outfile)
{
    binary_t binary;
    
    if (version == 5) {
        binary = &alder_kmer_binary2;
    } else {
        binary = &alder_kmer_binary1;
    }
    int s = (*binary)(ptr, size, subsize,
                      n_kmer, n_dna, n_seq,
                      version,
                      K, D,
                      n_nt,
                      totalfilesize,
                      n_byte,
                      progress_flag,
                      progressToError_flag,
                      infile, outdir, outfile);
    return s;
}

int
alder_kmer_binary1(void *ptr, size_t size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   long version,
                   int K, long D,
                   int n_nt,
                   size_t totalfilesize,
                   size_t *n_byte,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile)
{
    alder_log5("preparing for making a binary file...");
    n_nt = 1;
    int s = ALDER_STATUS_SUCCESS;
    /* Create variables for the threads. */
    alder_kmer_binary_t *data =
    alder_kmer_binary_create(K,
                             D,
                             n_nt,
                             totalfilesize,
                             *n_byte,
                             progress_flag,
                             progressToError_flag,
                             infile,
                             outdir,
                             outfile);
    alder_log5("creating %d threads: one for reader, and one for writer",
               n_nt);
    pthread_t *threads = malloc(sizeof(*threads)*n_nt);
    memset(threads, 0, sizeof(*threads)*n_nt);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY,
                   "failed to create fileseq_kmer_thread_readwriter");
        XFREE(threads);
        alder_kmer_binary_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    /* Initialize mutex and condition variable objects */
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_nt; i++) {
        s += pthread_create(&threads[i], &attr, binary, (void *)data);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "cannot create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    /* Wait for all threads to complete */
    alder_log5("Waiting for all threads to complete...");
    for (int i = 0; i < n_nt; i++) {
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
    alder_kmer_binary_destroy(data);
    alder_log5("Encoding Kmer has been finished with %d threads.", n_nt);
    return ALDER_STATUS_SUCCESS;
}

#pragma mark buffer

static int
assign_binary_id()
{
    int oval = binary_id_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = binary_id_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&binary_id_counter, (int)oval, (int)cval);
    }
    return oval;
}


/**
 *  This function returns 0,1,2 depending on the type of an input file.
 *
 *  @param a binary
 *
 *  @return 0 for FASTA, 1 for FASTQ, 2 otherwise.
 */
static uint8_t what_type_infile(alder_kmer_binary_t *a)
{
    uint8_t type_infile;
    if (a->type_infile & ALDER_FILETYPE_ISFASTA) {
        type_infile = ALDER_KMER_BINARY_FILETYPE_FASTA;
    } else if (a->type_infile & ALDER_FILETYPE_ISFASTQ) {
        type_infile = ALDER_KMER_BINARY_FILETYPE_FASTQ;
    } else {
        type_infile = ALDER_KMER_BINARY_FILETYPE_SEQ;
    }
    return type_infile;
}

#pragma mark thread

/**
 *  This funciton writes the output buffer to partition files.
 *
 *  @param a          binary
 *  @param binary_id binary id
 */
static size_t write_to_binary_file(alder_kmer_binary_t *a,
                                   int binary_id)
{
    uint8_t *outbuf = a->outbuf + ALDER_KMER_BINARY_OUTBUFFER_BODY;
    size_t cur_outbuf = to_size(a->outbuf,ALDER_KMER_BINARY_OUTBUFFER_CURRENT);
    if (cur_outbuf > a->size_fixed_outbuf) {
        cur_outbuf = a->size_fixed_outbuf;
    } else {
        size_t n_rblock = ALDER_BYTESIZE_KMER(cur_outbuf,a->size_fixed_inbuf);
        cur_outbuf = n_rblock * a->size_fixed_inbuf;
    }
    write(fileno(a->fpout), outbuf, cur_outbuf);
    to_size(a->outbuf,ALDER_KMER_BINARY_OUTBUFFER_CURRENT) = 0;
    return binary_id;
}

static int read_from_sequence_file(alder_kmer_binary_t *a, int binary_id)
{
    int s;
    assert(binary_id == 0);
    
    if (a->len_read == 0) {
        if (a->i_infile > 0) {
            close_infile(a);
        }
        if (a->i_infile < a->infile->qty) {
            s = open_infile(a, a->i_infile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->inbuf_type_infile = what_type_infile(a);
            a->i_infile++;
        } else {
            return a->n_binary;
        }
    }
    assert(a->i_infile <= a->infile->qty);
    
    if (a->type_infile & ALDER_FILETYPE_NOZ) {
        uint8_t *inbuf_body = a->inbuf + ALDER_KMER_BINARY_INBUFFER_BODY;
        a->len_read = (int) read((int)((intptr_t)a->fx), inbuf_body,
                                 a->size_fixed_inbuf);
        if (a->len_read == -1) {
            alder_loge(ALDER_ERR_FILE, "failed to read a file: %s",
                       strerror(errno));
        }
        a->inbuf[ALDER_KMER_BINARY_INBUFFER_TYPE_INFILE] = a->inbuf_type_infile;
        to_size(a->inbuf, ALDER_KMER_BINARY_INBUFFER_CURRENT) = 0;
        to_size(a->inbuf, ALDER_KMER_BINARY_INBUFFER_LENGTH) = (size_t)a->len_read;
    } else if (a->type_infile & ALDER_FILETYPE_GZ) {
        a->len_read = (ssize_t) gzread((gzFile)a->fx, a->inbuf,
                                       (unsigned)(a->size_inbuf));
    } else if (a->type_infile & ALDER_FILETYPE_BZ) {
        a->len_read = (ssize_t) gzread((BZFILE*)a->fx, a->inbuf,
                                       (unsigned)(a->size_inbuf));
    } else if (a->type_infile & ALDER_FILETYPE_LZ) {
        assert(0);
    }
    
    /* Progress */
    a->n_byte += a->len_read;
    if (a->progress_flag) {
        alder_progress_step(a->n_byte, a->totalFilesize,
                            a->progressToError_flag);
    }
    
    return binary_id;
}

/**
 *  This function runs on the binary thread.
 *
 *  @param t data
 *
 *  @return SUCCESS or FAIL
 *
 *  Outbuffer block contains sequence blocks. The first position in the block
 *  has a 8-byte length of the bytes in the outbuffer block. This is for
 *  reading a fixed-size buffer later in counting.
 *  
 *  In FASTA format, sequences can be too large to fit to an outbuffer block.
 *  I need to generate a header for a long sequence where it may need multiple
 *  blocks. In FASTQ format files, I'd move the last sequence that does not 
 *  fit to an outbuffer block to a next block. This is possible because a block
 *  can be at least 16KB, and a sequence in FASTQ would tens of bytes.
 *
 *  I assume that write-blocks are larger than read-blocks.
 *  Each read block can contain a partial or one or more sequence blocks.
 *  Because write-blocks are larger, I could write multiple read-blocks.
 *  A read-block has the following format:
 *
 *  [4-byte type: FSTA or FSTQ] [8-byte number of bytes] [sequence blocks]
 *
 *  The 8-byte number can be found if there are one or more sequence blocks.
 *  This is not possible if a sequence in a block is partial; or if a sequence
 *  is too long to fit to a block. In FASTQ, a sequence block is:
 *
 *  [2-byte length of DNA] [encoded bytes]
 *  , which I call a sequence block.
 *
 *  In FASTA,
 *  [8-byte length of DNA] [encoded bytes]
 *
 *  So, 8-byte number in read-block can be smaller than 8-byte length in a
 *  sequence block. So, if FASTA sequence is too long, I'd use 1 more bytes
 *  to indicate where the start bit in the [encoded bytes]. The 8-byte length
 *  of DNA would be adjusted when reading FASTA sequences. This value can 
 *  be different depending on the size of kmer. I'd not know how these values
 *  of FASTA sequence blocks until I start coutning.
 *
 *  In LONG FASTA,
 *  [8-byte length of DNA] [1-byte] [encoded bytes]
 *
 */
static void *binary(void *t)
{
    int binary_id = assign_binary_id();
    alder_log("binary(%d): START", binary_id);
    alder_kmer_binary_t *a = (alder_kmer_binary_t*)t;
    assert(a != NULL);
    
    /* Setup of outbuf */
    char c;                       // a read character
    int c_outbuffer = binary_id;  // no write at the start
    int c_inbuffer = a->n_binary; // read at the start
    // This is not a hard part; one-byte encoding.
    int e_4counter = 0;          // encoded position in a byte (0, 1, 2, or 3).
    uint8_t e_byte = 0;          // encoded byte
    uint16_t e_len = 0;          // encoded sequence length (not byte)
    //
    size_t cur_inbuf = 0;
    size_t len_inbuf = 0;
    size_t cur_outbuf = 0;
    size_t len_outbuf = 0;
    uint8_t *inbuf_main = a->inbuf;
    uint8_t *outbuf_main = a->outbuf;
    uint8_t *inbuf = inbuf_main + ALDER_KMER_BINARY_INBUFFER_BODY;
    uint8_t *outbuf = outbuf_main + ALDER_KMER_BINARY_OUTBUFFER_BODY;
    memcpy(outbuf, "FSTQ", 4);
    size_t opos_rblock_end = a->size_fixed_inbuf; // end position of a subblock
    size_t opos_rblock_byte = 4; // 4-byte after FSTQ.
    int s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_0; // first-line of FASTQ
    size_t opos_e_len = 0;       // pos in the outbuffer for e_len
    
    
    while (1) {
        
        ///////////////////////////////////////////////////////////////////////
        // Writer
        ///////////////////////////////////////////////////////////////////////
        if (c_outbuffer == a->n_binary) {
            c_outbuffer = (int) write_to_binary_file(a, binary_id);
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Reader
        ///////////////////////////////////////////////////////////////////////
        if (c_inbuffer == a->n_binary) {
            c_inbuffer = read_from_sequence_file(a, binary_id);
            if (c_inbuffer == a->n_binary) {
                break;
            }
        }
        
        ///////////////////////////////////////////////////////////////////////
        // Encoder
        ///////////////////////////////////////////////////////////////////////
        assert(c_inbuffer < a->n_binary);
        assert(c_outbuffer < a->n_binary);
        /* setup of inbuf */
        uint8_t type_infile = inbuf_main[ALDER_KMER_BINARY_INBUFFER_TYPE_INFILE];
        assert(type_infile == ALDER_KMER_BINARY_FILETYPE_FASTQ);
        cur_inbuf = to_size(inbuf_main,ALDER_KMER_BINARY_INBUFFER_CURRENT);
        len_inbuf = to_size(inbuf_main,ALDER_KMER_BINARY_INBUFFER_LENGTH);
        /* setup of outbuf */
        cur_outbuf = to_size(outbuf_main,ALDER_KMER_BINARY_OUTBUFFER_CURRENT);
        len_outbuf = to_size(outbuf_main,ALDER_KMER_BINARY_OUTBUFFER_LENGTH);
        if (cur_outbuf == 0) {
            // memcpy(outbuf, "FSTQ", 4); not needed because this is fixed.
            opos_rblock_end = a->size_fixed_inbuf;
            opos_rblock_byte = 4;
            cur_outbuf = ALDER_KMER_BINARY_READBLOCK_BODY;
            // A new outbuffer and state 1 would skip determining the state of
            // inbuf in the while-loop. So, I'd need to move cur_outbuf.
            if (s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_1) {
                opos_e_len = cur_outbuf;
                cur_outbuf += sizeof(e_len);
            }
        }
        
        while (cur_inbuf < len_inbuf && cur_outbuf < len_outbuf) {
            ///////////////////////////////////////////////////////////////////
            // Determine the state of inbuf.
            // Change cur_inbuf.
            while (s_inbuffer != ALDER_KMER_BINARY_INBUFFER_STATE_1 &&
                   cur_inbuf < len_inbuf) {
                if (s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_4) {
                    // Find a next ACGT and set the state 1.
                    int d = 4;
                    while (cur_inbuf < len_inbuf && d == 4) {
                        c = inbuf[cur_inbuf++];
//                        putc(c, stdout);
                        if (c == '\n') {
                            break;
                        }
                        d = dna_char2int[c];
                    }
                    //
                    if (d < 4) {
                        s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_1;
                        cur_inbuf--;
                        opos_e_len = cur_outbuf;
                        cur_outbuf += sizeof(e_len);
                        break;
                    }
                    // If not found, a new line is found change the state.
                    if (c == '\n') {
                        s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_2;
                    }
                    // If not found and no inbuf, then set end of inbuf.
                    if (cur_inbuf == len_inbuf) {
                        continue;
                    }
                }
                uint8_t *cinbuf = memchr(inbuf + cur_inbuf, '\n',
                                         len_inbuf - cur_inbuf);
                if (cinbuf != NULL) {
                    cur_inbuf = (size_t)(cinbuf - inbuf + 1);
                    if (s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_0) {
                        // header to sequence
                        s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_1;
                        c = inbuf[cur_inbuf];
                        assert(c != '\n');
                        int d = dna_char2int[c];
                        if (!(d < 4)) {
                            s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_4;
                            continue;
                        }
                        opos_e_len = cur_outbuf;
                        cur_outbuf += sizeof(e_len);
                    } else if (s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_2) {
                        // + to quality score
                        s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_3;
                    } else if (s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_3) {
                        // quality score to header
                        s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_0;
                    }
                } else {
                    // End of Inbuf.
                    cur_inbuf = len_inbuf;
                }
            }
            assert(s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_1 ||
                   cur_inbuf == len_inbuf);
            //
            ///////////////////////////////////////////////////////////////////
            
            
            ///////////////////////////////////////////////////////////////////
            //
            c = '\0';
            while (cur_inbuf < len_inbuf &&
                   cur_outbuf < len_outbuf &&
                   c != '\n') {
                
                if (cur_outbuf < opos_rblock_end) {
                    c = inbuf[cur_inbuf++];
                    putc(c, stdout);
                    
                    ///////////////////////////////////////////////////////////
                    // Encode it.
                    int d = dna_char2int[c];
                    if (d < 4) {
                        e_byte |= dna2byte[d][e_4counter];
                        e_len++;
                        e_4counter++;
                        // Increment cur_outbuf++.
                        if (e_4counter == 4) {
                            outbuf[cur_outbuf++] = e_byte;
                            e_byte = 0;
                            e_4counter = 0;
                        }
                    } else {
                        // End the current sequence.
                        if (e_len > 0) {
                            // Increment cur_outbuf++ for unfinishied part.
                            if (e_4counter > 0) {
                                outbuf[cur_outbuf++] = e_byte;
                                e_byte = 0;
                                e_4counter = 0;
                            }
                            assert(e_byte == 0);
                            assert(e_4counter == 0);
                            to_uint16(outbuf, opos_e_len) = e_len;
                            e_len = 0;
                        } else {
                            assert(e_4counter == 0);
                        }
                        if (c == '\n') {
                            s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_2;
                        } else {
                            s_inbuffer = ALDER_KMER_BINARY_INBUFFER_STATE_4;
                            c = '\n'; // let it exit this while-loop.
                        }
                    }
                }
                
                // Check if this is the end of a read-block.
                if (cur_outbuf >= opos_rblock_end &&
                    s_inbuffer == ALDER_KMER_BINARY_INBUFFER_STATE_1) {
                    // FIXME
                    size_t rblock_byte = opos_e_len - (opos_rblock_byte + 8);
                    
                    to_size(outbuf, opos_rblock_byte) = rblock_byte;
                    
                    // Start a new end_subblock.
                    cur_outbuf = opos_rblock_end;
                    memcpy(outbuf + cur_outbuf, "FSTQ", 4);
                    
                    size_t prev_opos_e_len = opos_e_len;
                    
                    opos_rblock_end += a->size_fixed_inbuf;
                    opos_rblock_byte = cur_outbuf + ALDER_KMER_BINARY_READBLOCK_LEN;
                    
                    opos_e_len = cur_outbuf + ALDER_KMER_BINARY_READBLOCK_BODY;
                    cur_outbuf = opos_e_len + sizeof(e_len);
                    
                    // Move opos_rblock_end and opos_rblock_byte.
                    // Copy the previously unfinished block.
                    if (e_len > 0) {
                        //
                        size_t e_len_byte = ALDER_BYTESIZE_KMER(e_len,4);
                        rblock_byte = e_len_byte + sizeof(e_len);
                        memcpy(outbuf + opos_e_len,
                               outbuf + prev_opos_e_len, rblock_byte);
                        cur_outbuf += e_len/4;
                    } else {
                        //
                    }
                }
                //
                ///////////////////////////////////////////////////////////
                
            }
//            assert(c == '\0' || c == '\n');
//            || cur_inbuf == len_inbuf || cur_outbuf <= len_outbuf);
        }
        assert(cur_inbuf <= len_inbuf);
        if (cur_inbuf < len_inbuf) {
            to_size(inbuf_main,ALDER_KMER_BINARY_INBUFFER_CURRENT) = cur_inbuf;
        } else {
            c_inbuffer = a->n_binary;
        }
        if (cur_outbuf < len_outbuf) {
            to_size(outbuf_main,ALDER_KMER_BINARY_OUTBUFFER_CURRENT) = cur_outbuf;
        } else {
            to_size(outbuf_main,ALDER_KMER_BINARY_OUTBUFFER_CURRENT) = cur_outbuf;
            size_t rblock_byte = opos_e_len - (opos_rblock_byte + 8);
            to_size(outbuf, opos_rblock_byte) = rblock_byte;
            
            c_outbuffer = a->n_binary;
        }
        
    }
    /* Flush any remaining output buffer. */
    if (cur_outbuf > 0) {
        size_t rblock_byte = cur_outbuf - (opos_rblock_byte + 8);
        to_size(outbuf, opos_rblock_byte) = rblock_byte;
        write_to_binary_file(a, binary_id);
    }
    
    alder_log("binary(%d): END", binary_id);
    pthread_exit(NULL);
}


