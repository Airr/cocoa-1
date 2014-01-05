/**
 * This file, alder_wordtable_bits.h, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_wordtable_alder_wordtable_bits_h
#define alder_wordtable_alder_wordtable_bits_h

#include <stdint.h>
#include "bstrlib.h"
#include "alder_wordtable_kfmdefaults.h"

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

size_t alder_wordtable_bits_bitSizeOfWord(alder_wordtable_t x);

size_t alder_wordtable_bits_log2ceil(size_t x);

alder_wordtable_t alder_wordtable_bits_intMaskLo(size_t bits);

int alder_wordtable_bits_shiftRight(alder_wordtable_t *arr,
                                    size_t arrLength,
                                    int rightshift,
                                    int insert);
int alder_wordtable_bits_shiftRight2(alder_wordtable_t *arr,
                                     size_t length,
                                     int rightshift,
                                     int insert);

int alder_wordtable_bits_shiftLeft(alder_wordtable_t *arr,
                                   size_t arrLength,
                                   int leftshift,
                                   int insert);

int alder_wordtable_bits_shiftLeft2(alder_wordtable_t *arr,
                                    size_t pos,
                                    size_t arrLength,
                                    int leftshift,
                                    int insert);

int alder_wordtable_bits_copyShiftLeft(alder_wordtable_t *from, size_t fromPos,
                                       alder_wordtable_t *to, size_t toPos,
                                       size_t len,
                                       int leftshift,
                                       int insert);


alder_wordtable_t alder_wordtable_bits_MAX_SAFE_INT();

size_t alder_wordtable_bits_arrayLength(size_t x);

size_t alder_wordtable_bits_longBitCount(uint64_t x);


int alder_wordtable_bits_bit(alder_wordtable_t *arr,
                             size_t arrLength,
                             size_t pos);

bstring
alder_wordtable_bits_asString(alder_wordtable_t *arr, size_t arrLength);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_bits_h */
