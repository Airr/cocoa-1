/**
 * This file, alder_kmer_match.c, is part of alder-kmer.
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
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_progress.h"
#include "alder_logger.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_match.h"

#define BUFFER_SIZE 16*1024

int
alder_kmer_match(int progress_flag,
                 struct bstrList *infile,
                 const char *tabfile,
                 int outfile_given,
                 const char *outdir,
                 const char *outfile)
{
    int status = ALDER_STATUS_SUCCESS;
    int s = ALDER_STATUS_SUCCESS;
    
    /* File input using file descriptor. */
    uint8_t *buf = malloc(sizeof(*buf) * BUFFER_SIZE);
    ALDER_RETURN_ERROR_IF_NULL(buf, ALDER_STATUS_ERROR);
    
    /* File input using file descriptor. */
    int fildes = -1;
    if (infile->qty == 0) {
        fildes = 0;
    } else {
        fildes = open(bdata(infile->entry[0]), O_RDONLY);
        s = fcntl(fildes, F_RDAHEAD);
        if (s == -1) {
            alder_loge(ALDER_ERR_FILE, "failed to read file %s.",
                       bdata(infile->entry[0]));
            XFREE(buf);
            return ALDER_STATUS_ERROR;
        }
    }
    /* File output using file pointer. */
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
    
    /* Open the table file. */
    int fildes_tab;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs = NULL;
    s = alder_hashtable_mcas_find_open(tabfile, &fildes_tab, &K, &nh, &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                tabfile, strerror(errno));
        XFREE(buf);
        return ALDER_STATUS_ERROR;
    }
    assert(n_nhs != NULL);
//    int flag_nh64 = 0;
//    if (nh <= UINT32_MAX) {
//        flag_nh64 = 0;
//    } else {
//        flag_nh64 = 1;
//    }
    
    /* Parsing input stream. */
    int lines = 0;
    size_t bytes_read;
    size_t len_remaining = 0;
    uint8_t *sbuf = buf;
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
            //            char c = (char)*sbuf;
            //            fprintf(fpout, "[%zu] %s\n", ++n_kmer, sbuf);
            s = alder_hashtable_mcas_find(fildes_tab, (char*)sbuf, K, nh, ni, np, tnh, n_nhs);
            if (s == ALDER_YES) {
                // Found!
            } else {
                status = ALDER_STATUS_ERROR;
                fprintf(stderr, "Fatal: %s is not found in the table.", sbuf);
                goto cleanup;
                // Not found!
                // Fatal!
                fprintf(stderr, "Fatal: %s is not found in the table.", sbuf);
                assert(0);
                abort();
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
    close(fildes_tab);
    XFREE(n_nhs);
    if (infile->qty > 0) {
        close(fildes);
    }
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return status;
}

