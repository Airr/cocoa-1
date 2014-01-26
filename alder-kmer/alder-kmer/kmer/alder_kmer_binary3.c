/**
 * This file, alder_kmer_binary3.c, is part of alder-kmer.
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
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_token_dna.h"
#include "alder_logger.h"
#include "alder_kseq_sequenceiterator.h"
#include "alder_kmer_binary.h"

/**
 *  dna2byte[DNA][POS].
 */
static uint8_t dna2byte[4][4] = {
    {0x00,0x00,0x00,0x00},
    {0x40,0x10,0x04,0x01},
    {0x80,0x20,0x08,0x02},
    {0xC0,0x30,0x0C,0x03}
};

/**
 *  This function copies some of outbuf to fpout. See write_to_binary2.
 *
 *  @param size_outbuf  fixed size of outbuf
 *  @param opos_e_len_p ?
 *  @param e_len        ?
 *  @param outbuf       outbuf
 *  @param spareOutbuf  spare buf
 *  @param fpout        destination buffer
 *  @param cur_outbuf_p ?
 */
static
void write_to_binary3(size_t size_outbuf, size_t *opos_e_len_p, uint16_t e_len,
                      uint8_t **outbuf, uint8_t *spareOutbuf,
                      size_t *cur_outbuf_p)
{
    /* Length check! */
    size_t unused = 0;
    assert(size_outbuf >= *opos_e_len_p);
    if (size_outbuf < *opos_e_len_p) {
        fprintf(stderr, "Error - size_outbuf must be greater than or equal to opos.\n");
        abort();
    }
    /* Save the part that would not be written. */
    unused = size_outbuf - *opos_e_len_p;
    if (unused > 0) {
        memcpy(spareOutbuf, *outbuf + *opos_e_len_p, unused);
    }
    
    to_size(*outbuf, ALDER_KMER_BINARY_READBLOCK_LEN) = *opos_e_len_p - ALDER_KMER_BINARY_READBLOCK_BODY;
    
    ///////////////////////////////////////////////////////////////////////////
    // THIS IS THE ONLY DIFFERENT PART FROM write_to_binary2.
//    memcpy(fpout, outbuf, size_outbuf);
    *outbuf += size_outbuf;
    //
    ///////////////////////////////////////////////////////////////////////////
    
    /* Restore the saved part if any. */
    if (unused > 0) {
        memcpy(*outbuf + ALDER_KMER_BINARY_READBLOCK_BODY,
               spareOutbuf, unused);
    }
    *cur_outbuf_p = (ALDER_KMER_BINARY_READBLOCK_BODY + (*cur_outbuf_p - *opos_e_len_p));
    *opos_e_len_p = ALDER_KMER_BINARY_READBLOCK_BODY;
}

/**
 *  This function reads a set of sequence data to create a buffer.
 *
 *  @param ptr                  pointer to the memory         <- relevant
 *  @param size                 size of the memory            <- relevant
 *  @param subsize              block size of the memory      <- relevant
 *  @param version              version
 *  @param K                    kmer size
 *  @param D                    disk space
 *  @param n_nt                 number of thread
 *  @param totalfilesize        total file size
 *  @param n_byte               ending position in the buffer <- relevant
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param infile               input files                   <- relevant
 *  @param outdir               output directory
 *  @param outfile              output file prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_binary3(void *ptr, size_t size, size_t subsize,
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
    alder_log("Binary command version: %ld", version);
    *n_kmer = 0;
    *n_dna = 0;
    *n_seq = 0;
    *n_byte = 0;
    
    /* Open kstream for tokens. */
    alder_kseq_sequenceiterator_t *fiter =
    alder_kseq_sequenceiterator_create();
    
    /* buffer */
    uint8_t e_byte = 0;
    uint16_t e_len = 0;
    int e_4counter = 0;
    size_t size_outbuf = subsize;
    assert(subsize > ALDER_KMER_BINARY_READBLOCK_BODY);
    //    size_t size_outbuf = ALDER_KMER_BINARY_READBLOCK_BODY + 9;
    uint8_t *spareOutbuf = NULL;
    spareOutbuf = malloc(size_outbuf);
    
    
    /**
     *  outbuf: 4B [lock], 4B [FSTQ], 8B [block length]
     */
    uint8_t *outbuf = ptr;
//    outbuf = malloc(size_outbuf + sizeof(e_len) + 1);
//    memset(outbuf, 0, sizeof(uint32_t));
    memcpy(outbuf + ALDER_KMER_BINARY_READBLOCK_TYPE, "FSTQ", 4);
    size_t cur_outbuf = ALDER_KMER_BINARY_READBLOCK_BODY;
    size_t opos_e_len = cur_outbuf;
    cur_outbuf += sizeof(e_len);
    
    for (int i = 0; i < infile->qty; i++) {
        alder_kseq_sequenceiterator_open(fiter, bdata(infile->entry[i]));
        int d = alder_kseq_sequenceiterator_tokenACTG(fiter);
        while (d <= ALDER_TOKEN_DNA_SIZE + 1) {
            
            if (d < ALDER_TOKEN_DNA_SIZE) {
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
                    if (K <= e_len) {
                        *n_kmer += (e_len + 1 - K);
                    }
                    (*n_seq)++;
                    *n_dna += (uint64_t)e_len;
                    
                    e_len = 0;
                    
                    ///////////////////////////////////////////////////////////
                    // Different from the previous version.
                    opos_e_len = cur_outbuf;
                    cur_outbuf += sizeof(e_len);
                    //
                    ///////////////////////////////////////////////////////////
                } else {
                    assert(e_4counter == 0);
                }
            }
            
            if (cur_outbuf >= size_outbuf) {
                write_to_binary3(size_outbuf, &opos_e_len, e_len, &outbuf,
                                 spareOutbuf, &cur_outbuf);
            }
            
            if (d > ALDER_TOKEN_DNA_SIZE) {
                break;
            }
            
            d = alder_kseq_sequenceiterator_tokenACTG(fiter);
        }
        alder_kseq_sequenceiterator_close(fiter);
    }
    // flush it!
    // Note that opos_e_len must be moved from the starting position of 12
    // if there are any sequences in the output buffer.
    if (opos_e_len > ALDER_KMER_BINARY_READBLOCK_BODY) {
        assert(cur_outbuf < size_outbuf);
        write_to_binary3(size_outbuf, &opos_e_len, e_len, &outbuf,
                         spareOutbuf, &cur_outbuf);
    }
    
    *n_byte = (outbuf - (uint8_t*)ptr);
    
//    XFREE(outbuf);
    XFREE(spareOutbuf);
    alder_kseq_sequenceiterator_destroy(fiter);
    return 0;
}
