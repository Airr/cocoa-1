/**
 * This file, alder_snippet_array.c, is part of alder-snippet.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-snippet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-snippet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-snippet.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_snippet_array.h"

static int
alder_snippet_array0();

int
alder_snippet_array()
{
    alder_snippet_array0();
    return 0;
}

static int compare_int(const void *a, const void *b)
{
//    return ( *(int*)a - *(int*)b );

//    const int *av = (const int*)a;
//    const int *bv = (const int*)b;
    const int *av = (const int*)a + 1;
    const int *bv = (const int*)b + 1;
    if (*av > *bv) {
        return 1;
    } else if (*av < *bv) {
        return -1;
    } else {
        return 0;
    }
}

static int
alder_snippet_array0()
{
    const int SIZE_I = 5;
    const int SIZE_J = 4;
    int **a = malloc(sizeof(*a) * SIZE_I);
    for (int i = 0; i < SIZE_I; i++) {
        a[i] = malloc(sizeof(**a) * SIZE_J);
    }
    
    srand(1);
    for (int i = 0; i < SIZE_I; i++) {
        for (int j = 0; j < SIZE_J; j++) {
            int x = rand() % 100;
            a[i][j] = x;
        }
    }
    
    for (int i = 0; i < SIZE_I; i++) {
        for (int j = 0; j < SIZE_J; j++) {
            fprintf(stdout, "%d ", a[i][j]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
    
    // #define MY_ARRAY_VALUE(index)   my_array[(index) / 3][(index) % 3]
    qsort(*a, SIZE_I * SIZE_J / 2, sizeof(int) * 2, compare_int);
    
    fprintf(stdout, "Sorting...\n");
    for (int i = 0; i < SIZE_I; i++) {
        for (int j = 0; j < SIZE_J; j++) {
            fprintf(stdout, "%d ", a[i][j]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");

    
    
    for (int i = 0; i < SIZE_I; i++) {
        XFREE(a[i]);
    }
    XFREE(a);
    return 0;
}
