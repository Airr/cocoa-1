//******************************************************************
//	File name: cas.c
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//******************************************************************

#include <stdio.h> 
#include <stdlib.h>
#include <libkern/OSAtomic.h>
#include <stdbool.h>
#include <stdint.h>
#include "cas.h"

uint64_t casAsm(volatile uint64_t *ptr, uint64_t oldVal, uint64_t newVal)
{
	if (DEBUG_LEV == 1)
			printf("before swap: *ptr=%llx , old=%llx, new=%llx \n",*ptr,oldVal, newVal);

    return __sync_val_compare_and_swap(ptr, oldVal, newVal);

	if (DEBUG_LEV == 1)
			printf("after swap: *ptr=%llx , old=%llx, new=%llx \n",*ptr,oldVal,newVal);
}
 
// Cas Function - Compare and swap calls Function casAsm
// Return int - True(1) - cas operation succeeded else False(0)
int Cas(void** a1, void* v1, void* w1)
{
	//cas((volatile uint64_t *) &n1,(uint64_t) n1,(uint64_t) n2)
	return (casAsm((volatile uint64_t *)a1,(uint64_t)v1,(uint64_t)w1) == (uint64_t)v1);
	
}




 
