/**
 * This file, alder_hash_test.c, is part of alder-hash.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-hash is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hash.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_hash.h"

#define N 2
void
alder_hash_test_00()
{
    
    uint64_t n[N] = {1313141,314234};
    int len = 28;
    
    for (int i = 0; i < 1 << len; i++) {
        n[0]++;
        n[1]++;
        alder_hash_code01(n, N);
    }
   
    for (int i = 0; i < 1 << len; i++) {
        n[0]++;
        n[1]++;
        alder_hash_code02(n, N);
    }
   
   
//    printf("n: %llu\n", v);
}



