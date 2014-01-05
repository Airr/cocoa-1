/**
 * This file, alder_encode_number.c, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "alder_dna.h"
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "alder_hash.h"
#include "alder_encode_number.h"

/* This function tests features of alder_encode_number.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_encode_number_test()
{
    int s = 0;
    alder_encode_number_t *n1 = NULL;
    bstring bkmer;
    
    // Kmer - 32-mer
    bkmer = bfromcstr("CGTACGTACGTACGTACGTACGTACGTACGT");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    s = alder_encode_number_is_same_byte(n1);
    assert(s == 1);
    alder_encode_number_print_in_DNA(stdout,n1);
    fprintf(stdout, "\n");
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    uint64_t x = 10347427875802419272LLU;
    x = x % 23;
    
    // AGAGAACATCTTGATATAAAGTGTA
    // TACACTTTATATCAAGATGTTCTCT
    bkmer = bfromcstr("CGCTCTCCAGCTCAGGCGCCTCGCT");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    fprintf(stdout, "\n");
    bdestroy(bkmer);
    uint64_t h = alder_encode_number_hash(n1);
    fprintf(stdout, "hash: %llu\n", h);
    h = h % 23;
    fprintf(stdout, "hash23: %llu\n", h);
    alder_encode_number_destroy(n1);
    
    bkmer = bfromcstr("AGCGAGGCGCCTGAGCTGGAGAGCG");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    fprintf(stdout, "\n");
    bdestroy(bkmer);
    h = alder_encode_number_hash(n1);
    fprintf(stdout, "hash: %llu\n", h);
    h = h % 23;
    fprintf(stdout, "hash23: %llu\n", h);
    alder_encode_number_destroy(n1);
    
    
    // Rev Kmer - A
    uint8_t ukmer[33] = {0};
    n1 = alder_encode_number_create_for_kmer(1);
    s = alder_encode_number_kmer_rev(n1, ukmer);
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1); fprintf(stdout, "\n");
    alder_encode_number_destroy(n1);
    
    // Rev Kmer - A
    for (int i = 0; i < 33; i++) {
        ukmer[i] = i % 4;
    }
    n1 = alder_encode_number_create_for_kmer(33);
    s = alder_encode_number_kmer_rev(n1, ukmer);
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1); fprintf(stdout, "\n");
    alder_encode_number_destroy(n1);
    
    
    // Kmer - A
    bkmer = bfromcstr("A");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // Kmer - ACGT
    bkmer = bfromcstr("ACGT");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // 31-mer
    // Kmer - ACGTACGTACGTACGTACGTACGTACGTACG
    bkmer = bfromcstr("ACGTACGTACGTACGTACGTACGTACGTACG");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // 62-mer
    // Kmer - ACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGT
    bkmer = bfromcstr("ACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGTACGT");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // 3-mer
    // Kmer - AAA
    // Shift left with C(1).
    bkmer = bfromcstr("AAA");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,1);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,2);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,3);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // 32-mer
    // Kmer - AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // Shift left with C(1).
    bkmer = bfromcstr("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,1);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,2);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_left_with(n1,3);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    
    // 3-mer
    // Kmer - AAA
    // Shift left with C(1).
    bkmer = bfromcstr("AAA");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_right_with(n1,1);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_right_with(n1,2);
    alder_encode_number_print_in_DNA(stdout,n1);
    alder_encode_number_kmer_shift_right_with(n1,3);
    alder_encode_number_print_in_DNA(stdout,n1);
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    
    // N-mer
    // Kmer - AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // Shift left with C(1).
    bkmer = bfromcstr("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    n1 = alder_encode_number_create_for_kmer(bkmer->slen);
    s = alder_encode_number_kmer_with_char(n1, bdata(bkmer));
    assert(s == 0);
    alder_encode_number_print_in_DNA(stdout,n1);
    for (int i = 0; i < bkmer->slen; i++) {
        alder_encode_number_kmer_shift_right_with(n1,i%4);
        alder_encode_number_print_in_DNA(stdout,n1);
    }
    bdestroy(bkmer);
    alder_encode_number_destroy(n1);
    return s;
}

/* This function creates a positive number for kmer of size being
 * kmer_size.
 * returns
 * alder_encode_number on success
 * nil if an error occurs.
 *
 * procedure
 * . Determine the number of 64-bit elements using kmer_size.
 *    1, 2, ...,31 -> n_int64 = 1
 *   32,34, ...,62 -> n_int64 = 2
 *   ...
 */
alder_encode_number_t * alder_encode_number_create_for_kmer(int kmer_size)
{
    int n_uint64 = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    alder_encode_number_t *o = malloc(sizeof(*o)*n_uint64);
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o)*n_uint64);
    o->n = malloc(sizeof(*o->n)*n_uint64);
    if (o->n == NULL) {
        alder_encode_number_destroy(o);
        return NULL;
    }
    memset(o->n, 0, sizeof(*o->n) * n_uint64);
    o->s = n_uint64;
    o->k = kmer_size;
    return o;
}

int alder_encode_number_copy(alder_encode_number_t *dst,
                             alder_encode_number_t *src)
{
    dst->k = src->k;
    dst->s = src->s;
    memcpy(dst->n, src->n, sizeof(*dst->n) * dst->s);
    return ALDER_STATUS_SUCCESS;
}

int alder_encode_number_compare(alder_encode_number_t *s1,
                                alder_encode_number_t *s2,
                                size_t d)
{
    
    
    return 0;
}

/* This function destroys the encoded large number.
 */
void alder_encode_number_destroy(alder_encode_number_t *o)
{
    if (o != NULL) {
        XFREE(o->n);
        XFREE(o);
    }
}

/* This function creates a large number representing a kmer without owning the array of 64-bit numbers.
 * NOTE: Use alder_encode_number_destroy_view to deallocate it.
 */
alder_encode_number_t * alder_encode_number_create_for_kmer_view(int kmer_size)
{
    int n_uint64 = ALDER_BYTESIZE_KMER(kmer_size,ALDER_NUMKMER_8BYTE);
    alder_encode_number_t *o = malloc(sizeof(*o)*n_uint64);
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o)*n_uint64);
    o->n = NULL;
    o->s = n_uint64;
    o->k = kmer_size;
    return o;
}

/* This function destroys the encoded large number for view.
 * NOTE: It does not deallocate the array of 64-bit numbers because it did not own it.
 * Call this function to delete alder_encode_number_t that is created by
 * alder_encode_number_create_for_kmer_view.
 */
void alder_encode_number_destroy_view(alder_encode_number_t *o)
{
    if (o != NULL) {
        XFREE(o);
    }
}

/* This function writes n2 to the file, and then n1. If some of digits do not
 * fit to a byte, the remaining digits are saved to n2.
 *
 * x is a byte.
 * i_x = 0..7 or index of bit in a byte.
 * l_n2 is the number of bits with remaining meaningful things in n2.
 * nk is the remaining k-mers in n2.
 * i_n is index of current uint64 position.
 * n is the number of uint64 type at the index i_n.
 * j_n is the index of the current bit.
 */
void alder_encode_number_packwrite_buffer(uint8_t *buf, size_t *pn_buf,
                                          alder_encode_number_t *n1,
                                          alder_encode_number_t *n2,
                                          int *pnk)
{
//    alder_encode_number_print_in_DNA(stdout, n1); fputc('\n', stdout);
//    alder_encode_number_print_in_DNA(stdout, n2); fputc('\n', stdout);
    // . Take ones from n2 and ...
    // . Take ones from n1.
    // http://www.codechef.com/wiki/tutorial-bitwise-operations
    size_t n_buf = *pn_buf;
    int nk = *pnk;
    unsigned char x = 0;
    int i_x = 0;
    int l_n2 = nk * 2;
    while (l_n2 > 0) {
        int i_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 / ALDER_NUMBIT_KMER_8BYTE : l_n2 / ALDER_NUMBIT_KMER_8BYTE - 1;
        uint64_t n = n2->n[i_n];
        int j_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;

        if (i_x == 8) {
            assert(0);
            *(buf + n_buf) = x;
            n_buf++;
            i_x = 0;
            x = 0;
        }

        if (i_x < 8) {
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
        }
        l_n2--;
    }

    // Now, we need to do this for n1. I use the carried-over number x.
    // x 
    // i_x
    int l_n1 = n1->k * 2;
    while (l_n1 > 0) {
        int i_n = l_n1 % ALDER_NUMBIT_KMER_8BYTE ? l_n1 / ALDER_NUMBIT_KMER_8BYTE : l_n1 / ALDER_NUMBIT_KMER_8BYTE - 1;
        uint64_t n = n1->n[i_n];
        int j_n = l_n1 % ALDER_NUMBIT_KMER_8BYTE ? l_n1 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;

        if (i_x == 8) {
            *(buf + n_buf) = x;
//            fprintf(stdout, "%x ", x);
            n_buf++;
            i_x = 0;
            x = 0;
        }

        if (i_x < 8) {
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
        }
        l_n1--;
    }
    if (i_x == 8) {
        *(buf + n_buf) = x;
        n_buf++;
        i_x = 0;
        x = 0;
    }
    
    // Now, I have i_x and x. I need to put it back to n2.
    n2->n[0] = 0;
    for (int i = 0; i < i_x; i++) {
        if (x & (0x01 << (7 - i))) {
            n2->n[0] |= (1ULL << (i_x - i - 1));
        }
    }
    assert(i_x % 2 == 0);
    *pnk = i_x / 2;
    *pn_buf = n_buf;
}

/* This function flushes out buffers.
 */
void alder_encode_number_packflush_buffer(uint8_t *buf, size_t *pn_buf,
                                          alder_encode_number_t *n2,
                                          int *pnk)
{
    size_t n_buf = *pn_buf;
    int nk = *pnk;
    unsigned char x = 0;
    int i_x = 0;
    int l_n2 = nk * 2;
    if (l_n2 > 0) {
        while (l_n2 > 0) {
            int i_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 / ALDER_NUMBIT_KMER_8BYTE : l_n2 / ALDER_NUMBIT_KMER_8BYTE - 1;
            uint64_t n = n2->n[i_n];
            int j_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;
            
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
            l_n2--;
        }
        *(buf + n_buf) = x;
        n_buf++;
        i_x = 0;
    }

    assert(i_x % 2 == 0);
    *pnk = i_x / 2;
    *pn_buf = n_buf;
}
/* This function writes n2 to the file, and then n1. If some of digits do not
 * fit to a byte, the remaining digits are saved to n2.
 *
 * x is a byte.
 * i_x = 0..7 or index of bit in a byte.
 * l_n2 is the number of bits with remaining meaningful things in n2.
 * nk is the remaining k-mers in n2.
 * i_n is index of current uint64 position.
 * n is the number of uint64 type at the index i_n.
 * j_n is the index of the current bit.
 */
void alder_encode_number_packwrite(FILE *fp, 
                                   alder_encode_number_t *n1,
                                   alder_encode_number_t *n2,
                                   int *pnk)
{
//    alder_encode_number_print_in_DNA(stdout, n1); fputc('\n', stdout);
//    alder_encode_number_print_in_DNA(stdout, n2); fputc('\n', stdout);
    // . Take ones from n2 and ...
    // . Take ones from n1.
    // http://www.codechef.com/wiki/tutorial-bitwise-operations
    int nk = *pnk;
    unsigned char x = 0;
    int i_x = 0;
    int l_n2 = nk * 2;
    while (l_n2 > 0) {
        int i_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 / ALDER_NUMBIT_KMER_8BYTE : l_n2 / ALDER_NUMBIT_KMER_8BYTE - 1;
        uint64_t n = n2->n[i_n];
        int j_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;

        if (i_x == 8) {
            assert(0);
            fputc(x,fp);
            i_x = 0;
            x = 0;
        }

        if (i_x < 8) {
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
        }
        l_n2--;
    }

    // Now, we need to do this for n1. I use the carried-over number x.
    // x 
    // i_x
    int l_n1 = n1->k * 2;
    while (l_n1 > 0) {
        int i_n = l_n1 % ALDER_NUMBIT_KMER_8BYTE ? l_n1 / ALDER_NUMBIT_KMER_8BYTE : l_n1 / ALDER_NUMBIT_KMER_8BYTE - 1;
        uint64_t n = n1->n[i_n];
        int j_n = l_n1 % ALDER_NUMBIT_KMER_8BYTE ? l_n1 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;

        if (i_x == 8) {
            fputc(x,fp);
            i_x = 0;
            x = 0;
        }

        if (i_x < 8) {
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
        }
        l_n1--;
    }

    // Now, I have i_x and x. I need to put it back to n2.
    n2->n[0] = 0;
    for (int i = 0; i < i_x; i++) {
        if (x & (0x01 << (7 - i))) {
            n2->n[0] |= (1ULL << (i_x - i - 1));
        }
    }
    assert(i_x % 2 == 0);
    *pnk = i_x / 2;
}

/* This function flushes the content of the number.
 */
void alder_encode_number_packflush(FILE *fp,
                                   alder_encode_number_t *n2,
                                   int *pnk)
{
    int nk = *pnk;
    unsigned char x = 0;
    int i_x = 0;
    int l_n2 = nk * 2;
    if (l_n2 > 0) {
        while (l_n2 > 0) {
            int i_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 / ALDER_NUMBIT_KMER_8BYTE : l_n2 / ALDER_NUMBIT_KMER_8BYTE - 1;
            uint64_t n = n2->n[i_n];
            int j_n = l_n2 % ALDER_NUMBIT_KMER_8BYTE ? l_n2 % ALDER_NUMBIT_KMER_8BYTE - 1 : ALDER_NUMBIT_KMER_8BYTE - 1;
            
            if (n & (1ULL << j_n)) {     // Check if (j_n)-th bit is on.
                x |= 1 << (7 - i_x);     // Set the (7 - i_x)-th bit on.
            }
            i_x++;
            l_n2--;
        }
        fputc(x,fp);
        i_x = 0;
    }
    
    assert(i_x % 2 == 0);
    *pnk = i_x / 2;
}
/* This function converts an array of numbers to kmer. Each uint64_t 
 * accommodates 31 bases not 32.
 * kmer: an array of 0,1,2,3. The size must be o->k or kmer_size.
 *
 * returns
 * 0 on success
 *
 * Consider kmer: ACGTC, [0] A [1] C [2] G [3] T [4] C. This is a natural way
 * of storing a sequence in an array because I'd read one letter at a time.
 * Assuming that [0] is the MSB, and [4] is the LSB. This could be confusing,
 * but I'd have to choose one or the other way. Then,
 * the bit packed array is stored in LITTLE endian way.
 * n: [0] CT [1] GC [2] A
 */
int alder_encode_number_kmer(alder_encode_number_t *o, uint8_t *kmer)
{
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = 0;
    for (int i = 0; i < kmer_size; i++) {
        x |= (uint64_t)kmer[i];
        if ((kmer_size - i - 1) % ALDER_NUMKMER_8BYTE == 0) {
            o->n[ni--] = x;
            x = 0;
        } else {
            x <<= 2;
        }
    }
    assert(ni == -1);
    return 0;
}

/* This function converts an array of numbers to the reverse complementary of a kmer.
 * kmer: an array of 0,1,2,3. The size must be o->k or kmer_size.
 *
 * returns
 * 0 on success
 */
int alder_encode_number_kmer_rev(alder_encode_number_t *o, uint8_t *kmer)
{
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = 0;
    for (int i = kmer_size - 1; !(i < 0); i--) {
        x |= (uint64_t)((kmer[i]+2)%4);
        if (i % ALDER_NUMKMER_8BYTE == 0) {
            o->n[ni--] = x;
            x = 0;
        } else {
            x <<= 2;
        }
    }
    assert(ni == -1);
    return 0;
}


/* This function encodes an array of A, C, G, T to a number.
 * returns
 * 0 on success
 * 1 if an error occurs.
 * 
 * kmer: [0]A [1]C [2]G [3]T
 * x: 0         x |= 0;
 * x: 00        x <<= 2;
 * x: 0000      x |= 1
 * x: 0001      x <<= 2;
 * x: 000100    x |= 3
 * x: 000111    x <<= 2;
 * x: 00011100  x |= 2
 * x: 00011110  o->n[0] = x;
 *
 * It is confusing how an array of A,C,G,T is packed into an array of 64-bit
 * values. Consider an array of 64-bit values of size 3.
 * n: [0] [1] [2]
 * I'd place the element of highest index first, and next elements.
 * n: [2] [1] [0]
 * Consider an array of A,C,G,T. 
 * n: [2] [1] [0] <- ACGTC
 * if n's element can take 2 letters,
 * n: [2] A [1] CG [0] TC
 *
 * This seems to be natural because I take one letter at a time and push it
 * from the right of the array n, where I'd see the index values decrease.
 * But, this could be weird if I see the index values naturally; i.e.,
 * n: [0] TC [1] CG [2] A
 * I could flop the letters in each element to see the reverse of the 5 
 * letters.
 * n: [0] CT [1] GC [2] A
 * So, I could see that ACGTC is pushed into the memory n:
 * ACGTC -> n: [0] [1] [2]
 * from left to right.
 * So, this reminds me of LITTLE ENDIAN scheme.
 * The second C is "LSB" and it is store in the lowest position.
 * A is "MSB", which is stored in the highest position of the memory.
 *
 */
int alder_encode_number_kmer_with_char(alder_encode_number_t *o, char *kmer)
{
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = 0;
    for (int i = 0; i < kmer_size; i++) {
        char c = kmer[i];
        if (c == 'A' || c == 'C' || c == 'G' || c == 'T' ||
            c == 'a' || c == 'c' || c == 'g' || c == 't') {
            x |=  (uint64_t)((((int)kmer[i])>>1)&3);
            if ((kmer_size - i - 1) % ALDER_NUMKMER_8BYTE == 0) {
                o->n[ni--] = x;
                x = 0;
            } else {
                x <<= 2;
            }
        } else {
            return 1;
        }
    }
    assert(ni == -1);
    return 0;
}

/* This function creates an encode number for the reverse complementary of a
 * DNA sequence encoded by o.
 */
alder_encode_number_t *
alder_encode_number_create_rev(alder_encode_number_t *o)
{
    alder_encode_number_t *o2 = alder_encode_number_create_for_kmer(o->k);
    ALDER_RETURN_NULL_IF_NULL(o2);
    
    bstring bs = bfromcstralloc(o->k + 1, "");
    int s = alder_encode_number_copy_in_revDNA((char*)bs->data, o);
    if (s != 0) {
        return NULL;
    }
    bs->slen = o->k;
    bs->data[o->k] = '\0';
    
    s = alder_encode_number_kmer_with_char(o2, (char*)bs->data);
    if (s != 0) {
        alder_encode_number_destroy(o2);
        o2 = NULL;
        return NULL;
    }
    bdestroy(bs);
    return o2;
}

/* This function converts a kmer in a byte buffer. 
 *
 * The buffer contains compacted kmers using 2 bits for a base. I test each of
 * the bits and insert it into the 64-bit number from right-hand side.
 * returns
 * cbuf - current buffer pointer
 * procedure
 * . Set x to the value pointed by cbuf.
 * . Set the least bit if j-th bit of the current byte x.
 * . If the current bit is the last bit in a 64-bit number, 
 *   store the value at the corresponding 64-bit number, and
 *   reset y.
 * . Otherwise, shift y to the left one bit.
 * . Decrease j by one.
 * . Adjust j if it becomes a negative value, and
 *   move the buffer and set the value of x to the value in
 *   the buffer.
 * .
 *
 * y : elements of 64-bit values in an encoded Kmer
 *
 */
uint8_t* alder_encode_number_kmer_with_packed(alder_encode_number_t *o,
                                              uint8_t *cbuf,
                                              int *i_8bit)
{
    int ni = o->s - 1;
    uint64_t y = 0;
    const int size_kmer_in_bit = 2 * o->k;
    
    uint8_t x = *cbuf;
    int j = *i_8bit;
    for (int i = 0; i < size_kmer_in_bit; i++) {
        if (j < 0) {
            j = 7;
            cbuf++;
            x = *cbuf;
        }
        if (x & (1 << j)) {
            y |= 1;
        }
        if ((size_kmer_in_bit - i) % ALDER_NUMBIT_KMER_8BYTE == 1) {
            o->n[ni--] = y;
            y = 0;
        } else {
            y <<= 1;
        }
        j--;
    }
    assert(y == 0);
    assert(ni == -1);
    assert(j == -1 || j % 2 == 1);
    *i_8bit = j;
    return cbuf;
}

#pragma mark print

/* This function copies the encoded number to a string.
 * bseq must have been allocated.
 * The start position of the string is at offset.
 * If offset is greater than or equal to the length of the string,
 * the sequence is truncated to a null string.
 */
int
alder_encode_number_to_bstringWithOffset(alder_encode_number_t *o,
                                         bstring bseq, int offset)
{
    int r = btrunc(bseq, 0);
    if (r != BSTR_OK) {
        return ALDER_STATUS_ERROR;
    }
    int kmer_size = o->k;
    int s = ALDER_BYTESIZE_KMER(kmer_size-offset,ALDER_NUMKMER_8BYTE);
    int ni = s - 1;
    uint64_t x = o->n[ni--];
    for (int i = offset; i < kmer_size; i++) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        bconchar(bseq, alder_dna_int2char(c));
        if (ni >= 0 && shift_len == 0) {
            x = o->n[ni--];
        }
    }
    return ALDER_STATUS_SUCCESS;
}

/* This function copies the encoded number to a string.
 */
int
alder_encode_number_to_bstring(alder_encode_number_t *o, bstring bseq)
{
    return alder_encode_number_to_bstringWithOffset(o, bseq, 0);
}

/*
 */
int alder_encode_number_print(alder_encode_number_t *o)
{
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = o->n[ni--];
    for (int i = 0; i < kmer_size; i++) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        fprintf(stdout, "%d", c);
        if (ni >= 0 && shift_len == 0) {
            x = o->n[ni--];
        }
    }
    fprintf(stdout, "\n");
    assert(ni == -1);
    return 0;
}

/* This function prints the encoded DNA sequence in the large number.
 */
int alder_encode_number_print_in_DNA(FILE *fp, alder_encode_number_t *o)
{
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = o->n[ni--];
    for (int i = 0; i < kmer_size; i++) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        if (c == 0) {
            fprintf(fp, "A");
        } else if (c == 1) {
            fprintf(fp, "C");
        } else if (c == 2) {
            fprintf(fp, "T");
        } else if (c == 3) {
            fprintf(fp, "G");
        }
        if (ni >= 0 && shift_len == 0) {
            x = o->n[ni--];
        }
    }
//    fprintf(fp, "\n");
    assert(ni == -1);
    return 0;
}

/* This function converts the encoded number to a string.
 * Make sure that s1 points to a proper memory.
 * given
 * s1 - the memory address for the string
 * o - the number representation of a DNA sequence
 */
int alder_encode_number_copy_in_DNA(char *s1, alder_encode_number_t *o)
{
    char *s2 = s1;
    int kmer_size = o->k;
    int ni = o->s - 1;
    uint64_t x = o->n[ni--];
    for (int i = 0; i < kmer_size; i++) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        if (c == 0) {
            *s2 = 'A'; s2++;
        } else if (c == 1) {
            *s2 = 'C'; s2++;
        } else if (c == 2) {
            *s2 = 'T'; s2++;
        } else if (c == 3) {
            *s2 = 'G'; s2++;
        }
        if (ni >= 0 && shift_len == 0) {
            x = o->n[ni--];
        }
    }
    *s2 = '\0';
    assert(ni == -1);
    return 0;
}


/* This function prints the reverse complementary of an encoded DNA sequence 
 * in the large number.
 */
int alder_encode_number_print_in_revDNA(FILE *fp, alder_encode_number_t *o)
{
    int kmer_size = o->k;
    int ni = 0;
    uint64_t x = o->n[ni++];
    for (int i = kmer_size - 1; !(i < 0); i--) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        if (c == 0) {
            fprintf(fp, "T");
        } else if (c == 1) {
            fprintf(fp, "G");
        } else if (c == 2) {
            fprintf(fp, "A");
        } else if (c == 3) {
            fprintf(fp, "C");
        }
        if (ni < o->s && shift_len == ALDER_NUMKMER_8BYTE - 1) {
            x = o->n[ni++];
        }
    }
//    fprintf(fp, "\n");
    assert(ni == o->s);
    return 0;
}

/* This function converts the encoded number to a string of the reverse complementary
 * of the sequence represented by the encoded number.
 * Make sure that s1 points to a proper memory.
 * given
 * s1 - the memory address for the string
 * o - the number representation of a DNA sequence
 */
int alder_encode_number_copy_in_revDNA(char *s1, alder_encode_number_t *o)
{
    char *s2 = s1;
    int kmer_size = o->k;
    int ni = 0;
    uint64_t x = o->n[ni++];
    for (int i = kmer_size - 1; !(i < 0); i--) {
        int shift_len = (kmer_size - i - 1) % ALDER_NUMKMER_8BYTE;
        int c = (x >> 2*shift_len) & 3LLU;
        if (c == 0) {
            *s2 = 'T'; s2++;
        } else if (c == 1) {
            *s2 = 'G'; s2++;
        } else if (c == 2) {
            *s2 = 'A'; s2++;
        } else if (c == 3) {
            *s2 = 'C'; s2++;
        }
        if (ni < o->s && shift_len == ALDER_NUMKMER_8BYTE - 1) {
            x = o->n[ni++];
        }
    }
    *s2 = '\0';
    assert(ni == o->s);
    return 0;
}

/* This function computes a hash value using a variant of double hashing.
 * h(m) = h00(m0) + 1 x h00(m1) + 2 x h00(m2) + ...
 * where h00() is a hash function on a 64bit value. I got this from DSK
 * authored by Rizk, et al. (2012).
 */
uint64_t alder_encode_number_hash(alder_encode_number_t *o)
{
    uint64_t x = alder_hash_code00(o->n[0]);
    for (int i = 1; i < o->s; i++) {
        x += (i * alder_hash_code00(o->n[i]));
    }
    return x;
}

/*
static uint64_t alder_bit_kmer2number_matrix[32][4] = {
    {0x0000000000000000, 0x0000000000000001, 0x0000000000000002, 0x0000000000000003},
    {0x0000000000000000, 0x0000000000000004, 0x0000000000000008, 0x000000000000000C},
    {0x0000000000000000, 0x0000000000000010, 0x0000000000000020, 0x0000000000000030},
    {0x0000000000000000, 0x0000000000000040, 0x0000000000000080, 0x00000000000000C0},
    {0x0000000000000000, 0x0000000000000100, 0x0000000000000200, 0x0000000000000300},
    {0x0000000000000000, 0x0000000000000400, 0x0000000000000800, 0x0000000000000C00},
    {0x0000000000000000, 0x0000000000001000, 0x0000000000002000, 0x0000000000003000},
    {0x0000000000000000, 0x0000000000004000, 0x0000000000008000, 0x000000000000C000},
    {0x0000000000000000, 0x0000000000010000, 0x0000000000020000, 0x0000000000030000},
    {0x0000000000000000, 0x0000000000040000, 0x0000000000080000, 0x00000000000C0000},
    {0x0000000000000000, 0x0000000000100000, 0x0000000000200000, 0x0000000000300000},
    {0x0000000000000000, 0x0000000000400000, 0x0000000000800000, 0x0000000000C00000},
    {0x0000000000000000, 0x0000000001000000, 0x0000000002000000, 0x0000000003000000},
    {0x0000000000000000, 0x0000000004000000, 0x0000000008000000, 0x000000000C000000},
    {0x0000000000000000, 0x0000000010000000, 0x0000000020000000, 0x0000000030000000},
    {0x0000000000000000, 0x0000000040000000, 0x0000000080000000, 0x00000000C0000000},
    {0x0000000000000000, 0x0000000100000000, 0x0000000200000000, 0x0000000300000000},
    {0x0000000000000000, 0x0000000400000000, 0x0000000800000000, 0x0000000C00000000},
    {0x0000000000000000, 0x0000001000000000, 0x0000002000000000, 0x0000003000000000},
    {0x0000000000000000, 0x0000004000000000, 0x0000008000000000, 0x000000C000000000},
    {0x0000000000000000, 0x0000010000000000, 0x0000020000000000, 0x0000030000000000},
    {0x0000000000000000, 0x0000040000000000, 0x0000080000000000, 0x00000C0000000000},
    {0x0000000000000000, 0x0000100000000000, 0x0000200000000000, 0x0000300000000000},
    {0x0000000000000000, 0x0000400000000000, 0x0000800000000000, 0x0000C00000000000},
    {0x0000000000000000, 0x0001000000000000, 0x0002000000000000, 0x0003000000000000},
    {0x0000000000000000, 0x0004000000000000, 0x0008000000000000, 0x000C000000000000},
    {0x0000000000000000, 0x0010000000000000, 0x0020000000000000, 0x0030000000000000},
    {0x0000000000000000, 0x0040000000000000, 0x0080000000000000, 0x00C0000000000000},
    {0x0000000000000000, 0x0100000000000000, 0x0200000000000000, 0x0300000000000000},
    {0x0000000000000000, 0x0400000000000000, 0x0800000000000000, 0x0C00000000000000},
    {0x0000000000000000, 0x1000000000000000, 0x2000000000000000, 0x3000000000000000},
    {0x0000000000000000, 0x4000000000000000, 0x8000000000000000, 0xC000000000000000}
};
*/

//static uint64_t alder_encode_number_kmer_shift_left_with_array[31] = {
//    0x3FFFFFFFFFFFFFFF,
//    0x0000000000000003,
//    0x000000000000000F,
//    0x000000000000003F,
//    0x00000000000000FF,
//    0x00000000000003FF,
//    0x0000000000000FFF,
//    0x0000000000003FFF,
//    0x000000000000FFFF,
//    0x000000000003FFFF,
//    0x00000000000FFFFF,
//    0x00000000003FFFFF,
//    0x0000000000FFFFFF,
//    0x0000000003FFFFFF,
//    0x000000000FFFFFFF,
//    0x000000003FFFFFFF,
//    0x00000000FFFFFFFF,
//    0x00000003FFFFFFFF,
//    0x0000000FFFFFFFFF,
//    0x0000003FFFFFFFFF,
//    0x000000FFFFFFFFFF,
//    0x000003FFFFFFFFFF,
//    0x00000FFFFFFFFFFF,
//    0x00003FFFFFFFFFFF,
//    0x0000FFFFFFFFFFFF,
//    0x0003FFFFFFFFFFFF,
//    0x000FFFFFFFFFFFFF,
//    0x003FFFFFFFFFFFFF,
//    0x00FFFFFFFFFFFFFF,
//    0x03FFFFFFFFFFFFFF,
//    0x0FFFFFFFFFFFFFFF
//};

/* This function shift-left the number.
 *
 * Consider an array of uint64_t.
 * n: [2] xA [1] CG [0] TC
 * Here ACGTC is the sequence. Let's add a letter to extend the sequence:
 * ACGTC. The letter is G. Then,
 * n: [2] AC [1] GT [0] CG
 *
 * -----
 * ACGTCG
 *  -----
 *
 * This is an imanginary array of size 3. Each word is of 8 bits.
 * [2] 0000abcd [1] 00efghij [0] 00klmnop
 * Iteration i = 0
 * ni = 00klmnop
 * msb2 = 00
 * msb4 = kl
 *
 * [2] 00abcdef [1] 00ghijkl [0] 00mnop00
 *
 * Make sure the two MSB bits stay puts.
 */
int alder_encode_number_kmer_shift_left(alder_encode_number_t *o)
{
    return alder_encode_number_kmer_shift_left_with(o, 3);
}

/* This function shifts a kmer one base right.
 *
 */
int alder_encode_number_kmer_shift_right(alder_encode_number_t *o)
{
    return alder_encode_number_kmer_shift_right_with(o, 3);
}

/* This function replace the last character with one that is represented by b.
 * e.g.,
 * CAG -> CAA if b is 0.
 */
int alder_encode_number_kmer_replace_left_with(alder_encode_number_t *o, int b)
{
    const uint64_t x3 = 3;
    uint64_t carryover = b;
    uint64_t ni = o->n[0];
    ni &= ~x3;
    ni |= carryover;
    o->n[0] = ni;
    return 0;
}

/* This function replace the first character with one that is represented by b.
 * e.g.,
 * GTG -> TTG if b is 3.
 */
int alder_encode_number_kmer_replace_right_with(alder_encode_number_t *o, int b)
{
    const uint64_t x3 = 3;
    uint64_t carryover = b;
    uint64_t ni = o->n[o->s - 1];
    int k = (o->k - 1) % ALDER_NUMKMER_8BYTE;
    ni &= ~(x3 << k * 2);       // reset 2 bits.
    ni |= (carryover << k * 2); // replace them with carryover.
    o->n[o->s - 1] = ni;
    return 0;
}

/* This function shifts the kmer one base left.
 *
 * 1. Shift the first (the highest order) element left by 2.
 *    000132
 *      ^
 *    00132?
 * 2. Mask the highest base.
 *    00032?
 *      ^ 
 * 3. Find the carryover (C).
 * 4. Replace the the lowest order element by the carryover.
 *    00032C.
 *
 * FIXME:
 */
int alder_encode_number_kmer_shift_left_with(alder_encode_number_t *o, int b)
{
    const uint64_t x3 = 3;
    uint64_t carryover = b;
    for (int i = 0; i < o->s; i++) {
        uint64_t ni = o->n[i];
        uint64_t msb2 = (ni >> 62) & x3;
        uint64_t msb4 = (ni >> 60) & x3;
        ni <<= 2;
        // Reset the left-most character to null.
        if (i == o->s - 1) {
            int k = o->k % ALDER_NUMKMER_8BYTE;
            ni &= ~(x3 << k * 2);       // reset 2 bits.
        }
        ni |= carryover;    // replace 2 lsb bits with carryover.
        ni &= ~(x3 << 62);  // reset 2 msb bits.
        ni |= (msb2 << 62); // replace 2 msb bits with msb2.
        carryover = msb4;
        
        o->n[i] = ni;
    }
    return 0;
}

/* This function shifts a kmer one base right.
 *
 * This is an imanginary array of size 3. Each word is of 8 bits.
 * [2] 0000abcd [1] 00efghij [0] 00klmnop
 * I'd want to have an array:
 * [2] 0000xxab [1] 00cdefgh [0] 00ijklmn
 * where b or the inserted value is xx.
 *
 * Iteration i = 0
 *   carryover = ij in position [1].
 *   ni = 00klmnop
 *   msb2 = 00
 *   ni = 0000klmn after ni >>= 2
 *   ni = 00ijklmn after ni is added with carryover.
 *   ni's 2 msb remained unchanged.
 * Iteration i = 1
 *   carryover = cd in position [2]
 *   ni = 00efghij
 *   msb2 = 00
 *   ni = 0000efgh after ni >>= 2
 *   ni = 00cdefgh after ni is added with carryover.
 *   ni's 2 msb remained unchanged.
 * Iteration i = 2
 *   carryover = b or xx
 *   ni = 0000abcd
 *   msb2 = 00
 *   ni = 000000ab after ni >>= 2
 *   ni = 0000xxab after ni is added with carryover or xx.
 *   ni's 2 msb remained unchanged.
 * I'd have the following result.
 * [2] 00abcdef [1] 00ghijkl [0] 00mnop00
 *
 * Make sure the two MSB bits stay puts, which are used for MCAS operation.
 */
int alder_encode_number_kmer_shift_right_with(alder_encode_number_t *o, int b)
{
    const uint64_t x3 = 3;
    uint64_t carryover;
    for (int i = 0; i < o->s; i++) {
        if (i == o->s - 1) {
            carryover = b;
        } else {
            uint64_t ni2 = o->n[i+1];
            uint64_t lsb2 = ni2 & x3;
            carryover = lsb2;
        }
        
        uint64_t ni = o->n[i];
        uint64_t msb2 = (ni >> 62) & x3;
        ni >>= 2;
        
        if (i == o->s - 1) {
            int k = (o->k - 1) % ALDER_NUMKMER_8BYTE;
            ni &= ~(x3 << k * 2);       // reset 2 bits.
            ni |= (carryover << k * 2); // replace them with carryover.
        } else {
            ni &= ~(x3 << 60);       // reset 2 msb bits at 62, 61.
            ni |= (carryover << 60); // replace them with carryover.
        }
        
        ni &= ~(x3 << 62);  // reset 2 msb bits.
        ni |= (msb2 << 62); // replace 2 msb bits with msb2.
        
        o->n[i] = ni;
    }
    return 0;
}

/* This function tests if all bytes in the number is the same.
 * returns
 * 1 if all the bytes are the same.
 * 0 otherwise.
 */
int alder_encode_number_is_same_byte(alder_encode_number_t *o)
{
    // Get a byte.
    uint8_t x = 0;
    for (int i = 0; i < 8; i++) {
        if (o->n[0] & (1LLU << i)) {
            x |= (1 << i);
        }
    }
    // Find the 64-bit value with the byte.
    uint64_t x64 = 0;
    for (int i = 0; i < 64; i++) {
        int j = i % 8;
        if (x & (1 << j)) {
            x64 |= (1LLU << i);
        }
    }
    // Check if all of the value in the array are the same.
    int is_same = 1;
    for (int i = 0; i < o->s; i++) {
        if (o->n[i] != x64) {
            is_same = 0;
            break;
        }
    }
    return is_same;
}

/* This function prints the encoded number in a meaningful way.
 *
 *
 */
int alder_encode_number_log(alder_encode_number_t *o)
{
    int kmer_size = o->k;
    
    uint64_t x1 = 1;
    bstring s = bfromcstr("");
    for (int i = o->s - 1; i >= 0; i--) {
        uint64_t x = o->n[i];
        bconchar(s, '\n');
        bconchar(s, '[');
        bconchar(s, i + 48);
        bconchar(s, ']');
        for (int j = 63; j >= 0; j--) {
            if ((j + 1) % 8 == 0) {
                bconchar(s, ' ');
            }
            if (((x >> j) & x1) == x1) {
                bconchar(s, '1');
            } else {
                bconchar(s, '0');
            }
        }
    }
    alder_log4("n: %s", bdata(s));
    
    int kmer_pos = kmer_size % ALDER_NUMKMER_8BYTE;
    uint64_t x3 = 3;
    btrunc(s, 0);
    for (int i = o->s - 1; i >= 0; i--) {
        uint64_t x = o->n[i];
        bconchar(s, '\n');
        bconchar(s, '[');
        bconchar(s, i + 48);
        bconchar(s, ']');
        for (int j = 31; j >= 0; j--) {
            if ((j + 1) % 8 == 0) {
                bconchar(s, ' ');
            }
            uint64_t xc = (x >> j*2) & x3;
            if (j == 31) {
                if (xc == 0) {
                    bconchar(s, 'O');
                } else {
                    bconchar(s, 'X');
                }
            } else {
                if (i < o->s - 1 || kmer_pos > j) {
                    if (xc == 0) {
                        bconchar(s, 'A');
                    } else if (xc == 1) {
                        bconchar(s, 'C');
                    } else if (xc == 2) {
                        bconchar(s, 'T');
                    } else if (xc == 3) {
                        bconchar(s, 'G');
                    }
                } else {
                    bconchar(s, '?');
                }
            }
        }
    }
    alder_log4("n: %s", bdata(s));
    bdestroy(s);
    return 0;
}

/* This function prints the Kmer sequence of the encoded number.
 * I'd print three columns:
 * DNA sequence, hash value, and reverse complementary DNA sequence.
 */
int alder_encode_number_print_DNA_and_hash(alder_encode_number_t *o, FILE *fp)
{
    int kmer_size = o->k;
    bstring s = bfromcstr("");

    ///////////////////////////////////////////////////////////////////////////
    // hash value
    uint64_t h = alder_encode_number_hash(o);
    
    ///////////////////////////////////////////////////////////////////////////
    // DNA sequence
    int kmer_pos = kmer_size % ALDER_NUMKMER_8BYTE;
    if (kmer_pos == 0) kmer_pos = ALDER_NUMKMER_8BYTE;
    uint64_t x3 = 3;
    btrunc(s, 0);
    for (int i = o->s - 1; i >= 0; i--) {
        uint64_t x = o->n[i];
        for (int j = 31; j >= 0; j--) {
            uint64_t xc = (x >> j*2) & x3;
            if (j == 31) {
                // No code.
            } else {
                if (i < o->s - 1 || kmer_pos > j) {
                    if (xc == 0) {
                        bconchar(s, 'A');
                    } else if (xc == 1) {
                        bconchar(s, 'C');
                    } else if (xc == 2) {
                        bconchar(s, 'T');
                    } else if (xc == 3) {
                        bconchar(s, 'G');
                    }
                }
            }
        }
    }
    
    fprintf(fp, "%s\t%llu\t", bdata(s), h);
    
    ///////////////////////////////////////////////////////////////////////////
    // Reverse complementary
    btrunc(s, 0);
    for (int i = 0; i < o->s; i++) {
        uint64_t x = o->n[i];
        for (int j = 0; j < 32; j++) {
            uint64_t xc = (x >> j*2) & x3;
            if (j == 31) {
                // No code.
            } else {
                if (i < o->s - 1 || kmer_pos > j) {
                    xc = (xc + 2) % 4;             // Reverse complementary DNA
                    if (xc == 0) {
                        bconchar(s, 'A');
                    } else if (xc == 1) {
                        bconchar(s, 'C');
                    } else if (xc == 2) {
                        bconchar(s, 'T');
                    } else if (xc == 3) {
                        bconchar(s, 'G');
                    }
                }
            }
        }
    }
    
    fprintf(fp, "%s\n", bdata(s));
    
    bdestroy(s);
    return 0;
}

int alder_encode_number_buffer_log(uint8_t *buf, size_t sizeBuf, int kmersize)
{
    bstring s = bfromcstr("\n[0] ");
//    int kmer_pos = kmer_size % ALDER_NUMKMER_8BYTE;
    uint8_t x3 = 3;
    size_t seqlen = 0;
    size_t i_seq = 0;
    for (size_t i = 0; i < sizeBuf; i++) {
        uint8_t x = buf[i];
        
        for (int j = 3; j >= 0; j--) {
            uint8_t xc = (x >> j*2) & x3;
            if (xc == 0) {
                bconchar(s, 'A');
            } else if (xc == 1) {
                bconchar(s, 'C');
            } else if (xc == 2) {
                bconchar(s, 'T');
            } else if (xc == 3) {
                bconchar(s, 'G');
            }
            seqlen++;
            if (seqlen % 8 == 0) {
                bconchar(s, ' ');
            }
            if (seqlen % kmersize == 0) {
                seqlen = 0;
                bformata(s, "\n[%d] ", ++i_seq);
            }
        }
    }
    alder_log4("buffer: %s", bdata(s));
    bdestroy(s);
    return 0;
}

size_t
alder_encode_number_adjust_buffer_size_for_packed_kmer
(int kmer_size, size_t size_buf)
{
    if (kmer_size % 4 == 0) {
        size_buf = size_buf / (kmer_size/4) * (kmer_size/4);
    } else if (kmer_size % 4 == 1 || kmer_size % 4 == 3) {
        size_buf = size_buf / kmer_size * kmer_size;
    } else if (kmer_size % 4 == 2) {
        size_buf = size_buf / (kmer_size/2) * (kmer_size/2);
    }

    return size_buf;
}

///////////////////////////////////////////////////////////////////////////////
// Retired functions.
///////////////////////////////////////////////////////////////////////////////

/* This function shifts the kmer one base left.
 *
 * 1. Shift the first (the highest order) element left by 2.
 *    000132
 *      ^
 *    00132?
 * 2. Mask the highest base.
 *    00032?
 *      ^ 
 * 3. Find the carryover (C).
 * 4. Replace the the lowest order element by the carryover.
 *    00032C.
 *
 * FIXME:
 */
//int alder_encode_number_kmer_shift_left_with(alder_encode_number_t *o, int b)
//{
//    int s = o->s;
//    for (int i = 0; i < s; i++) {
//        uint64_t x = o->n[s-i-1] << 2;
//        uint64_t carryover;
//        if (i == 0) {
//            // FIXME: 31 does not work.
//            int k = o->k % ALDER_NUMKMER_8BYTE;
////            x &= alder_encode_number_kmer_shift_left_with_array[k];
//            x &= ~(1LLU << (k * 2 + 1));
//            x &= ~(1LLU << k * 2);
//        } else {
////            x &= 0x3FFFFFFFFFFFFFFF;
//            x &= ~(1LLU << (ALDER_NUMBIT_KMER_8BYTE + 1));
//            x &= ~(1LLU << ALDER_NUMBIT_KMER_8BYTE);
//        }
//        
//        if (i == s - 1) {
//            carryover = b;
//        } else {
//            carryover = (o->n[s-i-2] >> 60) & 3LLU;
//        }
//        
//        if (carryover == 0) {
//            // No code.
//            x &= ~(1LLU << 1);
//            x &= ~(1LLU);
//        } else if (carryover == 1) {
//            // 01
//            x &= ~(1LLU << 1);
//            x |= 1LLU;
//        } else if (carryover == 2) {
//            // 10
//            x |= 1LLU << 1;
//            x &= ~(1LLU);
//        } else if (carryover == 3) {
//            // 11
//            x |= 1LLU << 1;
//            x |= 1LLU;
//        }
////        x = x & (1 << 2);
////        x |= 3LLU;
////        x &= carryover;
//        o->n[s-i-1] = x;
//    }
//    return 0;
//}

/* This function shifts a kmer one base right.
 *
 * 1. Shift the last (the lowest order) element right by 2.
 *    000132
 *      ^
 *    ?00013   2 -> carry
 * 2. Mask the hiest base
 *    000013
 * 3. Replace the highest order by the carryover.
 *    C00013
 */
//int alder_encode_number_kmer_shift_right_with(alder_encode_number_t *o, int b)
//{
//    int s = o->s;
//    for (int i = 0; i < o->s; i++) {
//        uint64_t x = o->n[i] >> 2;
//        uint64_t carryover;
//        if (i == o->s - 1) {
//            int k = o->k % ALDER_NUMKMER_8BYTE;
//            x &= alder_encode_number_kmer_shift_left_with_array[k];
//        } else {
//            x &= 0x3FFFFFFFFFFFFFFF;
//        }
//        
//        int k;
//        if (i == s - 1) {
//            k = o->k % ALDER_NUMKMER_8BYTE;
//            if (k == 0) {
//                k = 30;
//            } else {
//                k--;
//            }
//            carryover = b;
//        } else {
//            k = 30;
//            carryover = o->n[i+1] & 3LLU;
//        }
//        
//        if (carryover == 0) {
//            // 00
//            x &= ~(1LLU << (2 * k + 1));
//            x &= ~(1LLU << (2 * k));
//        } else if (carryover == 1) {
//            // 01
//            x &= ~(1LLU << (2 * k + 1));
//            x |= 1LLU << (2 * k);
//        } else if (carryover == 2) {
//            // 10
//            x |= 1LLU << (2 * k + 1);
//            x &= ~(1LLU << (2 * k));
//        } else if (carryover == 3) {
//            // 11
//            x |= 1LLU << (2 * k + 1);
//            x |= 1LLU << (2 * k);
//        }
//        
//        o->n[i] = x;
//    }
//    return 0;
//}

