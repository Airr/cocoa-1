/**
 * This file, alder_wordtable_compute.c, is part of alder-wordtable.
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

#include <stdlib.h>
#include <assert.h>
#include "alder_wordtable_compute.h"

/* This function computes upto how many elements could be given to each group
 * given the total number of elements and the number of groups. 
 * This is equivalent to saying that it computes the number of groups 
 * given the total number of elements and the number of elements per group.
 * I like the second description better.
 */
size_t
alder_wordtable_compute_groupmaxsize(size_t total, size_t number_per_group)
{
    assert(number_per_group > 0);
    if (total == 0) {
        return 0;
    } else {
        return 1 + (total-1)/number_per_group;
    }
}

