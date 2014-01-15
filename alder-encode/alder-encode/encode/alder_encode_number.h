/**
 * This file, alder_encode_number.h, is part of alder-encode.
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

#ifndef alder_encode_alder_encode_number_h
#define alder_encode_alder_encode_number_h

#include <stdio.h>
#include <stdint.h>
#include "bstrlib.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

typedef union {
    uint8_t key8[8];
    uint64_t key64;
} alder_encode_64to8_t;

typedef struct alder_encode_number2_struct {
    alder_encode_64to8_t *n;              /* number of 64-bit elements */
    int s;                                /* size of n                 */
    int k;                                /* kmer size                 */
    int b;                                /* size of valid bytes       */
} alder_encode_number2_t;

int alder_encode_number2_test();
alder_encode_number2_t * alder_encode_number2_createWithKmer(int kmer_size);
void alder_encode_number2_destroy(alder_encode_number2_t *o);
void alder_encode_number2_reset(alder_encode_number2_t *o);
int alder_encode_number2_shiftLeftWith(alder_encode_number2_t *o, int b);
int alder_encode_number2_shiftRightWith(alder_encode_number2_t *o, int b);
uint64_t alder_encode_number2_hash(alder_encode_number2_t *o);
int alder_encode_number2_log(alder_encode_number2_t *o);
size_t alder_encode_number2_adjustBufferSizeForKmer(int K, size_t s);
int alder_encode_number2_kmer(alder_encode_number2_t *o, uint8_t *kmer);
int alder_encode_number2_bytesize(int K);

#pragma mark version1

/* This struct is for a large number encoding a kmer.
 */
typedef struct alder_encode_number_struct {
    uint64_t *n;              /* number of 64-bit elements */
    int s;                    /* size of n                 */
    int k;                    /* kmer size                 */
} alder_encode_number_t;

alder_encode_number_t * alder_encode_number_create_for_kmer(int kmer_size);
void alder_encode_number_destroy(alder_encode_number_t *o);
int alder_encode_number_kmer(alder_encode_number_t *o, uint8_t *kmer);
int alder_encode_number_kmer_rev(alder_encode_number_t *o, uint8_t *kmer);
int alder_encode_number_kmer_with_char(alder_encode_number_t *o, char *kmer);
uint8_t* alder_encode_number_kmer_with_packed(alder_encode_number_t *o,
                                              uint8_t *cbuf,
                                              int *i_8bit);
int alder_encode_number_print(alder_encode_number_t *o);
int alder_encode_number_print_in_DNA(FILE *fp, alder_encode_number_t *o);
int alder_encode_number_print_in_revDNA(FILE *fp, alder_encode_number_t *o);
int alder_encode_number_copy_in_DNA(char *s1, alder_encode_number_t *o);
int alder_encode_number_copy_in_revDNA(char *s1, alder_encode_number_t *o);
uint64_t alder_encode_number_hash(alder_encode_number_t *o);

/* 31-pack not 32-pack. */
int alder_encode_number_kmer_shift_left_with(alder_encode_number_t *o, int b);
int alder_encode_number_kmer_shift_right_with(alder_encode_number_t *o, int b);
int alder_encode_number_kmer_shift_left(alder_encode_number_t *o);
int alder_encode_number_kmer_shift_right(alder_encode_number_t *o);
int alder_encode_number_kmer_replace_left_with(alder_encode_number_t *o, int b);
int alder_encode_number_kmer_replace_right_with(alder_encode_number_t *o, int b);

void alder_encode_number_packwrite(FILE *fp, 
                                   alder_encode_number_t *n1,
                                   alder_encode_number_t *n2,
                                   int *pnk);
void alder_encode_number_packflush(FILE *fp,
                                   alder_encode_number_t *n2,
                                   int *pnk);
void alder_encode_number_packwrite_buffer(uint8_t *buf, size_t *pn_buf,
                                          alder_encode_number_t *n1,
                                          alder_encode_number_t *n2,
                                          int *pnk);
void alder_encode_number_packflush_buffer(uint8_t *buf, size_t *pn_buf,
                                          alder_encode_number_t *n2,
                                          int *pnk);
alder_encode_number_t * alder_encode_number_create_for_kmer_view(int kmer_size);
void alder_encode_number_destroy_view(alder_encode_number_t *o);

int alder_encode_number_test();
int alder_encode_number_is_same_byte(alder_encode_number_t *o);

#pragma mark print

int
alder_encode_number_to_bstringWithOffset(alder_encode_number_t *o,
                                         bstring bseq, int offset);
int
alder_encode_number_to_bstring(alder_encode_number_t *o, bstring bseq);

int alder_encode_number_log(alder_encode_number_t *o);
int alder_encode_number_buffer_log(uint8_t *buf, size_t sizeBuf, int kmersize);

int alder_encode_number_print_DNA_and_hash(alder_encode_number_t *o, FILE *fp);

int alder_encode_number_copy(alder_encode_number_t *dst,
                             alder_encode_number_t *src);

int alder_encode_number_compare(alder_encode_number_t *s1,
                                alder_encode_number_t *s2,
                                size_t d);
size_t
alder_encode_number_adjust_buffer_size_for_packed_kmer
(int kmer_size, size_t size_buf);

alder_encode_number_t *
alder_encode_number_create_rev(alder_encode_number_t *o);
__END_DECLS


#endif /* alder_encode_alder_encode_number_h */
