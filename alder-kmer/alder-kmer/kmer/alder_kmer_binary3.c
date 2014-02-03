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
#include <fcntl.h>
#include <unistd.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_token_dna.h"
#include "alder_logger.h"
#include "alder_kseq_sequenceiterator.h"
#include "alder_file_size.h"
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
 *  This function opens a file; outdir/outfile.bin.
 *
 *  @param outfile outfile name
 *  @param outdir  outdir name
 *
 *  @return FILE pointer if successful, otherwise NULL
 */
static FILE * open_outfile(const char *outfile, const char *outdir)
{
    bstring bfpar = bformat("%s/%s.bin", outdir, outfile);
    ALDER_RETURN_NULL_IF_NULL(bfpar);
    FILE *fpout = fopen(bdata(bfpar), "wb");
    if (fpout == NULL) {
        bdestroy(bfpar);
        return NULL;
    }
    bdestroy(bfpar);
    return fpout;
}

/**
 *  This function writes the outbuf to file.
 *
 *  @param size_outbuf  size of outbuffer
 *  @param opos_e_len_p ?
 *  @param e_len        ?
 *  @param outbuf       outbuffer
 *  @param spareOutbuf  spare buffer
 *  @param fpout        file pointer
 *  @param cur_outbuf_p outbuffer position
 */
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


/**
 *  See alder_kmer_binary.c for alder_kmer_binary function.
 * 
 *  K is not used yet. It will be used in making a binary file for FASTA.
 *  D is not used; it could be used to check disk space.
 *  M is not used (loading to memory version).
 *  min_M_table, max_M_table are not used either (loading to memory version).
 *  nsplit is not used.
 *  
 */
int
alder_kmer_binary3(void *ptr, size_t total_size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   size_t *totalfilesize,
                   size_t *n_byte,
                   long version,
                   int K, long D, long M, long min_M_table, long max_M_table,
                   long nsplit,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile)
{
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Kmer binary setup ***");
    if (infile->qty > 0) {
        size_t total_file_size = 0;
        alder_totalfile_size(infile, &total_file_size);
        alder_log("%*s %zu (B)", width, "Total file size:", total_file_size);
    }
    
    FILE **fpout = NULL;
    *n_kmer = 0;
    *n_dna = 0;
    *n_seq = 0;
    *n_byte = 0;
    
    assert(version == 2 || version == 7);
    assert(ptr == NULL);
    assert(total_size == 0);
    assert(nsplit > 0);
    if (version == 2 || version == 7) {
        nsplit = 1;
    }
    fpout = malloc(sizeof(*fpout) * nsplit);
    if (fpout == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(fpout, 0, sizeof(*fpout) * nsplit);
    fpout[0] = open_outfile(outfile, outdir);
    
    /* Open kstream for tokens. */
    alder_kseq_sequenceiterator_t *fiter = alder_kseq_sequenceiterator_create();
    
    /* buffer */
    uint8_t e_byte = 0;
    uint16_t e_len = 0;
    int e_4counter = 0;
    size_t size_outbuf = subsize;
    assert(subsize > ALDER_KMER_BINARY_READBLOCK_BODY);
    uint8_t *spareOutbuf = NULL;
    spareOutbuf = malloc(size_outbuf);
    
    /**
     *  outbuf: 4B [lock], 4B [FSTQ], 8B [block length]
     */
//    uint8_t *outbuf = ptr;
    uint8_t *outbuf = malloc(size_outbuf + sizeof(e_len) + 1);
    memset(outbuf, 0, size_outbuf + sizeof(e_len) + 1);
    memcpy(outbuf + ALDER_KMER_BINARY_READBLOCK_TYPE, "FSTQ", 4);
    size_t cur_outbuf = ALDER_KMER_BINARY_READBLOCK_BODY;
    size_t opos_e_len = cur_outbuf;
    cur_outbuf += sizeof(e_len);
    
    int i_start = 0;
    if (infile->qty > 0) {
        i_start = 0;
    } else {
        i_start = -1;
    }
    size_t i_split = 0;
    for (int i = i_start; i < infile->qty; i++) {
        if (i == -1) {
            alder_kseq_sequenceiterator_open(fiter, NULL);
        } else {
            alder_kseq_sequenceiterator_open(fiter, bdata(infile->entry[i]));
        }
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
            
            ///////////////////////////////////////////////////////////////////
            // Parts different from prevous versions
            if (cur_outbuf >= size_outbuf) {
                // Write the buffer to file.
                write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                                 spareOutbuf, fpout[i_split % nsplit], &cur_outbuf);
                i_split++;
            }
            //
            ///////////////////////////////////////////////////////////////////
            
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
        ///////////////////////////////////////////////////////////////////////
        // Parts different from prevous versions
        // Write the buffer to file.
        write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                         spareOutbuf, fpout[i_split % nsplit], &cur_outbuf);
        i_split++;
        //
        ///////////////////////////////////////////////////////////////////////
    }
    
    for (int i = 0; i < nsplit; i++) {
        XFCLOSE(fpout[i]);
    }
    XFREE(fpout);
    
    /* Calcuate the size of all of the binary files. */
    *totalfilesize = 0;
    for (int i = 0; i < nsplit; i++) {
        bstring bfpar;
        bfpar = bformat("%s/%s.bin", outdir, outfile);
        if (bfpar == NULL) {
            XFREE(outbuf);
            XFREE(spareOutbuf);
            return ALDER_STATUS_ERROR;
        }
        size_t fsize = 0;
        alder_file_size(bdata(bfpar), &fsize);
        bdestroy(bfpar);
        *totalfilesize += fsize;
    }
    assert(*totalfilesize > 0);
    if (*totalfilesize == 0) {
        XFREE(spareOutbuf);
        alder_kseq_sequenceiterator_destroy(fiter);
        return ALDER_STATUS_ERROR;
    }
    *n_byte = 0;
    
    XFREE(outbuf);
    XFREE(spareOutbuf);
    alder_kseq_sequenceiterator_destroy(fiter);
    return 0;
}

