/**
 * This file, alder_gmp.c, is part of alder-gmp.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-gmp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-gmp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-gmp.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <gmp.h>
#include "alder_gmp.h"

void alder_gmp_test()
{
    mpz_t integ1, integ2;
    mpz_init2 (integ1, 54);
    mpz_init2 (integ2, 54);
    
    mpz_set_ui(integ1, 1);
    mpz_mul_2exp(integ1, integ1, 65);
    mpz_out_str (stdout, 2, integ1);
    printf("\n");
//    mpz_add_ui(integ1,integ1,1);
    
    mpz_out_str (stdout, 10, integ1); printf("\n");
//    mpz_mul_2exp(integ1, integ1, 2);
//    mpz_out_str (stdout, 2, integ1); printf("\n");
    
    uint64_t x2[2];
    x2[0] = 0; x2[1] = 0;
    uint64_t x = 0;
    size_t countp = 0;
    mpz_export(x2, &countp, -1, 8, 0, 0, integ1);
    printf("x[0]: %llu\n", x2[0]);
    printf("x[1]: %llu\n", x2[1]);
    x = UINT64_MAX;
    printf("x: %llu\n", x);
    
    mpz_set_ui(integ1,0);
    
    
//    mpz_mul_2exp(integ1, integ1, );
//    mpz_import(b, 1, 1, sizeof(a), 0, 0, &a);
    // mpz_export
    mpz_out_str (stdout, 2, integ1); printf("\n");
    
    
    mpz_out_str (stdout, 2, integ2);
    printf("\n");
    mpz_swap(integ1, integ2);
    mpz_mul_2exp(integ2, integ1, 2);
    
    mpz_setbit(integ2,0);
    mpz_setbit(integ2,1);
    mpz_out_str (stdout, 2, integ2);
    printf("\n");
    
    mpz_clear (integ1);
    mpz_clear (integ2);
    
}