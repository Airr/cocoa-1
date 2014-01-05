/**
 * This file, alder_bdwgc.c, is part of alder-gc.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-gc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-gc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-gc.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <gc.h>
#include "alder_bdwgc.h"

int alder_bdwgc()
{
    int i;
    
    GC_INIT();	/* Optional on Linux/X86; see below.  */
    for (i = 0; i < 10000000; ++i)
    {
        int **p = (int **) GC_MALLOC(sizeof(int *));
        int *q = (int *) GC_MALLOC_ATOMIC(sizeof(int));
        assert(*p == 0);
        *p = (int *) GC_REALLOC(q, 2 * sizeof(int));
        if (i % 100000 == 0)
            printf("Heap size = %zd\n", GC_get_heap_size());
    }
    
    return 0;
}