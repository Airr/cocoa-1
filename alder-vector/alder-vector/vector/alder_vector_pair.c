/**
 * This file, alder_vector_pair.c, is part of alder-vector.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-vector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-vector is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-vector.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "alder_vector_pair.h"

alder_vector_pair_t *
alder_vector_pair_t_alloc (const size_t n)
{
    alder_vector_pair_t * b;
    
    if (n == 0)
    {
        return 0;
    }
    
    b = (alder_vector_pair_t *) malloc (sizeof (alder_vector_pair_t));
    
    if (b == 0)
    {
        return 0;
    }
    
    b->data = (alder_block_pair_t *) calloc (1, 1 * n * sizeof (alder_block_pair_t));
    
    if (b->data == 0)
    {
        free (b);
        return 0;
    }
    
    b->size = n;
    
    return b;
}

void alder_vector_pair_t_free (alder_vector_pair_t * b)
{
    if (!b) {
        return;
    }
    free (b->data);
    free (b);
}

size_t alder_vector_pair_t_size (const alder_vector_pair_t * b)
{
    return b->size;
}

alder_block_pair_t * alder_vector_pair_t_data (const alder_vector_pair_t * b)
{
    return b->data;
}

