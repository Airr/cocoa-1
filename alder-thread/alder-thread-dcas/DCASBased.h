//******************************************************************
//	File name: DCASBased.h
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//******************************************************************


#ifndef DCAS_BASED_H
#define DCAS_BASED_H
#include "pointerCont.h"

void InitializationB();
int popRightB();
int popLeftB();
void pushRightB(int v);
void pushLeftB(int v);
void deleteRight();
void deleteLeft();
void PrintRB();
void PrintLB();
node* NodeMalloc();
 

#endif //DCAS_BASED_H
