//******************************************************************
//	File name: testAlgos.h
//
//	Description: Test algorithms in concurrent environment
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#ifndef TEST_PERFORM_DEF_H
#define TEST_PERFORM_DEF_H

#define UINT32 unsigned int

// Enums

// Local structures

typedef struct{
	UINT32 unTestNumber;
	UINT32 unNumberOfThreads;
	UINT32 unNumberOfOperations;
	UINT32 unPushLeftPer;
	UINT32 unPushRightPer;
	UINT32 unPopLeftPer;
	UINT32 unPopRightPer;
	eAlgorithmType eAlgoType;
} TestDescriptor;

// Static variables
//extern static pthread_key_t thread_log_key;

// Printers

// Initializers

// Test Functions

// Threads Functions
void* ThreadAlgoExec(void* desc);

// Main Test Handler - calls from main
void PreformPerformanceTest(TestDescriptor* tDesc);

// BreakPoint functions

#endif // TEST_PERFORM_DEF_H

