//******************************************************************
//	File name: main.c
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//	16.Sep.2008 - #DEF#16 : (Dotan.H) Fix PushLeft and PushRight regressions.
//  	13.Oct.2008 - #DEF#18 : (Dotan.H) Fix race condition problem in DCAS-BASED concurrent deque implementation.
//******************************************************************

// Include Test headers 
#include <stdio.h>
#include <string.h>
#include "commonStruct.h"
#include "testBasic.h"
#include "testPthread.h"
#include "testPerformance.h"
//#include <atomic.h>
#define NUMBER_OF_PARAMS 8
#define TOT_NUM_OF_PARAMS NUMBER_OF_PARAMS + 1

void PrintUsageMsg()
{
	printf("\n\033[37;1mUsage: main <TestNumber> <NumberOfThreads> <NumberOfOperations> <PL\%> <PR\%> <PoL\%> <PoR\%> <Algo> \n");
	printf("\n<TestNumber>\t\t- Test Number\n");
	printf("<NumberOfThreads>\t- Total number of threads in current test\n");
	printf("<NumberOfOperations>\t- Total number of operations in current test\n");
	printf("<PL\%>\t\t\t- Number of push left (percentage)\n");
	printf("<PR\%>\t\t\t- Number of push right (percentage)\n");
	printf("<PoL\%>\t\t\t- Number of pop left (percentage)\n");
	printf("<PoR\%>\t\t\t- Number of pop left (percentage)\n");
	printf("<Algo>\t\t\t - Algotithem to use (SIMPLE_DCAS | BASE_CONCURRENCY)\n\033[0m");
	printf("\n");
	
	return;
}

void ParseAgrv(TestDescriptor* tDesc, char** agrv)
{
	tDesc->unTestNumber = atoi(agrv[1]);
	tDesc->unNumberOfThreads = atoi(agrv[2]);
	tDesc->unNumberOfOperations = atoi(agrv[3]);
	tDesc->unPushLeftPer = atoi(agrv[4]);
	tDesc->unPushRightPer = atoi(agrv[5]);
	tDesc->unPopLeftPer = atoi(agrv[6]);
	tDesc->unPopRightPer = atoi(agrv[7]);
	tDesc->eAlgoType = atoi(agrv[8]);
	
	/*printf("\nTestNum:%d\nNumberOfThreads:%d\nNumberOfOperators:%d\n(%d,%d,%d,%d)\nAlgo:%d\n",
		tDesc->unTestNumber,
		tDesc->unNumberOfThreads,
		tDesc->unNumberOfOperations,
		tDesc->unPushLeftPer,
		tDesc->unPushRightPer,
		tDesc->unPopLeftPer,
		tDesc->unPopRightPer,
		tDesc->eAlgoType);
	*/
	return;
}

void printTestDesc(TestDescriptor* tDesc)
{
	printf("Would like to run test with follow parameters: (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			tDesc->unTestNumber,
			tDesc->unNumberOfThreads,
			tDesc->unNumberOfOperations,
			tDesc->unPushLeftPer,
			tDesc->unPushRightPer,
			tDesc->unPopLeftPer,
			tDesc->unPopRightPer,
			tDesc->eAlgoType);
}

int main(int agrc, char** agrv)
{
	int returnCode = 0;
	
	if (agrc == 1)
	{
		//PreformBasicTests();
		//PreformThreadTests();
		//PreformAlgoTests();
	}
	if (agrc != TOT_NUM_OF_PARAMS)
	{
		PrintUsageMsg();
		returnCode = -1;
	}
	if (agrc == TOT_NUM_OF_PARAMS)
	{
		TestDescriptor sTestDesc;
		TestDescriptor* pTestDesc;
		
		// Initiate pointer into testDescriptor
		pTestDesc = &sTestDesc;
		// Initiate structure with zeros
		memset(pTestDesc, 0, sizeof(sTestDesc));
		// Send to parse agrv
		ParseAgrv(&sTestDesc, agrv);
		// Send sTEstDesc into main PerformPerformanceTest
		printTestDesc(pTestDesc);
		PreformPerformanceTest(pTestDesc);
		
	}
	
	return returnCode;
} 

