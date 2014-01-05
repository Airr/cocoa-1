//******************************************************************
//	File name: testAlgos.c
//
//	Description: Test algorithms in concurrent environment
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "testAlgos.h"
#include "cas.h"
#include "dcas.h"
#include "simplerDcas.h"
#include "pointerCont.h"
#include "DCASBased.h"
#include "testEnv.h"

int* Barrier;

int TestUAlgorithmsPushR()
{
	int ans = FALSE;
	int i = 0;
	int threadCounter = 0;
	int NUM_OF_THREADS = 40;
	Barrier = malloc(sizeof(int));
	*Barrier = 0;

	pthread_t pthread_id_arr[NUM_OF_THREADS];
	int iret_arr[NUM_OF_THREADS];

	// Preform Initialization of SimpleAlgorithm
	InitializationS();
	pthread_key_create (&thread_log_key, CloseThreadLog);

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		pthread_create(&(pthread_id_arr[threadCounter]),NULL, (void*) ThreadUriAlgoF, NULL);
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		iret_arr[threadCounter] = pthread_join(pthread_id_arr[threadCounter],NULL);
		printf("Thread%d return with code %d \n", threadCounter ,iret_arr[threadCounter]);
	}

	PrintRS();
	PrintLS();

	return ans;
}

int TestBAlgorithmsPushR()
{
	int ans = FALSE;
	int i = 0;
	int threadCounter = 0;
	int NUM_OF_THREADS = 88;
	Barrier = malloc(sizeof(int));
	*Barrier = 0;

	pthread_t pthread_id_arr[NUM_OF_THREADS];
	int iret_arr[NUM_OF_THREADS];

	// Preform Initialization of SimpleAlgorithm
	InitializationB();
	pthread_key_create (&thread_log_key, CloseThreadLog);

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		pthread_create(&(pthread_id_arr[threadCounter]),NULL, (void*) ThreadBasedAlgoF, NULL);
	}

	for(threadCounter = 0; threadCounter < NUM_OF_THREADS; threadCounter++)
	{
		iret_arr[threadCounter] = pthread_join(pthread_id_arr[threadCounter],NULL);
		printf("Thread%d return with code %d \n", threadCounter ,iret_arr[threadCounter]);
	}

	PrintRB();
	PrintLB();

	return ans;
}

void* ThreadUriAlgoF()
{
	int i = 0;
	int NUM_OF_PUSH	= 40;
	int succ = FALSE;
	ThreadLoggerInit();

	(*Barrier)++;
	//printf("*");
	do{
		succ = Cas( (void**)Barrier, (void*)40, (void*)40);
	}while (!succ);

	Holder();
	printf("*\n");

	// Here are the functions that will be perform on Algo - Start
	for(i = 0; i < NUM_OF_PUSH; i++)
	{
		pushRightS(i*2);
		popLeftS();
	}
	
	for (i = 0; i < NUM_OF_PUSH; i++)
	{
		pushLeftS((i+1)*2);
		popRightS();
	}
	
	// Here are the functions that will be perform on Algo - End

	pthread_exit(0);
}

void* ThreadBasedAlgoF()
{
	int i = 0;
	int NUM_OF_PUSH	= 30;
	int succ = FALSE;
	ThreadLoggerInit();

	(*Barrier)++;
	//printf("*");
	do{
		succ = Cas( (void**)Barrier, (void*)88, (void*)88);
	}while (!succ);

	Holder();
	printf("*\n");

	// Here are the functions that will be perform on Algo - Start
	for(i = 0; i < NUM_OF_PUSH; i++)
	{
		pushRightB(StoreInt(i*2));
		popLeftB();
	}
	
	for (i = 0; i < NUM_OF_PUSH; i++)
	{
		pushLeftB(StoreInt((i+1)*2));
		//popRightB();
	}
	
	// Here are the functions that will be perform on Algo - End

	pthread_exit(0);
}

void Holder()
{
	return;
}

void PreformAlgoTests()
{
				//printf("size of Node is: %d \n", sizeof(Node));
				int testNum = 1;
				// Major algorithms - Uri.A && DCAS based concurrency - START
					//TestA(testNum++,TestUAlgorithmsPushR());
					TestA(testNum++,TestBAlgorithmsPushR());
				// Major algoritahems - Uri.A && DCAS based concurrency  - END
}
