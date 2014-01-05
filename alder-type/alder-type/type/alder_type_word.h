/**
 * This file, alder_type_word.h, is part of alder-type.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-type is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-type is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-type.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_type_alder_type_word_h
#define alder_type_alder_type_word_h

#include <stdint.h>
#include <stdbool.h>

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


#define ALL_ZERO_MSB_ONE        0x1000000000000000
#define ALL_ONE_MSB_ZERO        0x7fffffffffffffff
#define ALL_ZERO_TWO_MSB_ONE    0xC000000000000000
#define ALL_ONE_TWO_MSB_ZERO    0x3FFFFFFFFFFFFFFF
#define VALUE_BIT_PATTERN       0x4000000000000000

#define REFERENCE_BIT_PATTERN   0x8000000000000000

/*
 * |<-----------------value------------------->|1|
 * |<--index-->|<--thread-->|<--descriptor-->|1|1|
 */
//typedef struct {
//    union {
//        struct {
//            uint64_t value:63;
//            uint64_t word_type:1;
//        };
//        struct {
//            uint64_t index:21;
//            uint64_t thread:21;
//            uint64_t descriptor:20;
//            uint64_t ref_type:1;
//            uint64_t word_type_dummy:1;
//        };
//    };
//} word_t;

typedef uint64_t word_t;

#define STATUS_SUCCESS 0
#define STATUS_FAILED  1

// 
// STATUS_TRYING 3, 5, 7, ... status > 1 && status%2 == 1
// STATUS_GIVE   2, 4, 6, ... status > 1 && status%2 == 0
// STATUS_TRYING<0> = 3 or status*2 + 3.
// STATUS_GIVE<0> = 2 or status*2 + 2.
#define IS_STATUS_TRYING(s) (s>1 && s%2==1)
#define IS_STATUS_GIVE(s) (s>1 && s%2==0)

typedef struct {
    int refcount;
    word_t status;
    int thread;
    int seq;
    int size;
    struct {
        word_t *address;
        word_t old;
        word_t new;
    } words[2];
} descriptor_t;

int alder_type_word();

word_t Read(word_t *address);
bool CASN(int N, word_t **address, word_t *oldvalue, word_t *newvalue);

word_t WFRead(word_t *address, descriptor_t **desc);
void HelpCompareAndSwap(descriptor_t *desc, uint64_t avoidList);
void HelpGiveAway(descriptor_t *descTo, descriptor_t *descFrom);

__END_DECLS


#endif /* alder_type_alder_type_word_h */
