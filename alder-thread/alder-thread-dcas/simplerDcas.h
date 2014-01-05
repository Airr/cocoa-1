//******************************************************************
//	File name: simplerDcas.h
//
//	Description:
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************

#ifndef SIMPLER_DCAS_H
#define SIMPLER_DCAS_H

struct Node_el {
	int value;
	struct Node_el* left;
	struct Node_el* right;
};

typedef struct Node_el Node;

Node*  Empty_Node;
Node*  Right_Node;
Node*  Left_Node;

void InitializationS();
void PrintLS();
void PrintRS();
void pushLeftS(int Val);
void pushRightS(int Val);
int popLeftS(void);
int popRightS(void);

#endif //SIMPLER_DCAS_H
