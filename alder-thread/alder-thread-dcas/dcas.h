//******************************************************************
//	File name: dcas.h
//
//	Description:  
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//	8:19 PM 9/5/2008 - #DEF#16: Change the decleration of pointerDescriptor
//******************************************************************

#ifndef _TRUE_
#define _TRUE_
#define TRUE 1
#endif //_TRUE_

#ifndef _FALSE_
#define _FALSE_
#define FALSE 0
#endif //_FALSE_

#ifndef _POINTER_CONTAINER_el
#define _POINTER_CONTAINER_el

#define POINTER_CONTAINER_DSC 1
#define POINTER_CONTAINER_INT 0

#include <stdint.h>

//[ 8:20 PM 9/5/2008 - #DEF#16]
struct POINTER_DEL_LSB_el{
	uint64_t ptr	:	62;
	uint64_t del	:	2;
}; 

typedef struct POINTER_DEL_LSB_el POINTER_DEL_LSB;

struct POINTER_CONTAINER_el{
	union{
			POINTER_DEL_LSB ptrDel;
			uint64_t ptr;
		};
};

typedef struct POINTER_CONTAINER_el PointerContainer;
 
struct RDCSSDESCRIPTOR_el{
	PointerContainer a1;
	int	o1;
	PointerContainer a2;
	uint64_t o2;
	uint64_t n2;
};

typedef struct RDCSSDESCRIPTOR_el RDCSSDescriptor_t;

#endif // _POINTER_CONTAINER_el

#ifndef _DCAS_CONTAINER_el
#define _DCAS_CONTAINER_el
#define NUM_OF_ENTRIES 2

// DCASDescriptorState - applicable DCASDescriptor states
enum DCASDescriptorState_e{
	DCAS_UNDECIDED 		= 2,
	DCAS_SUCCEEDED 		= 4,
	DCAS_FAILED			= 6
} DCASDescriptorState;

// ENTRY_el of CASN record
struct ENTRY_el{
	PointerContainer* addr;
	uint64_t old;
	uint64_t new;
};

typedef struct ENTRY_el Entry;

// DCASDESCRIPTOR_el - struct that define the DCAS Desctriptor container
struct DCASDESCRIPTOR_el{
	uint64_t status;
	Entry entry[NUM_OF_ENTRIES];
};

typedef struct DCASDESCRIPTOR_el DCASDescriptor_t;

#endif // _DCAS_CONTAINER_el

#ifndef DCAS_DEF_H
#define DCAS_DEF_H
uint64_t Dcas(void** a1,void** a2,void* v1,void* v2,void* w1,void* w2);
uint64_t mDcas(uint64_t* a1,uint64_t* a2,uint64_t v1,uint64_t v2,uint64_t w1,uint64_t w2);
uint64_t DcasCallSym(uint64_t* a1,uint64_t* a2,uint64_t v1,uint64_t v2,uint64_t w1,uint64_t w2);
uint64_t RDCSS(RDCSSDescriptor_t *d);
uint64_t RDCSSRead (uint64_t* addr);
void Complete(RDCSSDescriptor_t *d);
RDCSSDescriptor_t* MallocRDCSSDescriptor();
void PrintRDCSSDescriptor(RDCSSDescriptor_t *ds);
uint64_t GetFackDescAddr( RDCSSDescriptor_t *ds);

// DCAS implementation according article "A practical Multi word compare and swap operation"
DCASDescriptor_t* CreateDCASDescriptor(uint64_t* addr1, uint64_t regOld1, uint64_t regNew1, uint64_t* addr2, uint64_t regOld2, uint64_t regNew2);
RDCSSDescriptor_t* CreateRDCSSDescriptor(uint64_t addr1, uint64_t regOld1, uint64_t addr2, uint64_t regOld2, uint64_t regNew2);
DCASDescriptor_t* MallocDCASDescriptor();
uint64_t CAS2 ( DCASDescriptor_t* cd); // TODO: Dotan.h - rename method into Dcas and current Dcas into DcasSim
DCASDescriptor_t* CAS2Copy ( DCASDescriptor_t* cd);
uint64_t DCASRead( uint64_t* addr);
void DCASReadAddr( uint64_t* addr, uint64_t* ret);
void PrintDCASDescriptor(DCASDescriptor_t* ds);
 
#endif // DCAS_DEF_H

