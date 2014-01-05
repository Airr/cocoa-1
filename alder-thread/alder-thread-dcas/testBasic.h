//******************************************************************
//	File name: testBasic.h
//
//	Description: Test the basic function of the algorithms
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#ifndef TEST_BASIC_DEF_H
#define TEST_BASIC_DEF_H

// Definition: precompiler
#define DEBUG 1

// Test Functions
int test_asm_cas_succ();
int test_asm_cas_succ_old();
int test_asm_cas_fail();
int test_asm_cas_fail_old();
int test_simpler_algorithm();
int test_myPointer();
int test_descriptor();
int test_nodeAllocation();
int test_nodeWithDeleteB();
int test_nodeWithDeleteB2();
int test_intOnPointer();
int test_completeRDCSSa1EQo1();
int test_completeRDCSSa1NEQo1();
int test_fullRDCSSTest();
int test_fullDCASTestSucc();
int test_fullDCASTestFail();
int test_fullDCASReadSucc();
int test_fullmDCASReadSucc();
int test_descCheck();
int MoreSimplerPopRight();


void TestBased();
void TestSimpler();

// Main Test Handler - calls from main
void PreformBasicTests();

#endif // TEST_BASIC_DEF_H
