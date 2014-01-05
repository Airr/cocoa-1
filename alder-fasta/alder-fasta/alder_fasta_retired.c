/**
 * This file, alder_fasta_retired.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_fasta_retired.h"

static void *encoder(void *t);
static void *readwriter(void *t);

#pragma mark implementation 1

/* This function encodes and saves kmers in multiple output buffers.
 * 
 * m - alder_encode_number_t
 */
static void *encoder(void *t)
{
    alder_thread_encoder_t *b = (alder_thread_encoder_t*)t;
    printf("Starting encoder(%d): thread.\n", b->i);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(b->k);
    uint8_t *in = (uint8_t*)b->inbuf;
    
    int *end_threebufs = malloc(sizeof(*end_threebufs)*b->n_np);
    memset(end_threebufs, 0, sizeof(*end_threebufs)*b->n_np);
    uint8_t **threebufs = malloc(sizeof(*threebufs)*b->n_np);
    memset(threebufs, 0, sizeof(*threebufs)*b->n_np);
    for (int i = 0; i < b->n_np; i++) {
        threebufs[i] = malloc(sizeof(*threebufs[i])*3);
        memset(threebufs[i], 0, sizeof(*threebufs[i])*3);
    }
    b->end_inbuf = 1; // Just for entering while loop.
    for (int i = 0; i < b->n_np; i++) {
        b->end_outbuf[i] = 0;
    }
    b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
    
    while (b->status != ALDER_THREAD_ENCODER_CAN_EXIT) {
        // 1. Keep notifying Readwriter that an Encoder needs inbuffer.
        //    Wait till Readwriter prepares the inbuffer for this Encoder.
        if (b->end_inbuf > 0) {
            printf("encoder(%d): waiting for readwriter\n", b->i);
        } else {
            printf("encoder(%d): last call readwriter\n", b->i);
            for (int i = 0; i < b->n_np; i++) {
                b->end_outbuf[i] = 0;
            }
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
        while (b->status == ALDER_THREAD_ENCODER_INBUFFER_EMPTY) {
            pthread_mutex_lock(&ec2rw_mutex);
            selected_encoder = b->i;
            pthread_cond_signal(&ec2rw_cv);
            pthread_mutex_unlock(&ec2rw_mutex);
        }
        
        // 2. Work on encoding of sequences.
        //    Sequences consists of letters, and are separated by '\n'.
        //    The readwriter thread takes care of parsing the FASTA file.
        //
        if (b->end_inbuf > 0) {
            
            // if pasted 5k/4 + 1
            int near_end_outbuf = b->size_outbuf - (b->k / 4 + 2);
            
            
            uint8_t *threebuf = NULL;
            uint8_t *outbuf = NULL;
//            uint8_t *midbuf = NULL;
            int end_threebuf = 0;
            int end_inbuf = b->end_inbuf - b->k + 1;
            int i_kmer = ALDER_FASTA_ENCODE_NEED_THREE_MORE_BYTE;
            while (i_kmer < end_inbuf) {
                // Check if this is a new seqeunce.
                // If so, skip the sequence name.
                if (in[i_kmer] == '>') {
                    while (in[i_kmer++] != '\n') {
                        if (!(i_kmer < end_inbuf)) {
                            break;
                        }
                    }
                    if (!(i_kmer < end_inbuf)) {
                        // Do something for the remaining in bufs.
                        // before exit of the while.
                        break;
                    } else {
                        // Initialize...?
                        // No code.
                    }
                }
                // 2. FIXME:
                // readwriter should copy "inbuf + 2"
                
                int kmer_start = i_kmer;
                int kmer_end = kmer_start + b->k;

                // Use the kmer m to compute the hash(m).
                // Read from kmer_start to kmer_end
                // to create a alder_encode_number_t.
                int s = alder_encode_number_kmer_with_char(m, (char*)(in + kmer_start));
                if (s != 0) {
                    char c = in[i_kmer];
                    while (c == 'A' || c == 'C' || c == 'G' || c == 'T' ||
                           c == 'a' || c == 'c' || c == 'g' || c == 't') {
                        i_kmer++;
                        if (!(i_kmer < end_inbuf)) {
                            break;
                        }
                        c = in[i_kmer];
                    }
                    if (!(i_kmer < end_inbuf)) {
                        // Do something for the remaining in bufs.
                        // before exit of the while.
                        break;
                    } else {
                        i_kmer++;
                        continue;
                    }
                }
                uint64_t hash_m = alder_encode_number_hash(m);
                uint64_t hash_m_mod_ni = hash_m % b->n_ni;
                if (hash_m_mod_ni == b->i_ni) {
                    uint64_t dj = hash_m / b->n_ni % b->n_np;
                    // Now I've found partition dj.
                    // Write the kmer to dj buffer.
                    threebuf = threebufs[dj];
                    outbuf = (uint8_t*)b->outbuf[dj];
//                    midbuf = (uint8_t*)b->midbuf[dj];
                    int end_outbuf = b->end_outbuf[dj];
                    end_threebuf = end_threebufs[dj];
                    
                    ///////////////////////////////////////////////////////////
                    // Restore unfinished nucleotides.
                    // buf_end is the size in the buf (size 3).
                    for (int j = 0; j < end_threebuf; j++) {
                        in[kmer_start - (end_threebuf - j)] = threebuf[j];
                    }
                    int kmer_point = kmer_start - end_threebuf;
                    
                    while (kmer_point < kmer_end - 3) {
                        // ACGT or 0132 -> 0x1E
                        uint8_t x = 0;
                        x |= ((in[kmer_point++])>>1)&0x03;
                        x <<= 2;
                        x |= ((in[kmer_point++])>>1)&0x03;
                        x <<= 2;
                        x |= ((in[kmer_point++])>>1)&0x03;
                        x <<= 2;
                        x |= ((in[kmer_point++])>>1)&0x03;
                        // Check if there is any outbuffer available.
                        // Choose the right output buffer using a hash function.
                        outbuf[end_outbuf++] = x;
                        if (!(end_outbuf < b->size_outbuf)) {
                            // Exit! No not here.
                            assert(0);
                        }
                    }
                    
                    // Save unfinished nucleotides.
                    for (int j = kmer_point; j < kmer_end; j++) {
                        threebuf[j - kmer_point] = in[j];
                    }
                    end_threebuf = kmer_end - kmer_point;
                    //
                    ///////////////////////////////////////////////////////////
                    end_threebufs[dj] = end_threebuf;
                    b->end_outbuf[dj] = end_outbuf;
                    
                    // Need to check whether there is enough buffer.
                    if (!(end_outbuf < near_end_outbuf)) {
                        break;
                    }
                }
                i_kmer++;
            }
            if (b->k % 4 != 0) {
                for (int pi = 0; pi < b->n_np; pi++) {
                    int x = alder_encode_pack_midbuf_save(b->k, end_threebufs[pi]);
                    int len_midbuf = b->k * x / 4;
                    if (len_midbuf > 0) {
                        int end_outbuf = b->end_outbuf[pi];
                        int save_end_outbuf = end_outbuf - len_midbuf;
                        assert(save_end_outbuf >= 0);
                        assert(len_midbuf <= b->size_midbuf);
                        memcpy(b->midbuf[pi], b->outbuf[pi] + save_end_outbuf, len_midbuf);
                        b->end_outbuf[pi] = save_end_outbuf;
                    }
                }
            }
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
    }
    printf("encoder(%d): exiting\n", b->i);
    for (int i = 0; i < b->n_np; i++) {
        XFREE(threebufs[i]);
    }
    XFREE(threebufs);
    XFREE(end_threebufs);
    alder_encode_number_destroy(m);
    pthread_exit(NULL);
}

static void *readwriter(void *t)
{
    printf("Starting readwriter(): thread.\n");
    alder_thread_rwe_t *a = (alder_thread_rwe_t*)t;
    int which_encoder = 0;
    
    // 1. Reads a chunk of data from the input file.
    int len_inbuf = 0;
    alder_fasta_streamer(&len_inbuf, a->rw->inbuf, a->rw->size_inbuf,
                         a->rw->fpin,  a->rw->k, 100);
//    int len_inbuf = (int)fread(a->rw->inbuf, 1, a->rw->size_inbuf, a->rw->fpin);
    a->rw->end_inbuf = len_inbuf;
    
    // 2. Repeat if there is a chunk of data.
    int remaining_encoder = a->n_encoder;
    while (remaining_encoder > 0) {
        // 3. Wait for a call from an Encoder.
        pthread_mutex_lock(&ec2rw_mutex);
        pthread_cond_wait(&ec2rw_cv, &ec2rw_mutex);
        // 4. Locate the Encoder in idle.
        which_encoder = selected_encoder;
        assert(which_encoder < a->n_encoder);
        alder_thread_encoder_t *ec = a->ec[which_encoder];
        // 5. Copy outbuffer of the encoder if needed
        int is_empty_outbuf = 1;
        for (int j = 0; j < a->rw->n_np; j++) {
            if (ec->end_outbuf[j] > 0) {
                is_empty_outbuf = 0;
                memcpy(a->rw->outbuf[j], ec->outbuf[j], ec->end_outbuf[j]);
                a->rw->end_outbuf[j] = ec->end_outbuf[j];
                ec->end_outbuf[j] = 0;
            }
        }
        // 6. Send part of chunk to encoder if needed.
        if (a->rw->end_inbuf > 0) {
            memcpy(ec->inbuf + ALDER_FASTA_ENCODE_NEED_THREE_MORE_BYTE, a->rw->inbuf, a->rw->end_inbuf);
            ec->end_inbuf = a->rw->end_inbuf + ALDER_FASTA_ENCODE_NEED_THREE_MORE_BYTE;
            a->rw->end_inbuf = 0;
            // 7. Let Encoder work on data.
            ec->status = ALDER_THREAD_ENCODER_INBUFFER_FULL;
            printf("readwriter(): full wakeup %d\n", which_encoder);
        } else if (is_empty_outbuf == 1) {
            remaining_encoder--;
            ec->end_inbuf = 0;
            ec->status = ALDER_THREAD_ENCODER_CAN_EXIT;
            printf("readwriter(): --- wakerup %d\n", which_encoder);
        } else {
            // The inbuffer is zero, but there were output.
            // So, encode might need to work on more data.
            // This is not the case for this example where
            // all of input are processed to create output.
            // Or, when output are there, input should be empty
            // in this case. But, when input can be processed
            // partially, I need this "else if is_empty_outbuf".
            ec->end_inbuf = 0;
            ec->status = ALDER_THREAD_ENCODER_CANNOT_EXIT;
            printf("readwriter(): ??? wakeup %d\n", which_encoder);
        }
        pthread_mutex_unlock(&ec2rw_mutex);
        
        // 8. Write outbuff to output files.
        for (int j = 0; j < a->rw->n_np; j++) {
            if (a->rw->end_outbuf[j] > 0) {
                fwrite(a->rw->outbuf[j], 1, a->rw->end_outbuf[j], a->rw->fpout[j]);
                a->rw->end_outbuf[j] = 0;
            }
        }
        // 9(1). Reads a chunk of data from the input file.
        if (len_inbuf > 0) {
//            len_inbuf = (int)fread(a->rw->inbuf, 1, a->rw->size_inbuf, a->rw->fpin);
            alder_fasta_streamer(&len_inbuf, a->rw->inbuf, a->rw->size_inbuf,
                                 a->rw->fpin, a->rw->k, 100);
            a->rw->end_inbuf = len_inbuf;
        }
    }
    pthread_exit(NULL);
}