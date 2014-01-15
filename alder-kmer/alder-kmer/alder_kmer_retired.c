/**
 * This file, alder_kmer_retired.c, is part of alder-kmer.
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

#pragma mark pack


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
    uint64_t S = 0;
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
    uint64_t S = 0;
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

#pragma mark hash table

int
alder_hashtable_mcas_print_with_FILE(alder_hashtable_mcas_t *o, FILE *fp)
{
    size_t s;
    s = fwrite(o->empty_key, sizeof(*o->empty_key),  o->stride, fp);
    if (s != o->stride) {
        return 1;
    }
    s = fwrite(o->key, sizeof(*o->key), o->size * o->stride, fp);
    if (s != o->size * o->stride) {
        return 1;
    }
    s = fwrite(o->value, sizeof(*o->value), o->size, fp);
    if (s != o->size) {
        return 1;
    }
    return 0;
}

int
alder_hashtable_mcas_load_RETIRED(const char *fn, int isSummary)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    
    int kmer_size;
    uint64_t n_nh;
    uint64_t n_ni;
    uint64_t n_np;
    read(fildes, &kmer_size, sizeof(kmer_size));
    read(fildes, &n_nh, sizeof(n_nh));
    read(fildes, &n_ni, sizeof(n_ni));
    read(fildes, &n_np, sizeof(n_np));
    
    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    uint64_t *empty_key = malloc(sizeof(*empty_key) * stride);
    memset(empty_key,0,sizeof(*empty_key) * stride);
    size_t bufsize = 1 << 23;
    
    //
    
    alder_hashtable_mcas_t *o = alder_hashtable_mcas_create(kmer_size,
                                                            n_nh);
    size_t n_empty_slot = 0;
    for (uint64_t i_ni = 0; i_ni < n_ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < n_np; i_np++) {
            // Read a hash table.
            ssize_t r = read(fildes, empty_key, sizeof(*empty_key) * stride);
            if (r != sizeof(*empty_key) * stride) {
                if (r == -1) {
                    fprintf(stderr, "Error - %s\n", strerror(errno));
                }
            }
            
            r = read_large_bytes_kmer_file(o, bufsize, fildes);
            r = read_large_bytes_count_file(o, bufsize, fildes);
            
            // Print or return it.
            n_empty_slot += alder_hashtable_mcas_print(o, stdout);
        }
    }
//    ssize_t r = read(fildes, empty_key, 1);
//    assert(r == 0);
    
    XFREE(empty_key);
    alder_hashtable_mcas_destroy(o);
    close(fildes);
    
    if (isSummary) {
        size_t n_total_slot = (size_t)n_nh * n_ni * n_np;
        fprintf(stdout, "Kmer size           : %d\n", kmer_size);
        fprintf(stdout, "Hash table size     : %llu\n", n_nh);
        fprintf(stdout, "Number of iteration : %llu\n", n_ni);
        fprintf(stdout, "Number of partition : %llu\n", n_np);
        fprintf(stdout, "Empty/Total slots = %zu/%zu (%0.1f%%)\n",
                n_empty_slot, n_total_slot, (double)n_empty_slot * 100/(double)n_total_slot);
    }
    return 0;
}


/* This function prints the content of a hash table.
 * returns
 * number of empty slots in the hash table
 */
size_t alder_hashtable_mcas_print(alder_hashtable_mcas_t *o, FILE *fp)
{
    size_t v = 0;
    alder_encode_number_t *m = alder_encode_number_create_for_kmer_view(o->k);
    for (int i = 0; i < o->size; i++) {
        // Print only key that is not empty.
        if (memcmp(o->key + i * o->stride, o->empty_key, sizeof(*o->key)*o->stride)) {
            // Print the key and its value.
            m->n = o->key + i * o->stride;
            alder_encode_number_print_in_DNA(fp, m);
            fprintf(fp, "\t");
            alder_encode_number_print_in_revDNA(fp, m);
            fprintf(fp, "\t");
            fprintf(fp, "%u\n", o->value[i]);
        } else {
            v++;
        }
    }
    alder_encode_number_destroy_view(m);
    return v;
}




/* 
 */
static ssize_t
read_large_bytes_kmer_file(alder_hashtable_mcas_t *o, size_t bufsize, int fildes)
{
    // Kmer sequences
    size_t keysize = (sizeof(*o->key) * o->stride);
    bufsize = bufsize / keysize * keysize;
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = keysize * o->size;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    
    r = read(fildes, o->key, sbyte);
    
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, o->key + cbyte/sizeof(*o->key), sbyte);
        
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
    return r;
}

static ssize_t
read_large_bytes_count_file(alder_hashtable_mcas_t *o, size_t bufsize, int fildes)
{
    // Count sequences
    size_t keysize = sizeof(*o->value);
    bufsize = bufsize / keysize * keysize;
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = sizeof(*o->value) * o->size;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    
    r = read(fildes, o->value, sbyte);
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, o->value + cbyte/sizeof(*o->value), sbyte);
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
    return r;
}


/* This function reads the kmer part in a hash table.
 */
static int
count_read_kmer(size_t *v, int K, size_t hashtablesize, int fildes)
{
    *v = 0;
    size_t v_c = 0;
    
    // Kmer sequences
    int stride = ALDER_BYTESIZE_KMER(K,ALDER_NUMKMER_8BYTE);
    
    // Empty key
    uint64_t *empty_key = malloc(sizeof(empty_key) * stride);
    if (empty_key == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(empty_key, 0x44, sizeof(*empty_key) * stride);
    
    // Prepare a buffer.
    uint64_t *key = NULL;
    size_t bufsize = 1 << 23;
    size_t keysize = sizeof(*key) * stride;
    bufsize = bufsize / keysize * keysize;
    key = malloc(bufsize);
    memset(key, 0, bufsize);
    assert(bufsize % keysize == 0);
    
    // Start reading.
    ssize_t totalRead = 0;
    size_t nbyte = hashtablesize * keysize;
    size_t rbyte = 0;
    
//    size_t cbyte = 0;
//    size_t rbyte = bufsize;
//    size_t sbyte = 0;
//    sbyte = rbyte < nbyte ? bufsize : nbyte;
//    r = read(fildes, key, sbyte);
//    if (r != sbyte && r == -1) {
//        fprintf(stderr, "Error - %s\n", strerror(errno));
//    }
//    assert(r % keysize == 0);
//    size_t count_key = r / keysize;
//    for (int i = 0; i < count_key; i++) {
//        // Print only key that is not empty.
//        if (memcmp(key + i * stride, empty_key, sizeof(*key)*stride)) {
//            // Print the key and its value.
//            v_c++;
//        }
//    }
//    totalRead += r;
    
    while (rbyte < nbyte) {
        size_t cbyte = rbyte;
        rbyte += bufsize;
        size_t sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        ssize_t r = read(fildes, key, sbyte);
        if (r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
            break;
        }
        assert(r >= 0);
        assert(r % keysize == 0);
        size_t count_key = r / keysize;
        for (int i = 0; i < count_key; i++) {
            // Print only key that is not empty.
            if (memcmp(key + i * stride, empty_key, sizeof(*key)*stride)) {
                // Print the key and its value.
                v_c++;
            }
        }
        
        // See the key.
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
   
    XFREE(empty_key);
    XFREE(key);
    *v = v_c;
    return ALDER_STATUS_SUCCESS;
}

/* This function reads the value (count) parts in a hash table.
 */
static int
count_read_count(size_t *v, int K, size_t hashtablesize, int fildes)
{
    // Kmer sequences
    int stride = 1;
    
    // Prepare a buffer.
    uint16_t *value = NULL;
    size_t bufsize = 1 << 23;
    size_t valuesize = sizeof(*value) * stride;
    bufsize = bufsize / valuesize * valuesize;
    value = malloc(bufsize);
    memset(value, 0, bufsize);
    assert(bufsize % valuesize == 0);
    
    // Start reading.
    ssize_t totalRead = 0;
    ssize_t r;
    size_t nbyte = hashtablesize * valuesize;
    
    size_t cbyte = 0;
    size_t rbyte = bufsize;
    size_t sbyte = 0;
    sbyte = rbyte < nbyte ? bufsize : nbyte;
    r = read(fildes, value, sbyte);
    if (r != sbyte && r == -1) {
        fprintf(stderr, "Error - %s\n", strerror(errno));
    }
    totalRead += r;
    
    while (rbyte < nbyte) {
        cbyte = rbyte;
        rbyte += bufsize;
        sbyte = rbyte < nbyte ? bufsize : nbyte - cbyte;
        
        r = read(fildes, value, sbyte);
        if (r != sbyte && r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
            break;
        }
        assert(r % valuesize == 0);
//        size_t count_value = r / valuesize;
        
//        for (int i = 0; i < count_value; i++) {
//            // Print value that is not empty.
//        }
        
        // See the key.
        totalRead += r;
    }
    assert(totalRead == (ssize_t)nbyte);
   
    XFREE(value);
//    *v = v_c;
    return ALDER_STATUS_SUCCESS;
}

/* This function counts non-empty slots in a hash table.
 *
 */
int
alder_hashtable_mcas_count(const char *fn,
                           size_t *volumn)
{
    int fildes = open(fn, O_RDONLY);
    if (fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    int kmer_size;
    int hashtable_size;
    uint64_t n_ni;
    uint64_t n_np;
    read(fildes, &kmer_size, sizeof(kmer_size));
    read(fildes, &hashtable_size, sizeof(hashtable_size));
    read(fildes, &n_ni, sizeof(n_ni));
    read(fildes, &n_np, sizeof(n_np));
    int stride = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    uint64_t *empty_key = malloc(sizeof(*empty_key) * stride);
    memset(empty_key,0,sizeof(*empty_key) * stride);
//    size_t bufsize = 1 << 23;
    
    // Count non-empty slots.
    size_t n_slot = 0;
    for (uint64_t i_ni = 0; i_ni < n_ni; i_ni++) {
        for (uint64_t i_np = 0; i_np < n_np; i_np++) {
            // Read a hash table.
            ssize_t r = read(fildes, empty_key, sizeof(*empty_key) * stride);
            if (r != sizeof(*empty_key) * stride) {
                if (r == -1) {
                    fprintf(stderr, "Error - %s\n", strerror(errno));
                }
            }
            size_t count_key = 0;
            r = count_read_kmer(&count_key, kmer_size, hashtable_size, fildes);
            r = count_read_count(NULL, kmer_size, hashtable_size, fildes);
            n_slot += count_key;
        }
    }
    
    XFREE(empty_key);
    close(fildes);
    
    size_t n_total_slot = (size_t)hashtable_size * n_ni * n_np;
    fprintf(stdout, "Kmer size           : %d\n", kmer_size);
    fprintf(stdout, "Hash table size     : %d\n", hashtable_size);
    fprintf(stdout, "Number of iteration : %llu\n", n_ni);
    fprintf(stdout, "Number of partition : %llu\n", n_np);
    fprintf(stdout, "Slots/Total slots = %zu/%zu (%0.1f%%)\n",
            n_slot, n_total_slot, (double)n_slot * 100/(double)n_total_slot);
    
    *volumn = n_slot;
    return ALDER_STATUS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Functions for command pack in alder-kmer
///////////////////////////////////////////////////////////////////////////////

int
alder_hashtable_mcas_large_convert(alder_hashtable_mcas_t *dst,
                                   alder_hashtable_mcas_t *src,
                                   const char *fn,
                                   uint64_t xi_np,
                                   uint64_t xn_np)
{
    
    // Open the input hash table.
    int K = 0;
    uint64_t S = 0;
    uint64_t ni = 0;
    uint64_t np = 0;
    int fildes;
    int s = alder_hashtable_mcas_large_open(fn, &fildes, &K, &S, &ni, &np);
    if (s != ALDER_STATUS_SUCCESS) {
        return s;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer_view(K);
    
    // Reset the x hash table.
    alder_hashtable_mcas_reset(dst);
    
    for (size_t i_ni = 0; i_ni < ni; i_ni++) {
        for (size_t i_np = 0; i_np < np; i_np++) {
            // Reset the main hash table.
            alder_hashtable_mcas_reset(src);
            // Extract each hash table over ni x np.
            alder_hashtable_mcas_large_next(src, fildes);
            
            // Read each key and find its place in the current table,
            // if the key belongs to this partition.
            // Find each element and put it if it is in the current partition.
            uint16_t value;
            alder_hashtable_mcas_next(src, key, &value);
            while (value != 0) {
                uint64_t w_np = alder_encode_number_hash(key) % xn_np;
                if (w_np == xi_np) {
                    alder_hashtable_mcas_push(dst, key, value);
                }
                alder_hashtable_mcas_next(src, key, &value);
            }
        }
    }
    // Close the input hash table.
    alder_hashtable_mcas_large_close(fildes);
    alder_encode_number_destroy_view(key);

    return s;
}

/* This function opens a hash table file and reads the header.
 * It does NOT close the file pointer.
 */
int
alder_hashtable_mcas_large_open(const char *fn, int *fildes,
                                int *K, uint64_t *S, uint64_t *Ni, uint64_t *Np)
{
    *fildes = open(fn, O_RDONLY);
    if (*fildes == -1) {
        fprintf(stderr, "Error - openning file %s : %s\n", fn, strerror(errno));
        return ALDER_STATUS_ERROR;
    }
    read(*fildes, K, sizeof(*K));
    read(*fildes, S, sizeof(*S));
    read(*fildes, Ni, sizeof(*Ni));
    read(*fildes, Np, sizeof(*Np));
    return ALDER_STATUS_SUCCESS;
}



int
alder_hashtable_mcas_large_next(alder_hashtable_mcas_t *o, int fildes)
{
    size_t bufsize = 1 << 23;
    // Read a hash table.
    ssize_t r = read(fildes, o->empty_key, sizeof(*o->empty_key) * o->stride);
    if (r != sizeof(*o->empty_key) * o->stride) {
        if (r == -1) {
            fprintf(stderr, "Error - %s\n", strerror(errno));
        }
        return ALDER_STATUS_ERROR;
    }
    
    r = read_large_bytes_kmer_file(o, bufsize, fildes);
    r = read_large_bytes_count_file(o, bufsize, fildes);
    
    o->index = 0;
    return ALDER_STATUS_SUCCESS;
}

int
alder_hashtable_mcas_next(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t *value)
{
    // Start at the current index to check if the key is non-empty, and to
    // return it.
    uint64_t x1 = 1;
    *value = 0;
    while (o->index < o->size) {
        key->n = o->key + o->index * o->stride;
        if((key->n[0] >> 62) & x1) {
            // no code: this key is empty.
        } else {
            *value = o->value[o->index];
            o->index++;
            break;
        }
        o->index++;
    }
    return ALDER_STATUS_SUCCESS;
}



int
alder_hashtable_mcas_push(alder_hashtable_mcas_t *o,
                          alder_encode_number_t *key,
                          uint16_t value)
{
    // Insert the key and value in the hash table.
    size_t tries = 1;
    uint64_t x = alder_hash_code01(key->n, o->stride) % o->size;
    // single threaded open addressing hash table operation
    int isKeyFound = memcmp(o->key + x * o->stride, key->n,
                            sizeof(*key->n) * o->stride);
    int isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                              sizeof(*o->empty_key) * o->stride);
    while (isEmptyFound != 0 && isKeyFound != 0 && tries <= o->size) {
        tries++;
        x = (x + 1) % o->size;
        isKeyFound = memcmp(o->key + x * o->stride, key->n,
                            sizeof(*key->n) * o->stride);
        isEmptyFound = memcmp(o->key + x * o->stride, o->empty_key,
                              sizeof(*o->empty_key) * o->stride);
    }
    if (o->size < tries) {
        return ALDER_STATUS_ERROR;
    }
    if (isEmptyFound == 0) {
        memcpy(o->key + x * o->stride, key->n, sizeof(*key->n) * o->stride);
    }
    o->value[x] = value;
    return ALDER_STATUS_SUCCESS;
}



int
alder_hashtable_mcas_convertCountToHashTable(alder_hashtable_mcas_t *ht,
                                             const char *fn,
                                             uint64_t xi_np,
                                             uint64_t xn_np)
{
    // Read the header.
    int K = 0;
    uint64_t S = 0;
    uint64_t ni = 0;
    uint64_t np = 0;
    int fildes;
    int s = alder_hashtable_mcas_large_open(fn, &fildes, &K, &S, &ni, &np);
    
    if (s != ALDER_STATUS_SUCCESS) {
        return s;
    }
    
    alder_encode_number_t *key = alder_encode_number_create_for_kmer(K);
    
    uint16_t value;
    //
    for (size_t i_s = 0; i_s < (size_t)S; i_s++) {
        // Read each element: key and value.
        //            alder_hashtable_mcas_large_next(src, fildes);
        //        alder_hashtable_mcas_nextFromFile(fildes, key, &value);
        
        assert(0);
        alder_hashtable_mcas_push(ht, key, value);
        
    }
    alder_hashtable_mcas_large_close(fildes);
    
    alder_encode_number_destroy(key);
    return 0;
}



