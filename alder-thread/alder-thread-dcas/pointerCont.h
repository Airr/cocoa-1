//******************************************************************
//	File name: pointerCont.h
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
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

#include <stdint.h>

#define POINTER_CONTAINER_DELETED_NODE 	3
#define POINTER_CONTAINER_DCAS_DSC 		2
#define POINTER_CONTAINER_RDSCC_DSC 	1
#define POINTER_CONTAINER_INT 			0

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
	uint64_t	o1;
	PointerContainer a2;
	uint64_t o2;
	uint64_t n2;
};

typedef struct RDCSSDESCRIPTOR_el RDCSSDescriptor_t;

#endif // _POINTER_CONTAINER_el

#ifndef _NODE_CONTAINER_el
#define _NODE_CONTAINER_el
struct NODE_CONTAINER_el{
	PointerContainer* L;
	PointerContainer* R;
	uint64_t value;
};

typedef struct NODE_CONTAINER_el node;
#endif // _NODE_CONTAINER_el

#ifndef POINTER_CONTAINER_DEF_H
#define POINTER_CONTAINER_DEF_H

// Functions that added after the change of PointerContainer into 2 bits -start
	uint64_t IsDcasDescriptor(uint64_t addr);
	uint64_t IsRDCSSDescriptor(uint64_t addr);
	uint64_t IsDeletedNode(uint64_t addr);
	uint64_t CreateFakeRDCSSDesc(uint64_t addr);
	uint64_t CreateFakeDCASDesc(uint64_t addr);
// Functions that added after the change of PointerContainer into 2 bits -end

uint64_t* MallocInt();
uint64_t* ValidateMem(uint64_t addr);
//int IsDescriptor(PointerContainer pc);
//int IsDescriptorInt(int add);

PointerContainer* MallocPointCont();
PointerContainer* MallocPInt();
PointerContainer* MallocPRDCSSDescriptor();
node* MallocNode();


PointerContainer* MallocPNode();
//int CheckLSB(int addr);
uint64_t GetRealAddr(uint64_t addr);
//int PutLSB(int source, int bit);

uint64_t StoreInt(uint64_t num);
uint64_t RetInt(uint64_t num);

#endif // POINTER_CONTAINER_DEF_H
