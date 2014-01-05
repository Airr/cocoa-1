//******************************************************************
//	File name: simplerDcas.c
//
//	Description:
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//	11.Oct.2008 - #DEF#18: Implement mutex dcas implementation.
//******************************************************************

#include <stdio.h>
#include <stdlib.h>

#include "simplerDcas.h"
#include "dcas.h"

void InitializationS(){
	Empty_Node = (Node*)malloc(sizeof(Node));
	Empty_Node -> left = Empty_Node;
	Empty_Node -> right = Empty_Node;
	Right_Node = Empty_Node;
	Left_Node = Empty_Node;
}

void PrintLS(){
	Node* temp = Left_Node;
	while(temp != Empty_Node){
		printf("%d",temp->value);
		if(temp != Right_Node)
		{
			temp = temp->right;
			printf("<=>");
		}
		else
			break;

	}
	printf("\n");
}

void PrintRS(){
	Node* temp = Right_Node;
	while(temp != Empty_Node &&
		  (!IsDcasDescriptor(temp))){
		printf("%d",temp->value);
		if(temp != Left_Node)
		{
			temp = temp->left;
			printf("<=>");
		}
		else
			break;
	}
	printf("\n");
}

//[8:31 PM 10/11/2008 #DEF#18 - Dotan.H -START]
void pushLeftS(int Val){
	Node* New_Node = (Node*)malloc(sizeof(Node));
	Node* e;
	New_Node -> left = New_Node;            /////////////////////
	New_Node -> value = Val;
	while(TRUE){
		New_Node -> right = New_Node;           //////////////////
		e = (Node*)GetRealAddr((int*)Left_Node);

		if(e == Empty_Node){
			if(mDcas((void**)&Right_Node ,(void**)&Left_Node ,(void*)Empty_Node ,(void*)Empty_Node ,(void*)New_Node ,(void*)New_Node))
				break;
			}else{
				New_Node -> right = e;
				if(mDcas((void**)&Left_Node ,(void**)&(e -> left) ,(void*)e ,(void*)(e -> left) ,(void*)New_Node ,(void*)New_Node))
					break;
			}
	}
}
//[8:31 PM 10/11/2008 #DEF#18 - Dotan.H -END]

//[8:31 PM 10/11/2008 #DEF#18 - Dotan.H -START]
void pushRightS(int Val){
        Node* New_Node = (Node*)malloc(sizeof(Node));
        Node* e;
        New_Node -> right = New_Node;              ////////////////////////////
        New_Node -> value = Val;
        while(TRUE){
                e = Right_Node;
                New_Node -> left = New_Node;
                if(e == Empty_Node){
                        if(mDcas((void**)&Right_Node ,(void**)&Left_Node ,(void*)Empty_Node ,(void*)Empty_Node ,(void*)New_Node ,(void*)New_Node))
                                break;
                        }else{
                                New_Node -> left = e;
                                if(mDcas((void**)&Right_Node ,(void**)&(e -> right) ,(void*)e ,(void*)(e -> right) , (void*)New_Node ,(void*)New_Node))
                                        break;
                        }
        }
}
//[8:31 PM 10/11/2008 #DEF#18 - Dotan.H -END]


int popLeftS(void){
	while(1){
		Node* x = Left_Node;
		Node* y = x -> right;
		if(x == Empty_Node){
			//printf("Empty list\n");
			return -1;
		}
		if(x != y)
			if(mDcas((void**)&Left_Node ,(void**)&(y -> left) ,(void*)x ,(void*)x ,(void*)y ,(void*)y))
				return x -> value;

		if(Right_Node == x)
			if(mDcas((void**)&Left_Node ,(void**)&Right_Node ,(void*)x ,(void*)x ,(void*)Empty_Node , (void*)Empty_Node))
				return x -> value;
	}
}

int popRightS(void){
	while(1){
		Node* x = Right_Node;
		Node* y = x -> left;
		if(x == Empty_Node){
			//printf("empty list\n");
			return -1;
		}
		if(x != y)
			if(mDcas((void**)&Right_Node ,(void**)&(x -> left) ,(void*)x ,(void*)y ,(void*)y ,(void*)x))
				return x -> value;
		if(Left_Node == x)
			if(mDcas((void**)&Left_Node ,(void**)&Right_Node ,(void*)x ,(void*)x ,(void*)Empty_Node ,(void*) Empty_Node))
			return x -> value;
	}
}
