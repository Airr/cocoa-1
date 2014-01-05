//******************************************************************
//	File name: testPthread.c
//
//	Description: Test function and modules in concurrent environment
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "testPthread.h"
#include "cas.h"
#include "dcas.h"
#include "simplerDcas.h"
#include "pointerCont.h"
#include "DCASBased.h"
#include "testEnv.h"

int performCasThreadTest()
{
	int ans = FALSE;

	// Create two casDesc.
	// this test checks if the cas operation can run concurrently
	CasDesc* cd1 = (CasDesc*)malloc(sizeof(CasDesc));
	CasDesc* cd2 = (CasDesc*)malloc(sizeof(CasDesc));
	// We want that the thread will deliver premmition each other the premmition to succeed
	unsigned long* myAddress = (unsigned long*)malloc(sizeof(unsigned long));
	*myAddress = 2; // allow the second thread to succeed first
	// struct initialize;
	cd1->addr = myAddress;
	cd1->oldReg = 1;
	cd1->newReg = 2;
	cd1->threadNo = "Thread1";

	cd2->addr = myAddress;
	cd2->oldReg = 2;
	cd2->newReg = 1;
	cd2->threadNo = "Thread2";

	// Prints initialization values
	printf("After Initialization:\n");
	printf("CASDESC I:\n\t");
	PrintCasDesc(cd1);
	printf("CASDESC II:\n\t");
	PrintCasDesc(cd2);

	pthread_t pthread1_id, pthread2_id;
	int iret1, iret2;
	pthread_create(&pthread1_id, NULL, (void*) threadCasTest, cd1);
	pthread_create(&pthread2_id, NULL, (void*) threadCasTest, cd2);

	iret2 = pthread_join(pthread2_id,NULL);
	printf("Thread2 has been returned with value: %d\n", iret2);
	iret1 = pthread_join(pthread1_id,NULL);
	printf("Thread1 has been returned with value: %d\n", iret1);


	// Prints initialization values
	printf("After Run:\n");
	printf("CASDESC I:\n\t");
	PrintCasDesc(cd1);
	printf("CASDESC II:\n\t");
	PrintCasDesc(cd2);

	ans = ( *(cd1->addr) == 2);

	return ans;
}


int performCasSingleThreadTest()
{
	int ans = FALSE;

	// Create single casDesc.
	// this test checks if the cas operation can run concurrently
	CasDesc* cd1 = malloc(sizeof(CasDesc));

	// We want that the thread will deliver premmition each other the premmition to succeed
	unsigned long* myAddress = malloc(sizeof(unsigned long));
	*myAddress = 1; // allow the second thread to succeed first
	// struct initialize;
	cd1->addr = myAddress;
	cd1->oldReg = 1;
	cd1->newReg = 2;
	cd1->threadNo = "Thread1";

	// Prints initialization values
	printf("After Initialization:\n");
	printf("CASDESC I:\n\t");
	PrintCasDesc(cd1);

	pthread_t pthread1_id;
	int iret1;
	pthread_create(&pthread1_id, NULL, (void*) threadCasTest, cd1);

	iret1 = pthread_join(pthread1_id,NULL);
	printf("Thread1 has been returned with value: %d\n", iret1);


	// Prints initialization values
	printf("After Run:\n");
	printf("CASDESC I:\n\t");
	PrintCasDesc(cd1);

	ans = ( *(cd1->addr) == 2);

	return ans;
}


void *threadCasTest( CasDesc* cd)
{
	// Local variable
	int succ = FALSE;
	int i;
	int try;

	for (i = 0; i< 100; i++)
	{
		try = 0;
		do
		{
			succ = Cas((void**)cd->addr,(void*) cd->oldReg,(void*) cd->newReg);
			// Check the times that thread was on busy wait

			if ((try % 10000) == 0)
				if (!succ)
				{
					printf("\t\t %s entered and tries %d times \n",cd->threadNo,try);
					PrintCasDesc(cd);
				}
				else
				{
					printf("\t\t %s SUCCEDDED in %d times \n",cd->threadNo,try);
					PrintCasDesc(cd);
				}
			try++;

		}while(!succ);
	}

	pthread_exit(0);
}

int performNoThreadTest()
{
	int ans = FALSE;

	unsigned long* a1 = malloc(sizeof(unsigned long));
	*a1 = 1;
	unsigned long v1 = 1;
	unsigned long w1 = 2;

	printf("*a1: %d, v1: %d, w1: %d \n", *a1, v1, w1);
	unsigned long retVal;
	retVal = Cas( (void**) a1,(void*) v1,(void*) w1);
	printf("*a1: %d, v1: %d, w1: %d \n", *a1, v1, w1);
	printf("retVal: %d\n", retVal);

	return (*a1 == 2);
}

int performRDCSSThreadTest()
{
	int ans;
	int threadCounter = 0;
	int NUM_OF_THREADS = 8;

	// Create ket to associate thread log file pointer in thread-specific data. use close thread_log to clean up the file pointers.
	pthread_key_create (&thread_log_key, CloseThreadLog);

	RdcssWrap* rdw_arr[NUM_OF_THREADS];
	pthread_t pthread_id_arr[NUM_OF_THREADS];
	int iret_arr[NUM_OF_THREADS];
	char* threadName_arr[NUM_OF_THREADS];

	PointerContainer* mNum1 = MallocPInt();
	PointerContainer* mNum2 = MallocPInt();

	*(int*)(mNum1->ptr) = StoreInt(1);
	*(int*)(mNum2->ptr) = StoreInt(NUM_OF_THREADS-1);

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		rdw_arr[threadCounter] = malloc(sizeof(RdcssWrap));
		rdw_arr[threadCounter]->rdcssDesc = malloc(sizeof(RDCSSDescriptor_t));
		rdw_arr[threadCounter]->rdcssDesc->a1 = *mNum1;

		rdw_arr[threadCounter]->rdcssDesc->o1 = StoreInt(1);
		rdw_arr[threadCounter]->rdcssDesc->a2 = *mNum2;
		rdw_arr[threadCounter]->rdcssDesc->o2 = StoreInt((threadCounter+1) % NUM_OF_THREADS);
		rdw_arr[threadCounter]->rdcssDesc->n2 = StoreInt((threadCounter) % NUM_OF_THREADS);
		rdw_arr[threadCounter]->threadNo = threadCounter+1;
	}
	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		pthread_create(&(pthread_id_arr[threadCounter]),NULL, (void*) threadRdcssF, rdw_arr[threadCounter]);
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		iret_arr[threadCounter] = pthread_join(pthread_id_arr[threadCounter],NULL);
		printf("Thread%d return with code %d \n", threadCounter ,iret_arr[threadCounter]);
	}

	//PrintRdcssWrapDesc(rdw_arr[0]);

	return ans;
}

void ThreadLoggerInit()
{
	char thread_log_filename[20];
	FILE* thread_log;
	//Generate the filename for this thread log file:
	sprintf(thread_log_filename, "thread%d.log", (int) pthread_self());
	//Open LogFile:
	thread_log = fopen (thread_log_filename, "w");
	// Stores file pointer in thread-specific data under thread_log_key:
	pthread_setspecific(thread_log_key, thread_log);
}

void* threadRdcssF(RdcssWrap* rdw)
{
	static int threadCo = 0;
	// Initialize Thread Logger file
	ThreadLoggerInit();

	int RunCounter = 0;
	int succ = FALSE;
	int pri = FALSE;
	for (RunCounter = 0; RunCounter < 10; RunCounter++)
	{
		PrintRdcssWrapDesc(rdw);
		do{
			succ = (RDCSS(rdw->rdcssDesc) == rdw->rdcssDesc->o2);
		} while (!succ);
		PrintRdcssWrapDesc(rdw);
	}
	pthread_exit(0);
}

void PrintRdcssWrapDesc(RdcssWrap* rdw)
{
	char msg[70];
	struct timeval tim;
    char messageTime [21];
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);

	sprintf( msg,"%f thread%d : *a1:%d o1:%d a2:%x *a2:%d o2:%d n2:%d", t1, rdw->threadNo,
											RetInt(*((int*)rdw->rdcssDesc->a1.ptr)),RetInt(rdw->rdcssDesc->o1),
											(int*)rdw->rdcssDesc->a2.ptr,
											RDCSSRead(((int*)rdw->rdcssDesc->a2.ptr)),
											RetInt(rdw->rdcssDesc->o2), RetInt(rdw->rdcssDesc->n2));
	WriteToThreadLog(msg);
}

void PrintCasDesc(CasDesc* cd)
{
	printf("*addr: %d, oldReg: %d, newReg: %d \n", *(cd->addr), cd-> oldReg, cd-> newReg);
}

void PrintDcasDesc(DcasWarp* dcw)
{
	char msg[70];
	char* stringStatus;

	switch(dcw->dcasDesc->status){
	case DCAS_UNDECIDED: stringStatus = "Undecided"; break;
	case DCAS_SUCCEEDED: stringStatus = "Succeeded"; break;
	case DCAS_FAILED: 	 stringStatus = "Failed"; break;
	default:
			stringStatus = "Unknown";
	}

	sprintf( msg,"%f,thread%d,status=%s,*a1=%d,o1=%d,n1=%d,*a2=%d,o2=%d,n2=%d",
									GetCurrentTimeMSC(),					    /* Time */
									dcw->threadNo,							    /* Thread Number */
			 						stringStatus,								/* DCAS descriptor status */
									RetInt(*((int*)dcw->dcasDesc->entry[0].addr->ptr)),	/* E1->addr	*/
									RetInt(dcw->dcasDesc->entry[0].old),		/* E1->old		*/
									RetInt(dcw->dcasDesc->entry[0].new),		/* E1->new		*/
									RetInt(*((int*)dcw->dcasDesc->entry[1].addr->ptr)),	/* E2->addr	*/
									RetInt(dcw->dcasDesc->entry[1].old),		/* E2->old		*/
									RetInt(dcw->dcasDesc->entry[1].new));		/* E2->new		*/
	WriteToThreadLog(msg);
}

int performDCASThreadTest()
{
	int ans = FALSE;
	int NUM_OF_THREADS = 88;
	int threadCounter = 0;
	int* mNum1 = malloc(sizeof(int));
	int* mNum2 = malloc(sizeof(int));
	// Create key to associate thread log file pointer in thread-specific data. use close thread_log to clean up the file pointers.
	pthread_key_create (&thread_log_key, CloseThreadLog);

	DcasWarp* rdw_arr[NUM_OF_THREADS];
	pthread_t pthread_id_arr[NUM_OF_THREADS];
	int iret_arr[NUM_OF_THREADS];

	// Debug only!!!
		extern int print_rdcss;
		print_rdcss = TRUE;
	// Debug only!!!

	// Variable Initialization
	*mNum1 = StoreInt(NUM_OF_THREADS-1); *mNum2 = StoreInt(0);

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		rdw_arr[threadCounter] = malloc(sizeof(DcasWarp));
		rdw_arr[threadCounter]->threadNo = threadCounter + 1;
		rdw_arr[threadCounter]->dcasDesc = CreateDCASDescriptor(
								mNum1,
								StoreInt(threadCounter),
								StoreInt((threadCounter+1) % NUM_OF_THREADS ),
								mNum2,
								StoreInt((threadCounter+1) % NUM_OF_THREADS ),
								StoreInt((threadCounter+2) %(NUM_OF_THREADS)));
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		pthread_create(&(pthread_id_arr[threadCounter]), NULL, (void*) threadDcasF, rdw_arr[threadCounter]);
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		iret_arr[threadCounter] = pthread_join( pthread_id_arr[threadCounter], NULL);
	}

	return ans;
}

int performCAS2ThreadTest()
{
	int ans = FALSE;
	int NUM_OF_THREADS = 8;
	int threadCounter = 0;
	int* mNum1 = malloc(sizeof(int));
	int* mNum2 = malloc(sizeof(int));
	// Create key to associate thread log file pointer in thread-specific data. use close thread_log to clean up the file pointers.
	pthread_key_create (&thread_log_key, CloseThreadLog);

	DcasWarp* rdw_arr[NUM_OF_THREADS];
	pthread_t pthread_id_arr[NUM_OF_THREADS];
	int iret_arr[NUM_OF_THREADS];

	// Debug only!!!
		extern int print_rdcss;
		print_rdcss = TRUE;
	// Debug only!!!

	// Variable Initialization
	*mNum1 = StoreInt(NUM_OF_THREADS-1); *mNum2 = StoreInt(0);

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		rdw_arr[threadCounter] = malloc(sizeof(DcasWarp));
		rdw_arr[threadCounter]->threadNo = threadCounter + 1;
		rdw_arr[threadCounter]->dcasDesc = CreateDCASDescriptor(
								mNum1,
								StoreInt(threadCounter),
								StoreInt((threadCounter+1) % NUM_OF_THREADS ),
								mNum2,
								StoreInt((threadCounter+1) % NUM_OF_THREADS ),
								StoreInt((threadCounter+2) %(NUM_OF_THREADS)));
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		pthread_create(&(pthread_id_arr[threadCounter]), NULL, (void*) threadCas2F, rdw_arr[threadCounter]);
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		iret_arr[threadCounter] = pthread_join( pthread_id_arr[threadCounter], NULL);
	}

	return ans;
}

void* threadDcasF(DcasWarp* dcw)
{
	int RunCounter;
	int succ;

	PrintDCASDescriptor(dcw->dcasDesc);
	ThreadLoggerInit();
	for (RunCounter = 0; RunCounter < 10; RunCounter++)
	{
		PrintDcasDesc(dcw);
		do{
			succ = Dcas(
						(void**)dcw->dcasDesc->entry[0].addr->ptr,
						(void**)dcw->dcasDesc->entry[1].addr->ptr,
						(void*)dcw->dcasDesc->entry[0].old,
						(void*)dcw->dcasDesc->entry[1].old,
						(void*)dcw->dcasDesc->entry[0].new,
						(void*)dcw->dcasDesc->entry[1].new
						);

			//printf("%d,",my_dc->status);
		} while (succ != TRUE);
		PrintDcasDesc(dcw);
	}
	pthread_exit(0);
}

void* threadCas2F(DcasWarp* dcw)
{
	int RunCounter;
	DCASDescriptor_t* my_dc = 0;
	//PrintDCASDescriptor(dcw->dcasDesc);
	ThreadLoggerInit();
	for (RunCounter = 0; RunCounter < 5; RunCounter++)
	{
		PrintDcasDesc(dcw);
		do{
			my_dc = CAS2Copy(dcw->dcasDesc); // ERROR, need to create each time descriptor
			CAS2(my_dc);
			//printf("%d,",my_dc->status);
		} while (my_dc->status != DCAS_SUCCEEDED);
		dcw->dcasDesc = my_dc;
		PrintDcasDesc(dcw);
	}
	pthread_exit(0);
}

void PreformThreadTests()
{
			// Test the basic functions on the single thread for correctness - START
				int testNum = 1;
				//Practical CASN implementation Tests:
					// Cas test
					//TestA(testNum++,performNoThreadTest()); 		// no thread
					//TestA(testNum++,performCasSingleThreadTest()); 	// single thread
					//TestA(testNum++,performCasThreadTest());		// double thread

					// RDCSS test
						//performRDCSSThreadTest();

					// DCAS test
						//TestA(testNum++,performCAS2ThreadTest());
						//TestA(testNum++,performDCASThreadTest());

				// Major algorithems - Uri.A && DCAS based concurrency - START

				// Major algorithems - Uri.A && DCAS based concurrency  - END
}
