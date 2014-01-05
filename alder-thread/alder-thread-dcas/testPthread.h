//******************************************************************
//	File name: testPthread.h
//
//	Description: Test function and modules in concurrent environment
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#ifndef TEST_PTHREAD_DEF_H
#define TEST_PTHREAD_DEF_H

// for DCASDescriptor_t sturct
#include "dcas.h"

// Local structures
typedef struct{
	unsigned long* addr;
	unsigned long oldReg;
	unsigned long newReg;
	char* threadNo;
	} CasDesc;
	
typedef struct{
	RDCSSDescriptor_t* rdcssDesc; 
	int threadNo;
} RdcssWrap; 

typedef struct{
	DCASDescriptor_t* dcasDesc;
	int threadNo;
} DcasWarp;

// Static variables
//extern static pthread_key_t thread_log_key;

// Printers
void PrintCasDesc(CasDesc*);
void PrintRdcssWrapDesc(RdcssWrap* rdw);
void PrintDcasDesc(DcasWarp* dcw);

// Initializers
void SimpleInit();

// Test Functions
int performCasSingleThreadTest();
int performNoThreadTest();
int performRDCSSThreadTest();
int persormCAS2ThreadTest();
int performDCASThreadTest();

// Threads Functions
void *threadCasTest( CasDesc*);
void* threadRdcssF(RdcssWrap* rdw);
void* threadDcasF(DcasWarp* dcw);
void* threadCas2F(DcasWarp* dcw);

// Main Test Handler - calls from main
void PreformThreadTests(); 

#endif // TEST_PTHREAD_DEF_H
