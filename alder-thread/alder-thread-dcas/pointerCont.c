//******************************************************************
//	File name: pointerCont.c
//
//	Description: Contains the pointerContainer structor and it's functionallity
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//	18.Oct.2008 - #DEF#20 : (Dotan.H) Fix Segmentaion fault in DCASBased.
//******************************************************************

#include <stdio.h>
#include <stdlib.h>

#include "pointerCont.h"
#include "dcas.h"

#define ALL_ZERO_LSB_ONE        0x0000000000000001
#define ALL_ONE_LSB_ZERO        0xfffffffffffffffe

#define ALL_ZERO_TWO_LSB_ONE    0x0000000000000003
#define ALL_ONE_TWO_LSB_ZERO    0xFFFFFFFFFFFFFFFC

#define RDCSS_DESC_BIT_PATTERN 	0x0000000000000001
#define DCAS_DESC_BIT_PATTERN 	0x0000000000000002
#define DELETE_BIT_PATTERN      0x0000000000000001


uint64_t IsDcasDescriptor(uint64_t addr)
{
	return (addr & ALL_ZERO_TWO_LSB_ONE) == DCAS_DESC_BIT_PATTERN;
}

uint64_t IsRDCSSDescriptor(uint64_t addr)
{
	return (addr & ALL_ZERO_TWO_LSB_ONE) == RDCSS_DESC_BIT_PATTERN;
}

uint64_t IsDeletedNode(uint64_t addr)
{
	return (addr & ALL_ZERO_TWO_LSB_ONE) == DELETE_BIT_PATTERN;
}

// IsDescriptor is an implementation of pointer without the LSB
/*
int IsDescriptor(PointerContainer pc)
{
	int ans = (CheckLSB(pc.ptr) == TRUE);
	return ans;
}

// IsDescriptorInt checks if the address that was given was with 1 on the LSB
int IsDescriptorInt(int add)
{
	//printf("IsDescriptorInt %d \n", add);
	int ans = (CheckLSB(add) == TRUE);
	return ans;
}
*/

// MallocInt() create a int* and checks if the pointer is validate (can return NULL)
uint64_t* MallocInt()
{
	uint64_t* ans = malloc(sizeof(uint64_t));
	ans = (uint64_t*) ValidateMem( (uint64_t) ans);
	return ans;
}

// ValidateMem method checks if the address of the pointer's lsb is 1 if so it return null else return the current address
uint64_t* ValidateMem(uint64_t addr)
{
	uint64_t ans = addr;
	if (IsRDCSSDescriptor(addr)== TRUE)
	{
		printf("Cannot allocate memory\n");
		ans = FALSE;
	}
	return (uint64_t*)ans;
}

//MallocPointCont() - Malloc a PointerContatiner (LSB sould not be 1) - Can return NULL
PointerContainer* MallocPointCont()
{
	PointerContainer* ans = malloc(sizeof(PointerContainer));
	ans = (PointerContainer* ) ValidateMem( (uint64_t) ans);
	
	return ans;
}

// MallocPInt() create a PointerContainer struct and checks if the pointer is validate (can return NULL)
// also will create a Int within;
PointerContainer* MallocPInt()
{
	PointerContainer* ans = malloc(sizeof(PointerContainer));
	ans = (PointerContainer* ) ValidateMem( (uint64_t) ans);
	// check if the pointerContainer have no 1 on the LSB - if so halt and retunr NULL
	if (ans != FALSE)
	{
		uint64_t ptrAddr = (uint64_t) MallocInt();
		// if the 
		if (ptrAddr == FALSE)
			ans = FALSE;
		else
		{
			ans->ptr = ptrAddr;
			ans->ptrDel.del = POINTER_CONTAINER_INT;
		}
	}
	return ans;
}
// MallocPDescriptor() create a PointerContainer struct and checks if the pointer is validate (can return NULL)
// also will create a PointerContainer within;
PointerContainer* MallocPRDCSSDescriptor()
{
	PointerContainer* ans = malloc(sizeof(PointerContainer));
	ans = (PointerContainer* ) ValidateMem( (uint64_t) ans);
	// check if the pointerContainer have no 1 on the LSB - if so halt and retunr NULL
	if (ans != FALSE)
	{
		uint64_t ptrAddr = (uint64_t) MallocRDCSSDescriptor();
		// if the 
		if (ptrAddr == FALSE)
			ans = FALSE;
		else
		{
			ans->ptr = ptrAddr;
			ans->ptrDel.del = POINTER_CONTAINER_RDSCC_DSC;
		}
	}
	
	return ans;
}

PointerContainer* MallocPDCASSDescriptor()
{
	PointerContainer* ans = malloc(sizeof(PointerContainer));
	ans = (PointerContainer* ) ValidateMem( (uint64_t) ans);
	// check if the pointerContainer have no 1 on the LSB - if so halt and retunr NULL
	if (ans != FALSE)
	{
		uint64_t ptrAddr = (uint64_t) MallocDCASDescriptor();
		if (ptrAddr == FALSE)
			ans = FALSE;
		else
		{
			ans->ptr = ptrAddr;
			ans->ptrDel.del = POINTER_CONTAINER_DCAS_DSC;
		}
	}
	
	return ans;
}

// check the LSB of any int field
/*
int CheckLSB(int addr)
{
	int ans = -1;
	ans = addr & 00000001 ;
	return ans;
}
*/


// creats an node structure and return it to caller.
node* MallocNode()
{
	node* ans = malloc(sizeof(node));
	ans = (node*) ValidateMem( (uint64_t) ans);
	return ans;
}

// Allow to malloc a pointercontainer pointer and also allocate a node structure.
PointerContainer* MallocPNode()
{
PointerContainer* ans = malloc(sizeof(PointerContainer));
	ans = (PointerContainer* ) ValidateMem( (uint64_t) ans);
	// check if the pointerContainer have no 1 on the LSB - if so halt and retunr NULL
	if (ans != FALSE)
	{
		uint64_t ptrAddr = (uint64_t) MallocNode();
		// if the 
		if (ptrAddr == FALSE)
			ans = FALSE;
		else
		{
			ans->ptr = ptrAddr; //18.Oct.2008 - #DEF#20 : (Dotan.H) 
			ans->ptrDel.del = 0;
		}
	}
	 
	return ans; 
}

// Retrieve real memory adress as int (will ask for cast afterwards)
uint64_t GetRealAddr(uint64_t addr)
{
	return (addr & ALL_ONE_TWO_LSB_ZERO);
}

// Inserte a bit on the LSB according to the BIT indication
/*int PutLSB(int source, int bit)
{
	int ans; 
	if (bit == 0)
		ans = source & ALL_ONE_LSB_ZERO; //0xfffffffe
	else
		ans = source | ALL_ZERO_LSB_ONE; //0x00000001
	return ans;
}*/

uint64_t CreateFakeRDCSSDesc(uint64_t addr)
{
	return (addr | RDCSS_DESC_BIT_PATTERN);
}
uint64_t CreateFakeDCASDesc(uint64_t addr)
{
	return (addr | DCAS_DESC_BIT_PATTERN);
}

uint64_t StoreInt(uint64_t num)
{
	return (num << 2);
}

uint64_t RetInt(uint64_t num)
{
	return (num >> 2);
}
