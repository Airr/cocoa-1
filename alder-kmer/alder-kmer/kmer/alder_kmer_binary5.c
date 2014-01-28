/**
 * This file, alder_kmer_binary5.c, is part of alder-kmer.
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
 *  This function creates a binary file to write outbuffer to.
 *
 *  @param outfile outfile name prefix.
 *  @param outdir  output directory.
 *
 *  @return file pointer
 */
static FILE * open_outfile(const char *outfile, const char *outdir)
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
 *  This function writes the initial content to the open binary file.
 *
 *  @param fpout  file
 *  @param buf    buffer
 *  @param outbuf ending pointer of the buffer
 */
static
void open_to_binary5(FILE *fpout, void *buf, uint8_t *outbuf,
                     size_t size_write)
{
    /* Length check! */
    assert(fpout != NULL);
    assert((uint8_t*)buf < outbuf);
    void *cur_buf = buf;
    
    size_t left_to_write = outbuf - (uint8_t*)buf;
    while (1) {
        if (left_to_write < size_write) {
            size_write = left_to_write;
        }
        ssize_t written = write(fileno(fpout), cur_buf, size_write);
        if (written == -1) {
            break;
        } else {
            left_to_write -= written;
            cur_buf += written;
        }
    }
    assert (left_to_write == 0);
}

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
void write_to_binary5(size_t size_outbuf, size_t *opos_e_len_p, uint16_t e_len,
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
 *  This function reads a set of sequence data to either load it on the memory
 *  or to write it to a binary file. I'd call the data full if it failed to be
 *  loaded on the memory.
 *  totalfilesize > 0 and n_byte = 0 if full
 *  totalfilesize = 0 and n_byte > 0 if not full or successfully loaded on 
 *    the memory
 *
 *  @param ptr                  memory
 *  @param total_size           memory size
 *  @param subsize              block size
 *  @param n_kmer               number of kmer in the data
 *  @param n_dna                number of dna in the data
 *  @param n_seq                number of sequences
 *  @param totalfilesize        binary file size if full
 *  @param n_byte               bytes in the memory if not full
 *  @param version              version
 *  @param K                    kmer size
 *  @param D                    disk space
 *  @param M                    memory
 *  @param min_M_table          minimum memory for a partition table
 *  @param max_M_table          maximum memory for a partition table
 *  @param n_nt                 threads
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param infile               infile
 *  @param outdir               output directory
 *  @param outfile              outfile name
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_binary5(void *ptr, size_t total_size, size_t subsize,
                   uint64_t *n_kmer, uint64_t *n_dna, uint64_t *n_seq,
                   size_t *totalfilesize,
                   size_t *n_byte,
                   long version,
                   int K, long D, long M, long min_M_table, long max_M_table,
                   int n_nt,
                   int progress_flag,
                   int progressToError_flag,
                   struct bstrList *infile, const char *outdir,
                   const char *outfile)
{
    alder_log("Binary command version: %ld", version);
    FILE *fpout = NULL;
    *n_kmer = 0;
    *n_dna = 0;
    *n_seq = 0;
    *n_byte = 0;
    size_t n_size = 0;
    size_t min_size_table = ((size_t)min_M_table << 20);
    int isFull = 0;
    if (version == 5) {
        fpout = open_outfile(outfile, outdir);
        if (fpout == NULL) {
            return ALDER_STATUS_ERROR;
        }
        isFull = 1;
    }
    
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
    
    int i_start = 0;
    if (infile->qty > 0) {
        i_start = 0;
    } else {
        i_start = -1;
    }
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
                if (isFull == 0) {
                    n_size = (outbuf - (uint8_t*)ptr) + min_size_table + size_outbuf;
                    if (total_size < n_size) {
                        // Overflow of the input data.
                        // Change to version 5 by writing the input to a bin file.
                        fpout = open_outfile(outfile, outdir);
                        if (fpout == NULL) {
                            XFREE(spareOutbuf);
                            alder_kseq_sequenceiterator_destroy(fiter);
                            return ALDER_STATUS_ERROR;
                        }
                        isFull = 1;
                    }
                    write_to_binary5(size_outbuf, &opos_e_len, e_len, &outbuf,
                                     spareOutbuf, &cur_outbuf);
                    if (isFull == 1) {
                        open_to_binary5(fpout, ptr, outbuf, size_outbuf);
                        memcpy(ptr, outbuf, cur_outbuf);
                        outbuf = ptr;
                    }
                } else {
                    // Write the buffer to file.
                    write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                                     spareOutbuf, fpout, &cur_outbuf);
                }
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
        if (isFull == 0) {
            n_size = (outbuf - (uint8_t*)ptr) + min_size_table + size_outbuf;
            assert(!(total_size < n_size));
            
            if (total_size < n_size) {
                // Overflow of the input data.
                // Change to version 5 by writing the input to a bin file.
                fpout = open_outfile(outfile, outdir);
                if (fpout == NULL) {
                    XFREE(spareOutbuf);
                    alder_kseq_sequenceiterator_destroy(fiter);
                    return ALDER_STATUS_ERROR;
                }
                isFull = 1;
            }
            write_to_binary5(size_outbuf, &opos_e_len, e_len, &outbuf,
                             spareOutbuf, &cur_outbuf);
            assert(isFull == 0);
            if (isFull == 1) {
                open_to_binary5(fpout, ptr, outbuf, size_outbuf);
                *n_byte = (outbuf - (uint8_t*)ptr);
                
                memcpy(ptr, outbuf, cur_outbuf); // Impossible?
                outbuf = ptr;
            } else {
                *n_byte = (outbuf - (uint8_t*)ptr);
            }
        } else {
            // Write the buffer to file.
            write_to_binary2(size_outbuf, &opos_e_len, e_len, outbuf,
                             spareOutbuf, fpout, &cur_outbuf);
            XFCLOSE(fpout);
            
        }
        //
        ///////////////////////////////////////////////////////////////////////
    }
    
    if (isFull) {
        bstring bfpar = bformat("%s/%s.bin", outdir, outfile);
        if (bfpar == NULL) {
            return ALDER_STATUS_ERROR;
        }
        alder_file_size(bdata(bfpar), totalfilesize);
        bdestroy(bfpar);
        assert(*totalfilesize > 0);
        
        if (*totalfilesize == 0) {
            XFREE(spareOutbuf);
            alder_kseq_sequenceiterator_destroy(fiter);
            return ALDER_STATUS_ERROR;
        }
        *n_byte = 0;
    } else {
        *totalfilesize = 0;
        *n_byte = (outbuf - (uint8_t*)ptr);
    }
    
    XFREE(spareOutbuf);
    alder_kseq_sequenceiterator_destroy(fiter);
    return 0;
}
