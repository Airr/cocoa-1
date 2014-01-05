/**
 * This file, alder_bit.h, is part of alder-bit.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-bit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-bit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-bit.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_bit_alder_bit_h
#define alder_bit_alder_bit_h

#include <stddef.h>
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

typedef uint64_t kmer_t;

int alder_bit_acgt2int(char c);
char alder_bit_int2acgt(int i);
uint8_t alder_bit_four2uc(int i1, int i2, int i3, int i4);
void alder_bit_uc2four(uint8_t uc, uint8_t *i1, uint8_t *i2, uint8_t *i3, uint8_t *i4);
kmer_t alder_bit_kmer2number(uint8_t *kmer, size_t size);
kmer_t alder_bit_kmerpos2number(char c, int pos);
void alder_bit_number2kmer(bstring bkmer, uint64_t n, int k);
int alder_bit_highest_bit_1(uint64_t x);
uint64_t alder_bit_all_1_with_left_zero_of(int n);
kmer_t alder_bit_hash(kmer_t x);

__END_DECLS

#endif /* alder_bit_alder_bit_h */
