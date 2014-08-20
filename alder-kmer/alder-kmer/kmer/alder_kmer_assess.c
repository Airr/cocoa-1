/**
 * This file, alder_kmer_assess.c, is part of alder-kmer.
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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_kmer_assess.h"
#include "alder_hashtable_mcas.h"

static int
read_table2_header(const char *fn, int *K_p,
                   uint64_t *nh_p, uint64_t *sv_p, uint64_t *si_p,
                   uint64_t *th_p)
                   
{
    ssize_t readLen = 0;
    int fildes = open(fn, O_RDWR); // O_RDONLY
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    readLen = read(fildes, &(*K_p), sizeof(*K_p));
    if (readLen != sizeof(*K_p) || *K_p <= 0) {
        fprintf(stderr, "Error - failed to read K in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("*** Table of alder-kmer ***");
    alder_log("%*s %d", width, "K:", *K_p);
    
    readLen = read(fildes, &(*nh_p), sizeof(*nh_p));
    if (readLen != sizeof(*nh_p) || *nh_p == 0) {
        fprintf(stderr, "Error - failed to read Nh in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "nh:", *nh_p);
    
    readLen = read(fildes, &(*sv_p), sizeof(*sv_p));
    if (readLen != sizeof(*sv_p) || *sv_p == 0) {
        fprintf(stderr, "Error - failed to read Ni in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "sv:", *sv_p);
    
    readLen = read(fildes, &(*si_p), sizeof(*si_p));
    if (readLen != sizeof(*si_p) || *si_p == 0) {
        fprintf(stderr, "Error - failed to read Np in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "si:", *si_p);
    
    readLen = read(fildes, &(*th_p), sizeof(*th_p));
    if (readLen != sizeof(*th_p) || *th_p == 0) {
        fprintf(stderr, "Error - failed to read Np in file %s\n", fn);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    alder_log("%*s %llu", width, "th:", *th_p);
    
    close(fildes);
    return ALDER_STATUS_SUCCESS;
}


/**
 *  This function takes topkmer's table to compute the errors of topkmer.
 *
 *  @param fpout  out file
 *  @param infile infile
 *  @param i_ni   index of ni
 *  @param i_np   index of np
 *  @param table  table file
 */
static int
alder_kmer_assess_topkmer_compare(double *error_rate, uint64_t *error_n,
                                  FILE *fpout, struct bstrList *infile,
                                  int K, uint64_t ni, uint64_t np,
                                  uint64_t i_ni, uint64_t i_np,
                                  alder_hashtable_mcas_t *table_dsk,
                                  int min_count)
{
    char *fn = bdata(infile->entry[0]);
    uint64_t nh;
    uint64_t si;
    uint64_t sv;
    uint64_t th;
    // K - kmer size
    // nh - table size
    // sv - number of bytes in a value
    // si - number of bytes in an index
    // th - actual number of elements in a table
    int s = read_table2_header("outfile.tbh", &K, &nh, &sv, &si, &th);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file outfile.tbh: %s\n",
                strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint64_t value;
    size_t pos;
    int size_key = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE) * sizeof(uint64_t);
    size_t h_size = size_key + sv + si;
    size_t bit_value = 8 * (8 - sv);
    size_t bit_index = 8 * (8 - si);
    
    size_t size_element = size_key + sv + si;
    uint8_t *entry_topkmer = NULL;
    size_t size_item = sizeof(*entry_topkmer) * size_element * 2;
    entry_topkmer = malloc(size_item);
    memset(entry_topkmer, 0, size_item);
    
    FILE *fp = fopen(fn, "rb");
    
    for (uint64_t i_s = 0; i_s < th; i_s++) {
        // Read each element: key and value.
//        alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
        
        size_t nitems = fread(entry_topkmer, size_element, 1, fp);
        if (nitems != 1) {
            alder_loge(ALDER_ERR_FILE, "failed to read a table %s", fn);
            abort();
        }
        size_t middle = 0 * h_size;
        for (int i = 0; i < key->s; i++) {
            key->n[i] = to_uint64(entry_topkmer, middle + i * 8);
        }
        pos = to_size(entry_topkmer, middle + size_key + sv);
        pos <<= bit_index;
        pos >>= bit_index;
        value = to_uint64(entry_topkmer, middle + size_key);
        value <<= bit_value;
        value >>= bit_value;
        
        // Search the dsk table.
        uint64_t h = alder_encode_number_hash(key);
        uint64_t x_ni = h % ni;
        uint64_t x_np = h / ni % np;
        if (i_ni == x_ni && i_np == x_np) {
            uint64_t pos_dsk = 0;
            s = alder_hashtable_mcas_search(table_dsk, key->n, &pos_dsk);
            if (s == ALDER_STATUS_SUCCESS) {
                if (table_dsk->value[pos_dsk] >= (uint16_t)min_count) {
                    *error_n = *error_n + 1;
                    double e_rate = (double)(table_dsk->value[pos_dsk] - (uint16_t)value)/ (double)table_dsk->value[pos_dsk];
                    *error_rate = *error_rate + e_rate;
                }
            } else {
                // The key is not found.
            }
        }
        
        
    }
    alder_encode_number_destroy(key);
    XFCLOSE(fp);
    
//    s = alder_hashtable_mcas_large_close(fildes);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to read the end of a table file %s\n",
                fn);
        return ALDER_STATUS_ERROR;
    }
    
    XFREE(entry_topkmer);
    
    return s;
}

/**
 *  This function computes the error rates of topkmer counting.
 *
 *  @param upper_count   upper count
 *  @param infile        infile or topkmer table file
 *  @param tabfile       table file
 *  @param outfile_given outfile given or not
 *  @param outdir        out directory
 *  @param outfile       out file name
 *
 *  @return success or fail
 */
static int
alder_kmer_assess_topkmer(int min_count,
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
    
    alder_hashtable_mcas2_t *table_topkmer =
    alder_hashtable_mcas2_read_table(bdata(infile->entry[0]));
    
    int fildes;
    int K;
    uint64_t nh;
    uint64_t ni;
    uint64_t np;
    uint64_t tnh;
    uint64_t *n_nhs;
    int s = open_table_header(tabfile, &fildes, &K, &nh, &ni, &np, &tnh, &n_nhs);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to open a table file %s: %s\n",
                tabfile, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    uint16_t value;
    size_t pos;
    int flag_nh64 = 0;
    if (nh <= UINT32_MAX) {
        flag_nh64 = 0;
    } else {
        flag_nh64 = 1;
    }
#if defined(PRINT_TABLE_ID)
    size_t prev_pos = INT32_MAX;
    int i_table = 0;
#endif
    
    double error_rate = 0;
    uint64_t error_n = 0;
    for (uint64_t i_s = 0; i_s < tnh; i_s++) {
        // Read each element: key and value.
        alder_hashtable_mcas_nextFromFile(fildes, key, &value, &pos, flag_nh64);
#if defined(PRINT_TABLE_ID)
        if (prev_pos > pos) {
            fprintf(stdout, "Table: %d\n", i_table++);
        }
        prev_pos = pos;
#endif
        // Print key and value.
//        alder_encode_number_print_in_DNA(stdout, key);
//        fprintf(stdout, "\t");
//        alder_encode_number_print_in_revDNA(stdout, key);
//        fprintf(stdout, "\t%d", value);
//        fprintf(stdout, "\t%zu\n", pos);
        
        // Search the topkmer table.
        uint64_t pos_topkmer = 0;
        s = alder_hashtable_mcas2_search(table_topkmer, key->n, &pos_topkmer);
        
        if (s == ALDER_STATUS_SUCCESS) {

            uint64_t value_topkmer = table_topkmer->key[pos_topkmer + 1];
            if (value >= (uint16_t)min_count) {
                error_n = error_n + 1;
                double e_rate = (double)((uint64_t)value - value_topkmer)/ (double)value;
                error_rate = error_rate + e_rate;
            }
        } else {
            // The key is not found.
            if (value >= (uint16_t)min_count) {
                error_n = error_n + 1;
                error_rate = error_rate + 1;
            }
        }
    }
    error_rate /= (double)error_n;
    if (error_n > 0) {
        fprintf(fpout, "error [%d]: %.3lf (%llu)\n", min_count, error_rate, error_n);
    } else {
        fprintf(fpout, "error [%d]: NA (%llu)\n", min_count, error_n);
    }
    
    alder_hashtable_mcas2_destroy(table_topkmer);
    alder_encode_number_destroy(key);
    s = alder_hashtable_mcas_large_close(fildes);
    
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "Error - failed to read the end of a table file %s\n",
                tabfile);
        XFREE(n_nhs);
        close(fildes);
        return ALDER_STATUS_ERROR;
    }
    
cleanup:
    XFREE(n_nhs);
    close(fildes);
    if (outfile_given) {
        XFCLOSE(fpout);
    }
    return s;
}

/**
 *  This function compares the approximately counted table with an exactly
 *  counted one.
 *
 *  @param tabfile exactly counted table
 *  @param infile  approximately counted table file
 *  @param outdir  output directory
 *  @param outfile outfile name
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_assess(int upper_count,
                      const char *tabfile,
                      struct bstrList *infile,
                      int outfile_given,
                      const char *outdir,
                      const char *outfile)
{
    
    double *error_kmer = malloc(sizeof(*error_kmer) * upper_count);
    if (error_kmer == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(error_kmer, 0, sizeof(*error_kmer) * upper_count);
    uint64_t *n_kmer = malloc(sizeof(*n_kmer) * upper_count);
    if (n_kmer == NULL) {
        XFREE(error_kmer);
        return ALDER_STATUS_ERROR;
    }
    
    upper_count = 10;
    for (int i = 1; i <= upper_count; i++) {
        alder_kmer_assess_topkmer(i,
                                  infile,
                                  tabfile,
                                  outfile_given,
                                  outdir,
                                  outfile);
    }
    
//    alder_hashtable_mcas12_assess(error_kmer, n_kmer, upper_count,
//                                  tabfile, bdata(infile->entry[0]), 0);
    
//    fprintf(stdout, "*** Assessment of Approximation ***\n");
//    for (long i = 0; i < upper_count; i++) {
//        fprintf(stdout, "[%ld] %.f (%llu)\n", i+1, error_kmer[i], n_kmer[i]);
//    }
    
    XFREE(error_kmer);
    XFREE(n_kmer);
    return ALDER_STATUS_SUCCESS;
}

