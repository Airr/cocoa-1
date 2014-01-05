#pragma mark alder_fileseq_kmer.c

/**
 * This file, alder_fileseq_kmer.c, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "alder_file.h"
#include "alder_cmacro.h"
#include "bstrmore.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_kmer.h"

/* This function converts input sequence files to files with kmers.
 * returns
 * 0 on success
 * 1 if an error occurs
 *
 * 1. Create n_partition files in directory dout.
 */
int alder_fileseq_kmer_encode(int n_thread,
                              int i_iteration,
                              int kmer_size,
                              long disk_space,
                              long memory_available,
                              uint64_t n_iteration,
                              uint64_t n_partition,
                              struct bstrList *infile,
                              bstring dout)
{
    int s = 0;
    
    // 1. Create n_partition files in directory dout.
    s = alder_file_mkdir(bdata(dout));
    ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(s,s);
    FILE **fpout = malloc(sizeof(*fpout)*n_partition);
    ALDER_RETURN_ERROR_IF_NULL(fpout, ALDER_STATUS_ERROR);
    
    for (int i = 0; i < n_partition; i++) {
        bstring bfile = bformat("%s/%d",dout->data,i);
        fpout[i] = fopen(bdata(bfile), "w");
        if (fpout[i] == NULL) {
            XFREE(fpout);
            return ALDER_STATUS_ERROR;
        }
        bdestroy(bfile);
    }
    
    // I would have readwriter/encoder here in fileseq not in each
    // file type. Besides, I would not want to create and destory threads
    // for each file type. The whole files are the input not each one of them.
    // Let's leave the thread code in fasta module. Use some of them here.
    for (int i = 0; i < infile->qty; i++) {
        //
        int t = alder_fileseq_type(bdata(infile->entry[i]));
        if (t & ALDER_FILETYPE_FASTA) {
            alder_fasta_kmer_encode(n_thread, i_iteration, kmer_size,
                                    disk_space,
                                    memory_available,
                                    n_iteration, n_partition,
                                    fpout, bdata(infile->entry[i]));
        }
    }
    
    for (int i = 0; i < n_partition; i++) {
        fclose(fpout[i]);
    }
    XFREE(fpout);
    return ALDER_STATUS_SUCCESS;
}


#pragma mark alder_fileseq_kmer.c

/* Encoder thread accesses this type.
 */
typedef struct alder_fileseq_kmer_thread_encoder_struct {
    int i;             /* encoder ID                                  */
    int k;             /* kmer size                                   */
    int status;        /* status of encoder's buffers                 */
    uint64_t i_ni;     /* iteration index                             */
    uint64_t n_ni;     /* number of iteration                         */
    uint64_t n_np;     /* number of out buffers or partitions         */
    int size_inbuf;    /* size of the input buffer                    */
    int size_outbuf;   /* size of the output buffer                   */
    int size_midbuf;   /* size of the output buffer                   */
    uint64_t nb;       /* for saving inbuf's header                   */
    uint64_t i_nb;     /* for saving inbuf's index within a sequence  */
    uint64_t i_kmer;   /* for saving inbuf's index                    */
    int end_inbuf;     /* ending position of input buffer             */
    int *end_outbuf;   /* ending position of output buffer            */
    int *end_midbuf;   /* ending position of mid buffer               */
    int *end_threebuf; /* ending position of three buffer             */
    char *inbuf;       /* in buffer (size_inbuf)                      */
    char **outbuf;     /* out buffers (n_np x size_outbuf)            */
    char **midbuf;     /* for saving unfinished encoding              */
    char **threebuf;   /* for saving unfinished input buffer          */
} alder_fileseq_kmer_thread_encoder_t;

/* This function encodes and saves kmers in multiple output buffers.
 *
 * m - alder_encode_number_t
 */
static void *encoder2(void *t)
{
    uint64_t encoding_count = 0;
    alder_fileseq_kmer_thread_encoder_t *b = (alder_fileseq_kmer_thread_encoder_t*)t;
    printf("Starting encoder(%d): thread.\n", b->i);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(b->k);
    
    b->nb = 0;        // Just for getting data in the first time
    b->end_inbuf = 1; // Just for entering while loop.
    for (int i = 0; i < b->n_np; i++) {
        b->end_outbuf[i] = 0;
    }
    b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
    
    while (b->status != ALDER_THREAD_ENCODER_CAN_EXIT) {
        // 1. Keep notifying Readwriter that an Encoder needs inbuffer.
        //    Wait till Readwriter prepares the inbuffer for this Encoder.
        //
        if (b->end_inbuf > 0) {
            printf("encoder(%d): waiting for the readwriter to have an input buffer\n", b->i);
        } else {
            printf("encoder(%d): no input - a call to readwriter\n", b->i);
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
        fprintf(stdout, "\n");
        while (b->status == ALDER_THREAD_ENCODER_INBUFFER_EMPTY) {
            pthread_mutex_lock(&ec2rw_mutex);
            selected_encoder = b->i;
            pthread_cond_signal(&ec2rw_cv);
            pthread_mutex_unlock(&ec2rw_mutex);
        }
        fprintf(stdout, "encoder(%d): starting working...(%llu-th)\n", b->i, encoding_count++);
        fprintf(stdout, "\n");
        
        // 2. Work on encoding of sequences.
        //    Sequences consists of numbers with a header of number of
        //    those numbers or bases.
        //    The readwriter thread takes care of parsing the FASTA file.
        //
        // Move any unfished buffer to out buffer.
        for (int pi = 0; pi < b->n_np; pi++) {
            if (b->end_midbuf[pi] > 0) {
                memcpy(b->outbuf[pi], b->midbuf[pi], b->end_midbuf[pi]);
                b->end_outbuf[pi] = b->end_midbuf[pi];
                b->end_midbuf[pi] = 0;
            }
        }
        if (b->end_inbuf > 0) {
            uint8_t *inbuf = (uint8_t*)b->inbuf;
            int near_end_outbuf = b->size_outbuf - (b->k / 4 + 2);
            int near_overflow_outbuf = 0;
            int end_inbuf = b->end_inbuf - b->k + 1;
            uint64_t i_kmer = ALDER_FASTA_ENCODE_NEED_THREE_MORE_BYTE;
            while (i_kmer < end_inbuf && near_overflow_outbuf == 0) {
                uint64_t nb;
                uint64_t i_nb;
                if (b->nb > 0) {
                    nb = b->nb;
                    i_kmer = b->i_kmer;
                    i_nb = b->i_nb;
                } else {
                    uint8_t *sep = memchr(inbuf + i_kmer, 0xFF, b->end_inbuf - i_kmer);
                    if (sep == NULL) {
                        nb = b->end_inbuf - i_kmer;
                    } else {
                        nb = (sep - inbuf)/sizeof(*sep) - i_kmer;
                    }
                    assert(nb >= b->k);
                    i_nb = 0;
                }
                for (; i_nb < nb - b->k + 1; i_nb++) {
                    
                    uint64_t kmer_start = i_kmer;
                    uint64_t kmer_end = kmer_start + b->k;
                    
                    // Use the kmer m to compute the hash(m).
                    // Read from kmer_start to kmer_end
                    // to create a alder_encode_number_t.
                    int s = alder_encode_number_kmer(m, (inbuf + kmer_start));
                    assert(s == 0);
                    uint64_t hash_m = alder_encode_number_hash(m);
                    uint64_t hash_m_mod_ni = hash_m % b->n_ni;
                    if (hash_m_mod_ni == b->i_ni) {
                        fprintf(stdout, "  encoder(%d) encoding...(%llu)\n",
                                b->i, i_nb);
                        uint64_t dj = hash_m / b->n_ni % b->n_np;
                        // Now I've found partition dj.
                        // Write the kmer to dj buffer.
                        uint8_t *outbuf = (uint8_t*)b->outbuf[dj];
                        uint8_t *threebuf = (uint8_t*)b->threebuf[dj];
                        int end_outbuf = b->end_outbuf[dj];
                        int end_threebuf = b->end_threebuf[dj];
                        
                        ///////////////////////////////////////////////////////
                        // Restore unfinished nucleotides.
                        // buf_end is the size in the buf (size 3).
                        for (int j = 0; j < end_threebuf; j++) {
                            inbuf[kmer_start - (end_threebuf - j)] = threebuf[j];
                        }
                        uint64_t kmer_point = kmer_start - end_threebuf;
                        
                        while (kmer_point < kmer_end - 3) {
                            // ACGT or 0132 -> 0x1E
                            uint8_t x = 0;
                            x |= inbuf[kmer_point++]; x <<= 2;
                            x |= inbuf[kmer_point++]; x <<= 2;
                            x |= inbuf[kmer_point++]; x <<= 2;
                            x |= inbuf[kmer_point++];
                            outbuf[end_outbuf++] = x;
                        }
                        
                        // Save unfinished nucleotides.
                        for (uint64_t j = kmer_point; j < kmer_end; j++) {
                            threebuf[j - kmer_point] = inbuf[j];
                        }
                        end_threebuf = (int)(kmer_end - kmer_point);
                        //
                        ///////////////////////////////////////////////////////
                        b->end_threebuf[dj] = end_threebuf;
                        b->end_outbuf[dj] = end_outbuf;
                        
                        // Need to check whether there is enough buffer.
                        if (!(end_outbuf < near_end_outbuf)) {
                            near_overflow_outbuf = 1; // let it break while.
                            b->nb = nb;
                            b->i_kmer = i_kmer + 1;
                            b->i_nb = i_nb + 1;
                            i_kmer++;
                            break;
                        }
                    }
                    i_kmer++;
                }
                
            }
            if (i_kmer == end_inbuf ) {
                b->nb = 0;
            }
            
            if (b->k % 4 != 0) {
                for (int pi = 0; pi < b->n_np; pi++) {
                    assert(b->end_threebuf[pi] < 4);
                    int x = alder_encode_pack_midbuf_save(b->k, b->end_threebuf[pi]);
                    int len_midbuf = b->k * x / 4;
                    if (len_midbuf > 0) {
                        int end_outbuf = b->end_outbuf[pi];
                        int save_end_outbuf = end_outbuf - len_midbuf;
                        assert(save_end_outbuf >= 0);
                        assert(len_midbuf <= b->size_midbuf);
                        memcpy(b->midbuf[pi], b->outbuf[pi] + save_end_outbuf, len_midbuf);
                        b->end_outbuf[pi] = save_end_outbuf;
                        b->end_midbuf[pi] = len_midbuf;
                    }
                }
            }
            
            // Print info
            fprintf(stdout, "encode(%d) done\n", b->i);
            for (int i = 0; i < b->n_np; i++) {
                fprintf(stdout, "out[%d](%d) ", i, b->end_outbuf[i]);
                for (int j = 0; j < b->end_outbuf[i]; j++) {
                    fprintf(stdout, "%hhx ", b->outbuf[i][j]);
                }
                fprintf(stdout, "\n");
                
                fprintf(stdout, "mid[%d](%d) ", i, b->end_midbuf[i]);
                for (int j = 0; j < b->end_midbuf[i]; j++) {
                    fprintf(stdout, "%hhx ", b->midbuf[i][j]);
                }
                fprintf(stdout, "\n");
                
                fprintf(stdout, "three[%d](%d) ", i, b->end_threebuf[i]);
                for (int j = 0; j < b->end_threebuf[i]; j++) {
                    fprintf(stdout, "%hhx ", b->threebuf[i][j]);
                }
                fprintf(stdout, "\n");
            }
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        } else {
            // No input is there, so I need to flush out all of the outbuffer.
            // NOT YET!!! THERE ARE OTHER FILES TO PROCESS.
            for (int pi = 0; pi < b->n_np; pi++) {
                if (b->end_threebuf[pi] > 0) {
                    uint8_t x = 0;
                    for (int j = 0; j < 3; j++) {
                        x |= b->threebuf[pi][j]; x <<= 2;
                    }
                    int end_outbuf = b->end_outbuf[pi];
                    b->outbuf[pi][end_outbuf++] = x;
                    b->end_outbuf[pi] = end_outbuf;
                    b->end_threebuf[pi] = 0;
                }
            }
            //            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
    }
    printf("encoder(%d): exiting\n", b->i);
    alder_encode_number_destroy(m);
    pthread_exit(NULL);
}

#pragma mark thread argument

/* This function prepares a data structure for encoder thread.
 * returns
 * alder_fileseq_kmer_thread_encoder_t on success
 * nil if an error occurs.
 */
alder_fileseq_kmer_thread_encoder_t * alder_fileseq_kmer_thread_encoder_create(int id,
                                                     uint64_t i_iteration,
                                                     uint64_t n_iteration,
                                                     uint64_t n_np,
                                                     int kmer_size,
                                                     int size_inbuf,
                                                     int size_outbuf)
{
    /* initialization */
    alder_fileseq_kmer_thread_encoder_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->i = id;
    o->k = kmer_size;
    o->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
    o->i_ni = i_iteration;
    o->n_ni = n_iteration;
    o->n_np = n_np;
    o->size_inbuf = size_inbuf;
    o->size_outbuf = size_outbuf;
    o->size_midbuf = alder_encode_pack_midbuf_size(kmer_size);
    o->end_inbuf = 0;
    o->end_outbuf = NULL;
    o->inbuf = NULL;
    o->outbuf = NULL;
    o->midbuf = NULL;
    o->end_outbuf = NULL;
    
    
    /* memory allocation */
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*n_np);
    o->end_midbuf = malloc(sizeof(*o->end_midbuf)*n_np);
    o->end_threebuf = malloc(sizeof(*o->end_threebuf)*n_np);
    o->inbuf = malloc(sizeof(*o->inbuf) * (size_inbuf + ALDER_FASTA_ENCODE_NEED_THREE_MORE_BYTE)); // Two more bytes.
    if (o->end_outbuf == NULL ||
        o->end_midbuf == NULL ||
        o->end_threebuf == NULL ||
        o->inbuf == NULL) {
        alder_fileseq_kmer_thread_encoder_destroy(o);
        return NULL;
    }
    o->outbuf = malloc(sizeof(*o->outbuf)*n_np);
    if (o->outbuf == NULL) {
        alder_fileseq_kmer_thread_encoder_destroy(o);
        return NULL;
    }
    o->midbuf = malloc(sizeof(*o->midbuf)*n_np);
    if (o->midbuf == NULL) {
        alder_fileseq_kmer_thread_encoder_destroy(o);
        return NULL;
    }
    o->threebuf = malloc(sizeof(*o->threebuf)*n_np);
    if (o->threebuf == NULL) {
        alder_fileseq_kmer_thread_encoder_destroy(o);
        return NULL;
    }
    
    memset(o->outbuf, 0, sizeof(*o->outbuf)*n_np);
    memset(o->midbuf, 0, sizeof(*o->midbuf)*n_np);
    memset(o->threebuf, 0, sizeof(*o->threebuf)*n_np);
    for (int i = 0; i < n_np; i++) {
        o->outbuf[i] = malloc(sizeof(*o->outbuf[i])*size_outbuf);
        if (o->outbuf[i] == NULL) {
            alder_fileseq_kmer_thread_encoder_destroy(o);
            return NULL;
        }
        o->end_outbuf[i] = 0;
        
        o->midbuf[i] = malloc(sizeof(*o->midbuf[i])*o->size_midbuf);
        if (o->midbuf[i] == NULL) {
            alder_fileseq_kmer_thread_encoder_destroy(o);
            return NULL;
        }
        o->end_midbuf[i] = 0;
        
        o->threebuf[i] = malloc(sizeof(*o->threebuf[i])*3);
        if (o->threebuf[i] == NULL) {
            alder_fileseq_kmer_thread_encoder_destroy(o);
            return NULL;
        }
        memset(o->threebuf[i], 0, sizeof(*o->threebuf[i])*3);
        o->end_threebuf[i] = 0;
    }
    return o;
}

void alder_fileseq_kmer_thread_encoder_destroy(alder_fileseq_kmer_thread_encoder_t *o)
{
    if (o != NULL) {
        if (o->threebuf != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                XFREE(o->threebuf[i]);
            }
        }
        XFREE(o->threebuf);
        if (o->midbuf != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                XFREE(o->midbuf[i]);
            }
        }
        XFREE(o->midbuf);
        if (o->outbuf != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                XFREE(o->outbuf[i]);
            }
        }
        XFREE(o->outbuf);
        
        XFREE(o->inbuf);
        XFREE(o->end_threebuf);
        XFREE(o->end_midbuf);
        XFREE(o->end_outbuf);
    }
}

/* This function prepares a data structure for readwriter thread.
 * returns
 * object on success
 * nil if an error occurs
 */
alder_fileseq_kmer_thread_readwriter_t *
alder_fileseq_kmer_thread_readwriter_create(int n_encoder,
                                            int i_iteration,
                                            int kmer_size,
                                            long disk_space,
                                            long memory_available,
                                            uint64_t n_iteration,
                                            uint64_t n_partition,
                                            struct bstrList *infile,
                                            bstring dout)
{
    int s = 0;
    int size_inbuf = (int) alder_fasta_kmer_encode_inbuf_size((int)memory_available,
                                                               n_encoder, kmer_size);
    int size_outbuf = (int) alder_fasta_kmer_encode_outbuf_size((size_t) size_inbuf,
                                                                 n_partition, kmer_size);
    size_inbuf = 70;
    size_outbuf = 70;
    
    
    alder_fileseq_kmer_thread_readwriter_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->infile = NULL;
    o->dout = NULL;
    o->fpin = NULL;
    o->i_infile = 0;
    o->fpout = NULL;
    o->end_outbuf = NULL;
    o->inbuf = NULL;
    o->inbuf2 = NULL;
    o->outbuf = NULL;
    
    o->n_ni = n_iteration;
    o->k = kmer_size;
    o->n_np = n_partition;
    o->begin_inbuf = 0;
    o->end_inbuf = 0;
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*n_partition);
    
    
    o->infile = bstrVectorCopy(infile);
    o->dout = bstrcpy(dout);
    o->size_inbuf = size_inbuf;
    o->size_outbuf = size_outbuf;
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * size_inbuf);
    o->outbuf = malloc(sizeof(*o->outbuf) * n_partition);
    
    if (o->infile == NULL ||
        o->dout == NULL ||
        o->end_outbuf == NULL ||
        o->inbuf == NULL ||
        o->inbuf2 == NULL ||
        o->outbuf == NULL) {
        alder_fileseq_kmer_thread_readwriter_destroy(o);
        return NULL;
    }
    
    for (int j = 0; j < n_partition; j++) {
        o->outbuf[j] = NULL;
        o->end_outbuf[j] = 0;
    }
    for (int j = 0; j < n_partition; j++) {
        o->outbuf[j] = malloc(sizeof(*o->outbuf[j]) * size_outbuf);
        if (o->outbuf[j] == NULL) {
            alder_fileseq_kmer_thread_readwriter_destroy(o);
            return NULL;
        }
    }
    
    // 1. Create n_partition files in directory dout.
    s = alder_file_mkdir(bdata(dout));
    if (s != 0) {
        alder_fileseq_kmer_thread_readwriter_destroy(o);
        return NULL;
    }
    
    FILE **fpout = malloc(sizeof(*fpout)*n_partition);
    if (fpout == NULL) {
        alder_fileseq_kmer_thread_readwriter_destroy(o);
        return NULL;
    }
    memset(fpout, 0, sizeof(*fpout)*n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfile = bformat("%s/%d",dout->data,i);
        fpout[i] = fopen(bdata(bfile), "w");
        if (fpout[i] == NULL) {
            alder_fileseq_kmer_thread_readwriter_destroy(o);
            return NULL;
        }
        bdestroy(bfile);
    }
    o->fpout = fpout;
    
    
    // Encoder
    o->n_encoder = n_encoder;
    o->ec = NULL;
    
    o->ec = malloc(sizeof(*o->ec)*n_encoder);
    if (o->ec == NULL) {
        alder_fileseq_kmer_thread_readwriter_destroy(o);
        return NULL;
    }
    memset(o->ec, 0, sizeof(*o->ec)*n_encoder);
    for (int i = 0; i < n_encoder; i++) {
        o->ec[i] = alder_fileseq_kmer_thread_encoder_create(i,
                                                            i_iteration,
                                                            n_iteration, n_partition,
                                                            kmer_size,
                                                            size_inbuf, size_outbuf);
        if (o->ec[i] == NULL) {
            alder_fileseq_kmer_thread_readwriter_destroy(o);
            return NULL;
        }
    }

    return o;
}

/* fpout is not deallocated because it is just a copy of pointer.
 * In other words, alder_fileseq_kmer_thread_readwriter_t does not own fpout.
 * See alder_fileseq_kmer_thread_readwriter_create to see that.
 */
void alder_fileseq_kmer_thread_readwriter_destroy(alder_fileseq_kmer_thread_readwriter_t *o)
{
    if (o != NULL) {
        XFCLOSE(o->fpin);
        for (int j = 0; j < o->n_np; j++) {
            XFREE(o->outbuf[j]);
        }
        bdestroy(o->dout);
        bstrVectorDelete(o->infile);
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        XFREE(o->inbuf2);
        XFREE(o->end_outbuf);
        for (int j = 0; j < o->n_encoder; j++) {
            alder_fileseq_kmer_thread_encoder_destroy(o->ec[j]);
        }
        // Close outfiles.
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
            }
        }
    }
}