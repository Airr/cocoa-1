/**
 * This file, mytest1.c, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cas.h"
#include "dcas.h"
#include "pointerCont.h"
#include "mytest1.h"

static int test_asm_cas_succ();
static int test_fullDCASTestSucc();
static int test_fullDCASReadSucc();

void mytest1()
{
    if (test_asm_cas_succ()) {
        printf("PASS: test_asm_cas_succ\n");
    }
    if (test_fullDCASTestSucc()) {
        printf("PASS: test_fullDCASTestSucc\n");
    }
    if (test_fullDCASReadSucc()) {
        printf("PASS: test_fullDCASReadSucc\n");
    }
}

static int test_asm_cas_succ()
{
	int rc = FALSE;
	uint64_t *n1;
	n1 = malloc(sizeof(uint64_t));
	*n1 = 3;
	int res;
    
	printf("before call n1 is %llu\n", *n1);
	res = Cas((void**) n1,(void*) 1,(void*) 2);
	printf("after call n1 is %llu\n", *n1);
    
	if (*n1 == 2) {
		rc = TRUE;
        assert(res == TRUE);
    } else {
        assert(res == FALSE);
    }
    
    free(n1);
	return rc;
}

static int test_fullDCASTestSucc()
{
	int ans;
	uint64_t a = StoreInt(5);
	uint64_t b = StoreInt(6);
    
	DCASDescriptor_t* ds = CreateDCASDescriptor(&a, StoreInt(5), StoreInt(6),
                                                &b, StoreInt(6), StoreInt(8));
	PrintDCASDescriptor(ds);
    
	CAS2(ds);
    
	PrintDCASDescriptor(ds);
    
	ans = (((*((uint64_t*)(ds->entry[0].addr->ptr))) == StoreInt(6)) &
           ((*((uint64_t*)(ds->entry[1].addr->ptr))) == StoreInt(8)));
    
	return ans;
}

static int test_fullDCASReadSucc()
{
	int ans;
	uint64_t a = StoreInt(5);
	uint64_t b = StoreInt(6);
    
	DCASDescriptor_t* ds = CreateDCASDescriptor(&a, StoreInt(5), StoreInt(6),
                                                &b, StoreInt(6), StoreInt(8));
	//PrintDCASDescriptor(ds);
    
	a = CreateFakeDCASDesc((uint64_t)ds);
    
	//CAS2(ds);
    
	DCASRead(&a);
    
	//PrintDCASDescriptor(ds);
    
	ans = (((*((uint64_t*)(ds->entry[0].addr->ptr))) == StoreInt(6)) &
           ((*((uint64_t*)(ds->entry[1].addr->ptr))) == StoreInt(8)));
    
	return ans;
}

