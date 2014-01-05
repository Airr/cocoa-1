/**
 * This file, alder_type_sundell.h, is part of alder-type.
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

#ifndef alder_type_alder_type_sundell_h
#define alder_type_alder_type_sundell_h

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

/*
 * |<-----------------value(63)-------------->|1|
 * |1|<-index(10)->|<------descriptor(53)------>|
 */
typedef uint64_t word_t;

typedef struct alder_type_sundell_entry_struct {
    word_t *address;
    word_t old;
    word_t new;
} alder_type_sundell_entry_t;

typedef struct alder_type_sundell_descriptor_struct {
    int mm_ref;
    word_t status;
    int thread;
    int seq;
    int size;
    alder_type_sundell_entry_t *words;
    struct alder_type_sundell_descriptor_struct *mm_next;
} alder_type_sundell_descriptor_t;

#define STATUS_SUCCESS 0
#define STATUS_FAILED  1

#pragma mark macro

bool alder_type_sundell_IS_REFERENCE(word_t x);

alder_type_sundell_descriptor_t *
alder_type_sundell_GET_DESCRIPTOR(word_t x);

word_t alder_type_sundell_MAKE_REF
(alder_type_sundell_descriptor_t *descriptor, int idx);

word_t alder_type_sundell_MAKE_DESCRIPTOR
(alder_type_sundell_descriptor_t *descriptor);

word_t alder_type_sundell_MAKE_STATUS_GIVE_DESCRIPTOR
(alder_type_sundell_descriptor_t *descriptor);
word_t alder_type_sundell_MAKE_STATUS_TRYING_DESCRIPTOR
(alder_type_sundell_descriptor_t *descriptor);

word_t alder_type_sundell_GET_INDEX(word_t x);
word_t alder_type_sundell_GET_VALUE(word_t value);
word_t alder_type_sundell_MAKE_VALUE(word_t value);
word_t alder_type_sundell_ADD_AVOIDLIST(word_t y, int x);
word_t alder_type_sundell_IS_EMPTY_AVOIDLIST(word_t x);
int alder_type_sundell_IS_IN_AVOIDLIST(int x, word_t X);

// STATUS_TRYING<0> = 3 or status*2 + 3.
// STATUS_GIVE<0> = 2 or status*2 + 2.
int alder_type_sundell_STATUS_TRYING_N(int N);
int alder_type_sundell_STATUS_GIVE_N(int N);
// STATUS_TRYING 3, 5, 7, ... status > 1 && status%2 == 1
// STATUS_GIVE   2, 4, 6, ... status > 1 && status%2 == 0
#define IS_STATUS_TRYING(s) ((s)>1 && (s)%2==1)
#define IS_STATUS_GIVE(s) ((s)>1 && (s)%2==0)

int alder_type_sundell();

__END_DECLS


#endif /* alder_type_alder_type_sundell_h */
