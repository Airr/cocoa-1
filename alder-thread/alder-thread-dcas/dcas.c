//******************************************************************
//	File name: dcas.c
//
//	Description:
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//	11.Oct.2008	- #DEF#18 : (Dotan.H) using a loked version of the dcas implementation.
//******************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "pointerCont.h"
#include "dcas.h"
#include "cas.h"

uint64_t print_rdcss = FALSE;
// Create A mutex to Dcas Function
pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Create a mutex for mDcas function
pthread_mutex_t dcas_mutex = PTHREAD_MUTEX_INITIALIZER;

// Implement new dcas after meeting with prof.Uri:
//int lockDcas( void* a1, void* a2, void v1, void v2, void n1, void n2)
//{
// check the a1 & a2 memory order

// try to block the first address

// try to block the second address

// free both of the addresses and return true/ false
//}

// Dummy Dcas Function Success oriented
// DCAS implementation using mutex.
// mDcas function guarantees the direction of dcas call.
uint64_t mDcas(uint64_t* a1,uint64_t* a2,uint64_t v1,uint64_t v2,uint64_t w1,uint64_t w2)
{
	uint64_t ans = FALSE;
	if (a1 < a2)
		ans = DcasCallSym(a1, a2, v1, v2, w1, w2);
	else
		ans = DcasCallSym(a2, a1, v2, v1, w2, w1);
	return ans;
}

uint64_t DcasCallSym(uint64_t* a1,uint64_t* a2,uint64_t v1,uint64_t v2,uint64_t w1,uint64_t w2)
{
	pthread_mutex_lock (&dcas_mutex);
	uint64_t ans = FALSE;
	if((*a1 == v1) && (*a2 == v2))
	{
		*a1=w1;
		*a2=w2;
		ans = TRUE;
	}
	pthread_mutex_unlock(&dcas_mutex);
    
	return ans;
}

// Real Dcas call implementation (which will create an discriptor)
uint64_t Dcas(void** a1,void** a2,void* v1,void* v2,void* w1,void* w2)
{
	//printf("Entering into DCAS function\n");
    
	uint64_t ans = FALSE;
    
	if (*a1 == 0)
		printf("Problem!!! *a1 == 0\n");
    
	pthread_mutex_lock (&job_queue_mutex);
	if (*a1 < *a2)
	{
		ans = CAS2(CreateDCASDescriptor((uint64_t*) a1, (uint64_t) v1, (uint64_t) w1, (uint64_t*) a2, (uint64_t) v2, (uint64_t) w2));
		//ans =  DcasCallSym((void**) a1, (void*) v1, (void*) w1, (void**) a2, (void*) v2, (void*) w2);
	}
	else
	{
		ans = CAS2(CreateDCASDescriptor((uint64_t*) a2, (uint64_t) v2, (uint64_t) w2, (uint64_t*) a1, (uint64_t) v1, (uint64_t) w1));
		//ans = DcasCallSym((void**) a2, (void*) v2, (void*) w2, (void**) a1, (void*) v1, (void*) w1);
        
	}
	//printf("Exiting from DCAS function\n");
	pthread_mutex_unlock (&job_queue_mutex);
	return ans;
}

uint64_t DCASRead( uint64_t* addr)
{
	uint64_t cell;
	do{
		cell = RDCSSRead( addr );
		//printf("\t\t\tcell:%x\n",*addr);
		if ( IsDcasDescriptor( cell ))
		{
			//printf("\t\t\t\t\t\t\DA\n");
			CAS2( (DCASDescriptor_t*) GetRealAddr(cell));
		}
	} while (IsDcasDescriptor (cell));
	return cell;
}

void DCASReadAddr( uint64_t* addr, uint64_t* ret)
{
	uint64_t cell;
	do{
		cell = RDCSSRead( addr );
		//printf("\t\t\tcell:%x\n",*addr);
		if ( IsDcasDescriptor( cell ))
		{
			//printf("\t\t\t\t\t\t\DA\n");
			CAS2( (DCASDescriptor_t*) GetRealAddr(cell));
		}
	} while (IsDcasDescriptor (cell));
	//printf("cell is:%x\n", cell);
	*ret = cell;
}

// Dcas implementation according to the article "A practical Multi word compare and swap operation"

/*int GetFackDescAddr( RDCSSDescriptor_t *ds)
 {
 int ans = PutLSB( (int) ds, 1 );
 return ans;
 }*/

// MallocDescriptor() create a RDCSSDescriptor_t struct and checks if the pointer is validate (can return NULL)
RDCSSDescriptor_t* MallocRDCSSDescriptor()
{
	RDCSSDescriptor_t* ans = malloc(sizeof(RDCSSDescriptor_t));
	ans = (RDCSSDescriptor_t*) ValidateMem( (uint64_t) ans);
	return ans;
}

// PrintDescriptor - print the content of a descriptor
void PrintRDCSSDescriptor(RDCSSDescriptor_t* ds)
{
	printf("Descriptor in adress: \t%p\n", ds);
	printf("\t\t*a1: \t%llx\n", *(uint64_t*)(ds->a1.ptr));
	printf("\t\t o1: \t%llu\n", ds->o1);
	printf("\t\t*a2: \t%llx\n", *(uint64_t*)(ds->a2.ptr));
	printf("\t\t o2: \t%lld\n", ds->o2);
	printf("\t\t n2: \t%lld\n", ds->n2);
	return;
}

// RDCSS implement double compare single swap
// Parameters:  RDCSSDescriptor_t *d should contain adress to a descriptor
uint64_t RDCSS(RDCSSDescriptor_t *d)
{
	//PrintDescriptor(d);
	uint64_t cell;
    
	//[D&T - 20-8-2008] int cell = Cas((void**) (int*)(d->a2.ptr),(void*) d->o2,(void*) GetFackDescAddr(d));
	do
	{
		//if (print_rdcss) printf("*");
		cell = casAsm((volatile uint64_t*)(d->a2.ptr)
                      ,(uint64_t) d->o2,
                      (uint64_t) CreateFakeRDCSSDesc((uint64_t)d));
		if (IsRDCSSDescriptor(cell))
		{
			// We send the cell as a real address pointer
			Complete((RDCSSDescriptor_t*) GetRealAddr(cell));
		}
	} while (IsRDCSSDescriptor(cell));
	//printf("test test : \n");
	if (cell == d->o2)
	{
		Complete(d);
	}
	return cell;
}

// Read the value on the given Address (which can point to  descriptor or real value)
// Parameters:  PointerContainer* addr should contain the address in the system (int or descriptor)
uint64_t RDCSSRead (uint64_t* addr)
{
	uint64_t cell;
    
	do
	{
		cell = *addr;
		//printf("cell:%x\n",cell);
		// Checks if the pointer container is descriptor
		if (IsRDCSSDescriptor(cell))
		{
			//printf("\t\t\t\tfound RDCSS:%x\n",cell);
			//printf("\t\tenter....\n");
			// Send the Real Adress of Descriptor (NOTE: fill the 31-32 bits with 0 LSB)
			Complete((RDCSSDescriptor_t*) GetRealAddr(cell) );
		}
        // loop until not getting descriptor on the addr
	}while(IsRDCSSDescriptor(cell));
	return cell;
}

void Complete(RDCSSDescriptor_t *d)
{
    
	uint64_t v = *((uint64_t*)(d->a1.ptr));
	// Create a new pointer (fake descriptor pointer) of d
	uint64_t fakeDescriptorPointer = CreateFakeRDCSSDesc((uint64_t)d);
	if (v == d->o1)
		//Cas((void**) (int*)(d->a2.ptr),(void*) fakeDescriptorPointer,(void*) d->n2);
		Cas((void**) (d->a2.ptr),(void*) fakeDescriptorPointer,(void*) d->n2);
	else
		//Cas((void**) (int*)(d->a2.ptr),(void*) fakeDescriptorPointer,(void*) d->o2);
		Cas((void**) (d->a2.ptr),(void*) fakeDescriptorPointer,(void*) d->o2);
    
	//PrintDescriptor(d); // TODO: (Dotan.H) Remove.
	return;
}



// DCAS implements double compare and swap operation (can return NULL pointer)
DCASDescriptor_t* CreateDCASDescriptor(uint64_t* addr1,
                                       uint64_t regOld1, uint64_t regNew1,
                                       uint64_t* addr2,
                                       uint64_t regOld2, uint64_t regNew2)
{
	DCASDescriptor_t* ans = MallocDCASDescriptor();
	if (ans)
	{
		ans->status = DCAS_UNDECIDED;
		ans->entry[0].addr = malloc(sizeof(PointerContainer));
        
		// TODO: Dotan - Heres a memory leak (Done.. delete comment)
		ans->entry[0].addr->ptr = (uint64_t)addr1;
		ans->entry[0].old = regOld1;
		ans->entry[0].new = regNew1;
        
		ans->entry[1].addr = malloc(sizeof(PointerContainer));
        
		// TODO: Dotan - Heres a memory leak (Done.. delete comment)
		ans->entry[1].addr->ptr = (uint64_t)addr2;
		ans->entry[1].old = regOld2;
		ans->entry[1].new = regNew2;
	}
	return ans;
}

// MallocDCASDescriptor() create a DCASDescriptor_t struct and checks if the pointer is validate (can return NULL)
DCASDescriptor_t* MallocDCASDescriptor()
{
	DCASDescriptor_t* ans = malloc(sizeof(DCASDescriptor_t));
	ans = (DCASDescriptor_t*) ValidateMem( (uint64_t) ans);
	return ans;
}

void PrintDCASDescriptor(DCASDescriptor_t* ds)
{
	printf("DCAS Descriptor in adress: \t%p\n", ds);
	printf("DCAS Status is: ");
	switch(ds->status){
        case DCAS_UNDECIDED:
            printf("Undecided\n");
            break;
        case DCAS_SUCCEEDED:
            printf("Succeeded\n");
            break;
        case DCAS_FAILED:
            printf("Failed\n");
            break;
        default:
			printf("Unknown\n");
	}
    
	printf("\t\t*a1: \t%lld\n", *(uint64_t*)(ds->entry[0].addr->ptr));
	printf("\t\t o1: \t%lld\n", ds->entry[0].old);
	printf("\t\t n1: \t%lld\n", ds->entry[0].new);
	printf("\t\t*a2: \t%lld\n", *(uint64_t*)(ds->entry[1].addr->ptr));
	printf("\t\t o2: \t%lld\n", ds->entry[1].old);
	printf("\t\t n2: \t%lld\n", ds->entry[1].new);
	return;
}

// Create a new (allocate) rdcss descriptor
RDCSSDescriptor_t* CreateRDCSSDescriptor(uint64_t addr1, uint64_t regOld1, uint64_t addr2, uint64_t regOld2, uint64_t regNew2)
{
    
	RDCSSDescriptor_t* ans = MallocRDCSSDescriptor();
	ans->a1.ptr = addr1;
	ans->o1		= regOld1;
	ans->a2.ptr = addr2;
	ans->o2		= regOld2;
	ans->n2		= regNew2;
    
	return ans;
    
}

DCASDescriptor_t* CAS2Copy ( DCASDescriptor_t* cd)
{
	DCASDescriptor_t* copy_cd = CreateDCASDescriptor(
                                                     (uint64_t*)cd->entry[0].addr->ptr,
                                                     cd->entry[0].old,
                                                     cd->entry[0].new,
                                                     (uint64_t*)cd->entry[1].addr->ptr,
                                                     cd->entry[1].old,
                                                     cd->entry[1].new);
    
	//PrintDCASDescriptor(cd);
	//PrintDCASDescriptor(copy_cd);
    
	return copy_cd;
}

uint64_t CAS2 (DCASDescriptor_t* cd)
{
	uint64_t status = DCAS_FAILED;
	uint64_t succ;
	uint64_t val;
	uint64_t i = 0;
    
    //	RDCSSDescriptor_t* rdcssDesc = 0;
	Entry entry;
	//PrintDCASDescriptor(cd);
	// If dcasContStatus is valid (not NULL) and dcasDesctiptor is ready for enter
	if (cd -> status == DCAS_UNDECIDED)
	{
		status = DCAS_SUCCEEDED;
        
		for (i = 0; (i < 2) && (status == DCAS_SUCCEEDED); i++)
		{
        retry_entry:
            entry = cd -> entry[i];
            
            val = RDCSS(CreateRDCSSDescriptor((uint64_t)&(cd -> status), DCAS_UNDECIDED,
                                              entry.addr->ptr, entry.old, CreateFakeDCASDesc((uint64_t)cd) ));
            
            //printf("test : %d\n", i);
            if (IsDcasDescriptor(val))
            {
                if (GetRealAddr((uint64_t)val) != (uint64_t)cd)
                {
                    CAS2( (DCASDescriptor_t*) GetRealAddr((uint64_t)val) );
                    goto retry_entry;
                }
            }
            else
            {
                if (GetRealAddr((uint64_t)val) != entry.old)
                {
                    status = DCAS_FAILED;
                }
            }
            //printf("test : %d\n", i);
		}
		Cas( (void**)&(cd->status), (void*)DCAS_UNDECIDED,(void*) status);
	}
	// phase 2
	succ = ( cd->status == DCAS_SUCCEEDED );
	for (i = 0; i < 2; i++)
	{
		Cas((void**) cd-> entry[i].addr->ptr, (void*)CreateFakeDCASDesc((uint64_t)cd), (void*) (succ ? (cd->entry[i].new) : (cd->entry[i].old)));
	}
	//printf("test5 \n");
	//PrintDCASDescriptor(cd);
	return succ;
}

//int main(){}
