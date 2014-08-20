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

/**
 *  This function takes kmc files in infile to find kmers and decrement the 
 *  count values in the table. Kmers in the table buffer are all from 
 *  a particular i_ni x i_np combination.
 *
 *  @param fpout  out file
 *  @param infile infile
 *  @param i_ni   index of ni
 *  @param i_np   index of np
 *  @param table  table file
 */
static int
alder_kmer_match_kmc_compare(FILE *fpout, struct bstrList *infile,
                             int K, uint64_t ni, uint64_t np,
                             uint64_t i_ni, uint64_t i_np,
                             alder_hashtable_mcas_t *table)
{
    int s = ALDER_STATUS_SUCCESS;
//    int n_uint64_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    
    alder_encode_number_t *kmer = alder_encode_number_create_for_kmer(K);
    
    for (int i_infile = 0; i_infile < infile->qty; i_infile++) {
        FILE *fpin = fopen(bdata(infile->entry[i_infile]), "rb");
        /* Read a pair of key and value. */
        
        while (1) {
            size_t n1 = fread(kmer->n, sizeof(*kmer->n), kmer->s, fpin);
            uint32_t kmer_count = 0;
            size_t n2 = fread(&kmer_count, sizeof(kmer_count), 1, fpin);
            if (n1 != kmer->s && n2 != 1) {
                if (feof(fpin)) {
                    break;
                }
                alder_loge(ALDER_ERR, "failed to read a kmer from file %s",
                           bdata(infile->entry[i_infile]));
                XFCLOSE(fpin);
                alder_encode_number_destroy(kmer);
                return ALDER_STATUS_ERROR;
            }
            
            uint64_t h = alder_encode_number_hash(kmer);
            uint64_t x_ni = h % ni;
            uint64_t x_np = h / ni % np;
            if (i_ni == x_ni && i_np == x_np) {
                uint64_t pos = 0;
                s = alder_hashtable_mcas_search(table, kmer->n, &pos);
                if (s == ALDER_STATUS_SUCCESS) {
                    table->value[pos] -= (uint16_t)kmer_count;
                } else {
                    // The key is not found.
                }
            }
        }
        // key size  : K
        // value size: 32-bit
        // hash value of the key
        // compute x_ni and x_np
        // Use only i_ni == x_ni and i_np == x_np
        XFCLOSE(fpin);
    }
    /* Print keys with positive count values. */
    alder_hashtable_mcas_print(fpout, table, 0);
    
    alder_encode_number_destroy(kmer);
    return s;
}

/**
 *  This function checks whether kmc files contains all of the kmers in a
 *  given table file.
 *  1. Read a partition table in each iteration.
 *  2. Keep decrementing kmer in the table using kmc files.
 *  3. Iterate over all of the partition tables in the table.
 *  4. Print kmers in a partition table if there are kmers with positive value.
 *  5. No output is given if kmc files contains all of the kmers in a table.
 *
 *  @param progress_flag progress
 *  @param infile        infile
 *  @param tabfile       tabfile
 *  @param format_infile infile format: 0/2 (fa), 1/3 (fq), 4 (seq), 5 (kmc)
 *  @param outfile_given outfile is given or not
 *  @param outdir        output directory
 *  @param outfile       outfile name
 *
 *  @return SUCCESS or FAIL
 */
static int
alder_kmer_match_kmc(int progress_flag,
                     struct bstrList *infile,
                     const char *tabfile,
                     int outfile_given,
                     const char *outdir,
                     const char *outfile)
{
    if (infile == NULL) {
        alder_loge(ALDER_ERR_FILE,
                   "command match with kmc cannot handle STDIN.");
        return ALDER_STATUS_ERROR;
    }
    int s = ALDER_STATUS_SUCCESS;
    
    /* File output using file pointer. */
    FILE *fpout = NULL;
    bstring bfn = bformat("%s/%s.match", outdir, outfile);
    if (outfile_given) {
        fpout = fopen(bdata(bfn), "w");
        if (fpout == NULL) {
            return ALDER_STATUS_ERROR;
        }
    } else {
        fpout = stdout;
    }
    
    /* Read the head of the dsk table file. */
    long fpos;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs = NULL;
    s = alder_hashtable_mcas_read_headerFILE(tabfile, &fpos, &K, &nh,
                                             &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        alder_loge(ALDER_ERR_FILE, "failed to open a table file %s: %s\n",
                   tabfile, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    assert(n_nhs != NULL);
    
    /* Find the maximum value of slots in all of the partition table. */
    uint64_t n_max = 0;
    for (uint64_t i_ni = 0; i_ni < ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < np; i_np++) {
            /* Read a partition table. */
            uint64_t ns = n_nhs[i_np + i_ni * np];
            if (n_max < ns) {
                n_max = ns;
            }
        }
    }
    
    /* Allocate the memory for a partition table. */
//    nc = UINT16_MAX;
//    size_t n_byte_entry = alder_hashtable_mcas_table_entry_sizeof(K, nc, nh);
    
    /* Iterate over all partition tables. */
    alder_hashtable_mcas_t *t = NULL;
    for (uint64_t i_ni = 0; i_ni < ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < np; i_np++) {
            t = alder_hashtable_mcas_read_table(tabfile, i_ni, i_np);
            if (t == NULL) {
                alder_loge(ALDER_ERR_HASH,
                           "failed to read a table (ni:%llu, np:%llu)"
                           " in file %s", i_ni, i_np, tabfile);
                s = ALDER_STATUS_ERROR;
                goto cleanup;
            }
            /* Read all of the kmers in kmc file. */
            s = alder_kmer_match_kmc_compare(fpout, infile, K, ni, np,
                                             i_ni, i_np, t);
            alder_hashtable_mcas_destroy(t);
            if (s != ALDER_STATUS_SUCCESS) {
                goto cleanup;
            }
        }
    }
    
cleanup:
    XFREE(n_nhs);
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return s;
}

/**
 *  This function checks whether the given table contains all of the kmers
 *  in sequence files.
 *
 *  @param progress_flag progress
 *  @param infile        infile
 *  @param tabfile       tabfile
 *  @param format_infile infile format: 0/2 (fa), 1/3 (fq), 4 (seq), 5 (kmc)
 *  @param outfile_given outfile is given or not
 *  @param outdir        output directory
 *  @param outfile       outfile name
 *
 *  @return SUCCESS or FAIL
 */
static int
alder_kmer_match_seq(int progress_flag,
                     struct bstrList *infile,
                     const char *tabfile,
                     int format_infile,
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


/**
 *  This function does:
 *  1. checks whether an exactly counted table contains all of the kmers in
 *     sequence files.
 *  2. uses an exactly counted table to check whether the kmc file or files 
 *     contains all of the kmers in the table.
 *
 *  @param progress_flag progress
 *  @param infile        infile
 *  @param tabfile       tabfile
 *  @param format_infile infile format: 0/2 (fa), 1/3 (fq), 4 (seq), 5 (kmc)
 *  @param outfile_given outfile is given or not
 *  @param outdir        output directory
 *  @param outfile       outfile name
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_match(int progress_flag,
                 struct bstrList *infile,
                 const char *tabfile,
                 int format_infile,
                 int outfile_given,
                 const char *outdir,
                 const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    
    if (format_infile == 5) {
        s = alder_kmer_match_kmc(progress_flag,
                                 infile,
                                 tabfile,
                                 outfile_given,
                                 outdir,
                                 outfile);
    } else {
        s = alder_kmer_match_seq(progress_flag,
                                 infile,
                                 tabfile,
                                 format_infile,
                                 outfile_given,
                                 outdir,
                                 outfile);
    }
    return s;
}

