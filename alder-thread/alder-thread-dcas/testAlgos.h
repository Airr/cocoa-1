//******************************************************************
//	File name: testAlgos.h
//
//	Description: Test algorithms in concurrent environment
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#ifndef TEST_ALGOS_DEF_H
#define TEST_ALGOS_DEF_H

// for DCASDescriptor_t sturct
#include "dcas.h"

// Local structures
	

// Static variables
//extern static pthread_key_t thread_log_key;

// Printers

// Initializers

// Test Functions
int TestUAlgorithmsPushR();
int TestBAlgorithmsPushR();

// Threads Functions
void* ThreadUriAlgoF();
void* ThreadBasedAlgoF();

// Main Test Handler - calls from main
void PreformAlgoTests(); 

// BreakPoint functions
void Holder();

#endif // TEST_ALGOS_DEF_H
