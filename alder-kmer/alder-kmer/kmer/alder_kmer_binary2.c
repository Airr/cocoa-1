/**
 * This file, alder_kmer_binary2.c, is part of alder-kmer.
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

#define ALDER_KMER_BINARY_READBLOCK_LEN        4
#define ALDER_KMER_BINARY_READBLOCK_BODY       12

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_token_dna.h"
#include "alder_logger.h"
#include "alder_kseq_sequenceiterator.h"

/**
 *  dna2byte[DNA][POS].
 */
static uint8_t dna2byte[4][4] = {
    {0x00,0x00,0x00,0x00},
    {0x40,0x10,0x04,0x01},
    {0x80,0x20,0x08,0x02},
    {0xC0,0x30,0x0C,0x03}
};

static
void write_to_binary2(size_t size_outbuf, size_t *opos_e_len_p, uint16_t e_len,
                      uint8_t *outbuf, uint8_t *spareOutbuf, FILE *fpout,
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
        memcpy(spareOutbuf, outbuf + *opos_e_len_p, unused);
    }
    
    to_size(outbuf, ALDER_KMER_BINARY_READBLOCK_LEN) = *opos_e_len_p - ALDER_KMER_BINARY_READBLOCK_BODY;
    size_t written = fwrite(outbuf, size_outbuf, 1, fpout);
    if (written != 1) {
        fprintf(stderr, "Error - write to the bin file.\n");
        abort();
    }
    /* Restore the saved part if any. */
    if (unused > 0) {
        memcpy(outbuf + ALDER_KMER_BINARY_READBLOCK_BODY,
               spareOutbuf, unused);
    }
    *cur_outbuf_p = (ALDER_KMER_BINARY_READBLOCK_BODY + (*cur_outbuf_p - *opos_e_len_p));
    *opos_e_len_p = ALDER_KMER_BINARY_READBLOCK_BODY;
}

//static
//void write_to_binary(size_t size_outbuf, size_t *opos_e_len_p, uint16_t e_len,
//                     uint8_t *outbuf, uint8_t *spareOutbuf, FILE *fpout,
//                     size_t *cur_outbuf_p)
//{
//    /* Length check! */
//    size_t unused = 0;
//    size_t overused = 0;
//    if (size_outbuf > *opos_e_len_p) {
//        unused = size_outbuf - *opos_e_len_p;
//    } else {
//        overused = *opos_e_len_p - size_outbuf;
//    }
//    assert(overused <= sizeof(e_len) + 1);
//    if (unused > 0) {
//        memcpy(spareOutbuf, outbuf + *opos_e_len_p, unused);
//        memset(outbuf + *opos_e_len_p, 0xFF, unused);
//    }
//    
//    to_size(outbuf, ALDER_KMER_BINARY_READBLOCK_LEN) = *opos_e_len_p - ALDER_KMER_BINARY_READBLOCK_BODY;
//
//    size_t written = fwrite(outbuf, size_outbuf, 1, fpout);
//#if !defined(NDEBUG)
//    alder_loga("A", outbuf, size_outbuf);
//    if (unused > 0) {
//        alder_loga("S", spareOutbuf, unused);
//    }
//#endif
//    if (written != 1) {
//        fprintf(stderr, "Error - write to the bin file.\n");
//        abort();
//    }
//    if (unused > 0) {
//        memcpy(outbuf + ALDER_KMER_BINARY_READBLOCK_BODY,
//               spareOutbuf, unused);
//#if !defined(NDEBUG)
//        alder_loga("B", outbuf, size_outbuf);
//#endif
//    } else {
//    }
//    *cur_outbuf_p = (ALDER_KMER_BINARY_READBLOCK_BODY +
//                     (*cur_outbuf_p - *opos_e_len_p));
//    *opos_e_len_p = ALDER_KMER_BINARY_READBLOCK_BODY;
//}

FILE * open_outfile(const char *outfile, const char *outdir)
{
    /* Open an output file. */
    bstring bfpar = bformat("%s/%s.bin", outdir, outfile);
    if (bfpar == NULL) {
        return NULL;
    }
    FILE *fpout = fopen(bdata(bfpar), "wb");
    if (fpout == NULL) {
        bdestroy(bfpar);
        return NULL;
    }
    bdestroy(bfpar);
    return fpout;
}

/**
 *  This function converts sequence files to a binary file.
 *
 *  @param version              version
 *  @param K                    kmer size
 *  @param D                    disk space
 *  @param n_nt                 number of threads
 *  @param totalfilesize        total file size
 *  @param n_byte               n byte
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param infile               infile
 *  @param outdir               out directory
 *  @param outfile              out file prefix
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_binary2(long version,
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
    FILE *fpout = open_outfile(outfile, outdir);
    if (fpout == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    /* Open kstream for tokens. */
    alder_kseq_sequenceiterator_t *fiter =
    alder_kseq_sequenceiterator_create();
    
    /* buffer */
    uint8_t e_byte = 0;
    uint16_t e_len = 0;
    int e_4counter = 0;
    size_t size_outbuf = (1 << 16);
//    size_t size_outbuf = ALDER_KMER_BINARY_READBLOCK_BODY + 9;
    uint8_t *spareOutbuf = NULL;
    spareOutbuf = malloc(size_outbuf);
    uint8_t *outbuf = NULL;
    outbuf = malloc(size_outbuf + sizeof(e_len) + 1);
    memcpy(outbuf, "FSTQ", ALDER_KMER_BINARY_READBLOCK_LEN);
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
                write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                                 spareOutbuf, fpout, &cur_outbuf);
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
        write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                         spareOutbuf, fpout, &cur_outbuf);
    }
    
    
    XFREE(outbuf);
    XFREE(spareOutbuf);
    alder_kseq_sequenceiterator_destroy(fiter);
    return 0;
}






