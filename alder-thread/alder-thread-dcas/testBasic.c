//******************************************************************
//	File name: testBasic.c
//
//	Description: Test the basic function of the algorithms
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#include <stdio.h>
#include <stdlib.h>

#include "testBasic.h"
#include "cas.h"
#include "dcas.h"
#include "simplerDcas.h"
#include "pointerCont.h"
#include "DCASBased.h"
#include "testEnv.h"

int test_asm_cas_succ()
{
	int rc = FALSE;
	unsigned int *n1;
	n1 = malloc(sizeof(unsigned int));
	//n2 = malloc(sizeof(unsigned int));

	*n1 = 1;
	//*n2 = 2;
	unsigned long res;

	printf("before call n1 is %d\n", *n1);
	res = Cas((void**) n1,(void*) 1,(void*) 2);
	printf("after call n1 is %d\n", *n1);
	//printf("Result is %d\n", res);
	printf("Answer is: %d \n", *n1);

	if (*n1 == 2)
		rc = TRUE;

	return rc;
}
#define ALL_ZERO_TWO_LSB_ONE 0x00000003

int test_descCheck()
{
	int ans;
	ans = (IsDcasDescriptor(0x00433212) && !IsDcasDescriptor(0x00433210)
	&& !IsDcasDescriptor(0x00433213) && !IsDcasDescriptor(0x00433211));

	return ans;
}

int test_asm_cas_fail()
{
	int rc = FALSE;
	unsigned int *n1,*n2,*n3;
	n1 = malloc(sizeof(unsigned int));
	n2 = malloc(sizeof(unsigned int));
	n3 = malloc(sizeof(unsigned int));

	*n1 = 1;
	*n2 = 2;
	*n3 = 3;
	unsigned long res;

	printf("before call n1 is %d\n", *n1);
	res = Cas((void**) &n1,(void*) n3,(void*) n2);
	printf("after call n1 is %d\n", *n1);
	//printf("Result is %d\n", res);
	printf("Answer is: %d \n", *n1);

	if (*n1 != *n2)
		rc = TRUE;

	return rc;
}


int test_simpler_algorithm()
{
	int rc = FALSE;
		int Number;
	int tipe;

	InitializationS();

	while(1){
		printf("enter 0 to Pop_Left\n");
		printf("enter 1 to Pop_Right\n");
		printf("enter 2 to Push_Left\n");
		printf("enter 3 to Push_Right\n");
		printf("enter -1 to exit\n");
		scanf("%d",&tipe);
		if(tipe == -1 )
			break;
		if(tipe == 0)
			printf("Pop_Left value Who's %d\n", popLeftS());
		else if(tipe == 1)
				printf("Pop_Right value Who's %d\n", popRightS());
			else	if(tipe == 2){
						printf("enter anumber\n");
						scanf("%d", &Number);
						pushLeftS(Number);
					}else	if(tipe == 3){
								printf("enter anumber\n");
								scanf("%d", &Number);
								pushRightS(Number);
							}
		PrintLS();
	}
	return rc;
}

// PointerContainer Structure implementation
int test_myPointer()
{
	int* myPoint = MallocInt();
	printf("New int allocation is in addr: %p \n", myPoint);
	PointerContainer pc;
	pc.ptr = (int) myPoint;
	pc.ptrDel.del = 1;
	printf("New Pointer allocation is in addr %x \n", pc.ptr);

	return IsRDCSSDescriptor(pc.ptr);
}

int test_descriptor()
{
	int ans = FALSE;
	// lets create a pointer to a descriptot:
	PointerContainer* dPointer = MallocPRDCSSDescriptor();
	//printf("desc adress is %x \n", dPointer->ptr);
	// ans need to return TRUE
	ans = IsRDCSSDescriptor(dPointer->ptr);

	PointerContainer* dInt = MallocPInt();
	ans = ans & !IsRDCSSDescriptor(dInt->ptr);

	return ans;
}

int test_nodeAllocation()
{
	node* nodTest= MallocNode();
	PointerContainer* pc = malloc(sizeof(PointerContainer));
	pc-> ptr = (int) nodTest;
	return (int)nodTest == pc->ptr;
}

int test_nodeWithDeleteB()
{
	node* nodTest= MallocNode();
	PointerContainer* pc = malloc(sizeof(PointerContainer));
	pc-> ptr = (int) nodTest;
	pc-> ptrDel.del = 1;
	return ((int)nodTest != pc->ptr);
}

int test_nodeWithDeleteB2()
{
	node* nodTest= MallocNode();
	PointerContainer* pc = malloc(sizeof(PointerContainer));
	pc-> ptr = (int) nodTest;
	pc-> ptrDel.del = 3;
	return ((int)nodTest == GetRealAddr(pc->ptr));
}

int test_intOnPointer()
{
	int ans;

	PointerContainer* pc = MallocPInt();
	if (!IsRDCSSDescriptor(pc->ptr))
	{
		*((int*) pc->ptr) = StoreInt(5);
	}
	ans = *(int*)(RDCSSRead( &(pc->ptr) ));

	return (RetInt(ans)==5);
}

int test_completeRDCSSa1EQo1()
{
	int ans;
	PointerContainer* pc = MallocPRDCSSDescriptor();
	RDCSSDescriptor_t* rd = (RDCSSDescriptor_t*) GetRealAddr(pc->ptr);
	// RDCSSDescriptor_t init
	rd->a1 = *(MallocPInt());
	rd->a2 = *(MallocPInt());

	// *a1 = 5; o1 = 5; a2 = descriptor; o2 = 10; n2 = 15;
	*((int*) rd->a1.ptr) = StoreInt(5);
	rd->o1 = StoreInt(5);
	rd->n2 = StoreInt(15);
	rd->o2 = StoreInt(10);

	if (IsRDCSSDescriptor(pc->ptr))
	{
		// a2 points into an int cell that contains of the descriptor adress ( extended adress )
		*(int*)(rd->a2.ptr) = pc->ptr;
		printf("rd->a2.ptr:%x\n",rd->a2.ptr);

		//PrintDescriptor(rd);
		//PrintDescriptor(rd); // TODO: (Dotan.H) Remove.
		ans = RDCSSRead((int*)rd->a2.ptr);

		//printf("%d == %d \n", *((int*)(rd->a2.ptr)), rd->n2);

		ans = (*((int*)(rd->a2.ptr)) == rd->n2);
	}
	return ans;
}

int test_completeRDCSSa1NEQo1()
{
	int ans;
	PointerContainer* pc = MallocPRDCSSDescriptor();
	RDCSSDescriptor_t* rd = (RDCSSDescriptor_t*) GetRealAddr(pc->ptr);
	// RDCSSDescriptor_t init
	rd->a1 = *(MallocPInt());
	rd->a2 = *(MallocPInt());

	// *a1 = 5; o1 = 5; a2 = descriptor; o2 = 10; n2 = 15;
	*((int*) rd->a1.ptr) = StoreInt(5);
	rd->o1 = StoreInt(6);
	rd->n2 = StoreInt(15);
	rd->o2 = StoreInt(10);

	if (IsRDCSSDescriptor(pc->ptr))
	{
		// a2 points into an int cell that contains of the descriptor adress ( extended adress )
		*((int*)(rd->a2.ptr)) = pc->ptr;
		//PrintDescriptor(rd); // TODO: (Dotan.H) Remove.
		ans = RDCSSRead( (int*)rd->a2.ptr );

		ans = (*((int*)(rd->a2.ptr)) == rd->o2);
	}
	return ans;
}

int test_fullRDCSSTest()
{
	int ans;
	PointerContainer* pc = MallocPRDCSSDescriptor();
	RDCSSDescriptor_t* rd = (RDCSSDescriptor_t*) GetRealAddr(pc->ptr);
	// RDCSSDescriptor_t init
	rd->a1 = *(MallocPInt());
	rd->a2 = *(MallocPInt());

	// *a1 = 5; o1 = 5; a2 = 10; o2 = 10; n2 = 15;
	*((int*) rd->a1.ptr) = StoreInt(5);
	//*((int*) rd->a2.ptr) = StoreInt(10);
	*(int*)(rd->a2.ptr) = pc->ptr;
	rd->o1 = StoreInt(5);
	rd->o2 = StoreInt(10);
	rd->n2 = StoreInt(15);

	// before RDCSS print the Decriptor content
	PrintRDCSSDescriptor(rd); // TODO: (Dotan.H) Remove.
	RDCSS(rd);
	// after RDCSS print the Decriptor content
	PrintRDCSSDescriptor(rd); // TODO: (Dotan.H) Remove.

	ans = (	*((int*) rd->a1.ptr) == rd->o1 &&
			*((int*) rd->a2.ptr) == rd->n2);

	return ans;
}

// Test the full DCAS operation using implementation of the article "A practical Multi word compare and swap operation"
int test_fullDCASTestSucc()
{
	int ans;
	int a = StoreInt(5);
	int b = StoreInt(6);

	DCASDescriptor_t* ds = CreateDCASDescriptor(&a, StoreInt(5), StoreInt(6),
                                                &b, StoreInt(6), StoreInt(8));
	PrintDCASDescriptor(ds);

	CAS2(ds);

	PrintDCASDescriptor(ds);

	ans = (((*((int*)(ds->entry[0].addr->ptr))) == StoreInt(6)) &
			((*((int*)(ds->entry[1].addr->ptr))) == StoreInt(8)));

	return ans;
}

int test_fullDCASTestFail()
{
	int ans;
	int a = StoreInt(5);
	int b = StoreInt(6);

	DCASDescriptor_t* ds = CreateDCASDescriptor(&a, StoreInt(5), StoreInt(6),
                                                &b, StoreInt(7), StoreInt(8));
	//PrintDCASDescriptor(ds);

	CAS2(ds);

	//PrintDCASDescriptor(ds);

	ans = (((*((int*)(ds->entry[0].addr->ptr))) == StoreInt(5)) &
			((*((int*)(ds->entry[1].addr->ptr))) == StoreInt(6)));

	return ans;
}

int test_fullDCASReadSucc()
{
	int ans;
	int a = StoreInt(5);
	int b = StoreInt(6);

	DCASDescriptor_t* ds = CreateDCASDescriptor(&a, StoreInt(5), StoreInt(6),
                                                &b, StoreInt(6), StoreInt(8));
	//PrintDCASDescriptor(ds);

	a = CreateFakeDCASDesc((int)ds);

	//CAS2(ds);

	DCASRead(&a);

	//PrintDCASDescriptor(ds);

	ans = (((*((int*)(ds->entry[0].addr->ptr))) == StoreInt(6)) &
			((*((int*)(ds->entry[1].addr->ptr))) == StoreInt(8)));

	return ans;
}

int test_fullmDCASReadSucc()
{
	int ans;
	printf("start test_fullmDCASReadSucc...\n");
	int* memA = malloc(sizeof(int));
	int* memB = malloc(sizeof(int));
	
	*memA = 5;
	*memB = 6;
	
	mDcas(memA, memB, 5, 6, 6, 8);
	
	ans = ((*memA == 6) && (*memB == 8));
	printf("end test_fullmDCASReadSucc...\n");
	return ans;
}


void TestBased()
{
	int i = 1;
	InitializationB();

	printf("10 pushRight\n");
	for(i =1 ; i < 11 ; i++)
	{
		pushRightB(StoreInt(i));
		PrintRB();
	}
	
	printf("12 times popLeft\n");
	for(i =1 ; i < 13 ; i++)
	{
		popLeftB();
		PrintRB();
	}

	

	
	printf("0<i<10   if i%2 = 0 then pusheRight else pushLeft\n");
	for(i =1 ; i < 11 ; i++)
	{

		if(i%2==0)
		{
			pushRightB(StoreInt(i));
		}
		else
			pushLeftB(StoreInt(i));

		PrintRB();
	}

	printf("0<i<12 if i%2 = 0 popRight else popLeft\n");
	for(i =1 ; i < 13 ; i++)
	{

		if(i%2==0)
		{
			popRightB();
		}
		else
			popLeftB();

		PrintRB();
	}

	printf("10 times pushLeft, popRight popRight\n");
	for(i = 1; i < 11 ; i++)
	{
		pushLeftB(StoreInt(i));
		PrintRB();
		popRightB();
		popRightB();
		PrintRB();
	}

	printf("10 pushRight\n");
	for(i =1 ; i < 11 ; i++)
	{
		pushRightB(StoreInt(i));
		PrintRB();
	}

	printf("10 popRightB and pushRight\n");
	for(i =1 ; i < 11 ; i++)
	{
		popRightB();
		PrintRB();
		pushRightB(StoreInt(i));
		PrintRB();
	}
	
}

int MoreSimplerPopRight()
{
	int ans = FALSE;
	// Initialization of Uri's Algorithm
	InitializationS();
	// Push-right number 10 and 20 and then print data stucture
	pushRightS(StoreInt(10));
	pushRightS(StoreInt(20));
	PrintRS();

	return ans;
}


void TestSimpler()
{
	int i = 1;
	InitializationS();

	printf("10 pushRight\n");
	for(i =1 ; i < 11 ; i++)
	{
		pushRightS(StoreInt(i));
		PrintRS();
	}

	printf("12 times popLeft\n");
	for(i =1 ; i < 13 ; i++)
	{
		//popRightB();
		popLeftS();
		PrintRS();
	}

	printf("0<i<10   if i%2 = 0 then pusheRight else pushLeft\n");
	for(i =1 ; i < 11 ; i++)
	{

		if(i%2==0)
			pushRightS(StoreInt(i));
		else
			pushLeftS(StoreInt(i));

		PrintRS();
	}

	printf("0<i<12 if i%2 = 0 popRight else popLeft\n");
	for(i =1 ; i < 13 ; i++)
	{

		if(i%2==0)
			popRightS();
		else
			popLeftS();

		PrintRS();
	}

	printf("10 times pushLeft, popRight popRight\n");
	for(i = 1; i < 11 ; i++)
	{
		pushLeftS(StoreInt(i));
		PrintRS();
		popRightS();
		popRightS();
		PrintRS();
	}

	printf("10 times pushLeft, popRight popRight\n");
	for(i = 1; i < 0 ; i++)
	{
		pushLeftS(StoreInt(i));
		PrintRS();
		popLeftS();
		PrintRS();
		PrintLS();
	}
}

void PreformBasicTests()
{
	if (DEBUG)
	{
		// Test the basic functions on the single thread for correctness - START
			int tNum = 1;
			// Tests for CAS Functionality - START

				//TestA(tNum++, test_asm_cas_succ());
				//TestA(tNum++, test_asm_cas_fail());

			// Test CAS Functionality - START

			// Test for pointer containers - START

				//TestA(tNum++, test_myPointer());
				//TestA(tNum++, test_descriptor());
				//TestA(tNum++, test_nodeAllocation());
				//TestA(tNum++, test_nodeWithDeleteB());
				//TestA(tNum++, test_nodeWithDeleteB2());

			// Test for pointer containers - END

			// Test for RDCSS functionality - START

				//TestA(tNum++, test_intOnPointer());
				//TestA(tNum++, test_completeRDCSSa1EQo1());
				//TestA(tNum++, test_completeRDCSSa1NEQo1());
				//TestA(tNum++, test_fullRDCSSTest());
				//TestA(tNum++, test_fullDCASTestSucc());
				//TestA(tNum++, test_fullDCASTestFail());
			// Test for RDCSS functionality - END

			// Test for changes of the rdcss desciptor identifier - START
				//TestA(tNum++, test_fullDCASReadSucc());
				//TestA(tNum++, test_descCheck());
			// Test for changes of the rdcss desciptor identifier - END

			//test for DCASBased - START
				TestBased();
			//test for DCASBased -  END

			//test for simplerDcas - START
				//TestA(tNum++, MoreSimplerPopRight());
				//TestSimpler();
			//test for simplerDcas -  END
			
			// test for mDcas - START
				//TestA(tNum++, test_fullmDCASReadSucc());
			// test for mDcas - END

		// Test the basic functions on the single thread for correctness - END

		// Major algorithems - Uri.A && DCAS based concurrency - START (Single thread)
			//TestA(tNum++, test_simpler_algorithm());
		// Major algorithems - Uri.A && DCAS based concurrency - END
	}
}
