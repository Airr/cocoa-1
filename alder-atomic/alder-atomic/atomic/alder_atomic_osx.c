/**
 * This file, alder_atomic_osx.c, is part of alder-atomic.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-atomic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-atomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-atomic.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdbool.h>
#include "alder_type.h"
#include "alder_atomic_osx.h"

bool alder_atomic_bool_CASint(int *ptr, int oldval, int newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

/* This function atomically add value to the value pointed by ptr.
 */
void alder_atomic_FAAint(int *ptr, int value)
{
    __sync_fetch_and_add(ptr, value);
}

bool alder_atomic_CASstatus(word_t *ptr, word_t oldval, word_t newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

bool alder_atomic_CASword(word_t *ptr, word_t oldval, word_t newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

bool
alder_atomic_CASdescriptor(alder_type_sundell_descriptor_t **ptr,
                           alder_type_sundell_descriptor_t *oldval,
                           alder_type_sundell_descriptor_t *newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

alder_type_sundell_descriptor_t*
alder_atomic_SWAPdescriptor(alder_type_sundell_descriptor_t **ptr,
                            alder_type_sundell_descriptor_t *value)
{
    return __sync_lock_test_and_set(ptr, value);
}

word_t alder_atomic_SWAPword(word_t *ptr, word_t value)
{
    return __sync_lock_test_and_set(ptr, value);
}
