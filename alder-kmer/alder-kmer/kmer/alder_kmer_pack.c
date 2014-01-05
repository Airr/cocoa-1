/**
 * This file, alder_kmer_pack.c, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
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
#include "alder_logger.h"
#include "alder_file_availablememory.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_pack.h"

/* This function makes a hash table to be packed.
 */
int
alder_kmer_pack(const char *fn,
                const char *outdir,
                const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    
    size_t volume = 0;
    s = alder_hashtable_mcas_count(fn, &volume);
    
    int K = 0;
    int S = 0;
    uint64_t ni = 0;
    uint64_t np = 0;
    s = alder_hashtable_mcas_header(fn, &K, &S, &ni, &np);
                            
    // Create a hash table.
    volume = (size_t)((double)volume / 0.7);
    
    // Compute the number of partitions of 8MB.
    /* Compute np = v*(2k+16)/(0.7*ni*M). */
    size_t M = 1 << 23;
    int n_uint64 = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t hash_element_size = (n_uint64 * sizeof(uint64_t) +
                                (sizeof(uint16_t)));
    size_t X_np = (((volume * hash_element_size) /
                    M) + 1);
    
    size_t X_nh = M / hash_element_size;
    
    // Create a hash table for this partition.
    alder_hashtable_mcas_t *ht = alder_hashtable_mcas_create(K, X_nh);
    if (ht == NULL) {
        fprintf(stderr, "failed to pack the hash table in file %s.\n",
                fn);
        return ALDER_STATUS_ERROR;
    }
    ///////////////////////////////////////////////////////////////////////////
    // Print the header of x hash table.
    bstring bht = bformat("%s/%s.tb2", outdir, outfile);
    if (bht == NULL) return ALDER_STATUS_ERROR;
    alder_log5("creating a hash count table (%s)", bdata(bht));
    FILE *fpout = fopen(bdata(bht), "wb");
    if (fpout == NULL) {
        bdestroy(bht);
        return ALDER_STATUS_ERROR;
    }
    // This is the place to write the header part of the hash count table.
    alder_log5("writing the header of the hash count table ...");
    s = alder_hashtable_mcas_print_header_with_FILE(fpout,
                                                    K, (int)X_nh,
                                                    1, X_np);
    
    alder_hashtable_mcas_t *mht = alder_hashtable_mcas_create(K, S);
    for (size_t x_np = 0; x_np < X_np; x_np++) {
        
        alder_hashtable_mcas_large_convert(ht, mht, fn, x_np, X_np);
        // Print the x hash table.
        alder_hashtable_mcas_print_with_FILE(ht, fpout);
    }
    XFCLOSE(fpout);
    alder_hashtable_mcas_destroy(mht);
    alder_hashtable_mcas_destroy(ht);
    
    return s;
}


/* This function converts a kmer count data set to a hash table with a single
 * iteration.
 */
int
alder_kmer_pack_kmercount(const char *fn,
                          const char *outdir,
                          const char *outfile)
{
    int s = ALDER_STATUS_SUCCESS;
    
    size_t volume = 0;
    
    int K = 0;
    int S = 0;
    uint64_t ni = 0; // N/A
    uint64_t np = 0; // N/A
    s = alder_hashtable_mcas_header(fn, &K, &S, &ni, &np);
    
    // Create a hash table.
    volume = (size_t)((double)S / 0.7);
    
    // Find the available memory.
    int64_t free_memory = 0;
    int64_t used_memory = 0;
    alder_file_availablememory2(&free_memory, &used_memory);
    size_t M = (size_t)free_memory;
    // Compute the number of partitions of 8MB.
    /* Compute np = v*(2k+16)/(0.7*ni*M). */
    int n_uint64 = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    size_t hash_element_size = (n_uint64 * sizeof(uint64_t) +
                                (sizeof(uint16_t)));
    size_t X_np = (((volume * hash_element_size) /
                    M) + 1);
    
//    size_t X_nh = M / hash_element_size;
    size_t X_nh = volume;
    
    // Create a hash table for this partition.
    alder_hashtable_mcas_t *ht = alder_hashtable_mcas_create(K, X_nh);
    if (ht == NULL) {
        fprintf(stderr, "failed to pack the hash table in file %s.\n",
                fn);
        return ALDER_STATUS_ERROR;
    }
    ///////////////////////////////////////////////////////////////////////////
    // Print the header of x hash table.
    bstring bht = bformat("%s/%s.tb2", outdir, outfile);
    if (bht == NULL) return ALDER_STATUS_ERROR;
    alder_log5("creating a hash count table (%s)", bdata(bht));
    FILE *fpout = fopen(bdata(bht), "wb");
    if (fpout == NULL) {
        bdestroy(bht);
        return ALDER_STATUS_ERROR;
    }
    // This is the place to write the header part of the hash count table.
    alder_log5("writing the header of the hash count table ...");
    s = alder_hashtable_mcas_print_header_with_FILE(fpout,
                                                    K, (int)X_nh,
                                                    1, X_np);
    // Create an array of hash tables.
    // Read the table and put them in the array of hash tables.
    // If the array of the hash tables is too big to fit to the available
    // memeory, then repeat over the smaller number of partitions.
    for (size_t x_np = 0; x_np < X_np; x_np++) {
        alder_hashtable_mcas_convertCountToHashTable(ht, fn, x_np, X_np);
        alder_hashtable_mcas_print_with_FILE(ht, fpout);
    }
    // Print the x hash table.
    
    XFCLOSE(fpout);
    
    alder_hashtable_mcas_destroy(ht);
    
    return s;
}

