/**
 * This file, alder_kmer_list.c, is part of alder-kmer.
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
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "alder_cmacro.h"
#include "alder_progress.h"
#include "alder_logger.h"
#include "alder_kmer_list.h"

#define BUFFER_SIZE 16*1024

/**
 *  This function lists kmers in text format.
 *
 *  @param K             kmer size
 *  @param progress_flag progress
 *  @param infile        input file name (NULL: stdin)
 *  @param outdir        out directory
 *  @param outfile       out file name (NULL: stdout)
 *
 *  @return SUCCESS or FAIL
 *
 *  http://stackoverflow.com/questions/13993742/is-there-anyway-to-peek-at-the-stdin-buffer
 */
int
alder_kmer_list(int K,
                int progress_flag,
                struct bstrList *infile,
                int outfile_given,
                const char *outdir,
                const char *outfile)
{
    /* Resources */
    // buf
    // fildes
    // fpout
    if (infile->qty > 1) {
        alder_loge(ALDER_ERR_FILE, "needs a single input file.");
        return ALDER_STATUS_ERROR;
    }
    
    /* File input using file descriptor. */
    uint8_t *buf = malloc(sizeof(*buf) * BUFFER_SIZE);
    ALDER_RETURN_ERROR_IF_NULL(buf, ALDER_STATUS_ERROR);
    
    /* Open a file or stdin. */
    int fildes = -1;
    if (infile->qty == 0) {
        fildes = 0;
    } else {
        fildes = open(bdata(infile->entry[0]), O_RDONLY);
        if (fildes == -1) {
            alder_loge(ALDER_ERR_FILE, "failed to open file: %s",
                       bdata(infile->entry[0]));
            XFREE(buf);
            return ALDER_STATUS_ERROR;
        }
        // s = fcntl(fildes, F_RDAHEAD); do not know the effect of this function
    }
    
    /* Open a file for output using file pointer. */
    FILE *fpout = NULL;
    bstring bfn = bformat("%s/%s.lst", outdir, outfile);
    if (outfile_given) {
        fpout = fopen(bdata(bfn), "w");
        if (fpout == NULL) {
            if (fildes > 0) {
                close(fildes);
            }
            XFREE(buf);
            return ALDER_STATUS_ERROR;
        }
    } else {
        fpout = stdout;
    }
    
    /* Parsing input stream. */
    int lines = 0;
    size_t n_kmer = 0;
    size_t bytes_read;
    size_t len_remaining = 0;
    uint8_t *sbuf = buf;
    int isSequence = 1;
    while((bytes_read = read(fildes, sbuf, BUFFER_SIZE - len_remaining))) {
        if (bytes_read == (size_t)-1) {
            alder_loge(ALDER_ERR_FILE, "failed to read a line file.");
            goto cleanup;
        }
        if (!bytes_read)
            break;
        
        sbuf = buf;
        for (uint8_t *p = buf;
             (p = (uint8_t*) memchr(p, '\n', (size_t)((buf + bytes_read + len_remaining) - p)));
             p++) {
            *p = '\0';
            
            ///////////////////////////////////////////////////////////////////
            // sbuf: a point to a string.
            char c = (char)*sbuf;
            if (c == '@') {
                // p -> '\n'
                isSequence = 1;
            } else if (c == '+') {
                // p -> '\n'
                isSequence = 0;
            } else if (isSequence == 1) {
                // THIS IS A SEQ BLOCK.
                int len = (int)(p - sbuf);
                //                assert(len == 100);
                for (int i = 0; i < len - K + 1; i++) {
                    size_t end_pos = K+i;
                    uint8_t c = sbuf[end_pos];
                    sbuf[end_pos] = '\0';
                    uint8_t *kbuf = sbuf + i;
                    
                    n_kmer++;
                    if (strspn((char*)kbuf, "ACGTacgt") == K) {
                        fprintf(fpout, "%s\n", kbuf);
                    }
                    sbuf[end_pos] = c;
                }
                
            } else {
                // THIS IS A QUAL BLOCK.
                //                fprintf(stdout, "QUALITY SCORE!\n");
            }
            // END
            ///////////////////////////////////////////////////////////////////
            
            *p = '\n';
            uint8_t *ebuf = buf + bytes_read + len_remaining;
            if (ebuf > p) {
                sbuf = p + 1;
            }
            ++lines;
        }
        len_remaining += (size_t)((buf + bytes_read) - sbuf);
        if (len_remaining > 0) {
            memmove(buf, sbuf,len_remaining);
        }
        sbuf = buf + len_remaining;
    }
cleanup:
    XFREE(buf);
    if (infile->qty > 0) {
        close(fildes);
    }
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return ALDER_STATUS_SUCCESS;
}
