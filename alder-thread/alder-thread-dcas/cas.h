//******************************************************************
//	File name: cas.h
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	25.Aug.2008 - (NoNum) : (Dotan.H) Small examle to test the cas operation on multithreaded environment
//******************************************************************

#ifndef _DEBUG_LEV_
#define _DEBUG_LEV_
#define DEBUG_LEV 0
#endif //_DEBUG_LEV_

#ifndef _TRUE_
#define _TRUE_
#define TRUE 1
#endif //_TRUE_

#ifndef _FALSE_
#define _FALSE_
#define FALSE 0
#endif //_FALSE_

#ifndef CAS_DEF_H
#define CAS_DEF_H

#include <stdint.h>

inline int Cas(void** a1, void* v1, void* w1);
inline uint64_t casAsm(volatile uint64_t *ptr,
							uint64_t oldVal,
							uint64_t newVal);
#endif //CAS_DEF_H
