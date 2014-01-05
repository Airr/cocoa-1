//******************************************************************
//	File name: DCASBased.c
//
//	Description:  Main program file
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : (Dotan.H) Separate main test to test according their functionality
//	11.Oct.2008 - #DEF#18: Implement mutex dcas implementation.
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "dcas.h"
#include "pointerCont.h"
#include "DCASBased.h"

#define NODE_VALUE_NULL -20
#define NODE_VALUE_SENR -100
#define NODE_VALUE_SENL -200

#define NODE_DELETE_FALSE 	0
#define NODE_DELETE_TRUE 	1

node *SL,*SR;

node* NodeMalloc()
{
	node* NewNode = MallocNode();
	NewNode -> L = MallocPNode();
	NewNode -> R = MallocPNode();
	return NewNode;
}



void InitializationB()
{
	SL = NodeMalloc();
	SR = NodeMalloc(); 
	
	//SL -> L;
	SL -> value = NODE_VALUE_SENL;
	(SL -> R) -> ptrDel.del = NODE_DELETE_FALSE;
	(SL -> R) -> ptr	= (int)SR;
	
	SR -> value = NODE_VALUE_SENR;
	(SR -> L) -> ptrDel.del = NODE_DELETE_FALSE;
	(SR -> L) -> ptr = (int)SL;
}


int popRightB()
{
	PointerContainer* oldL;
	PointerContainer* newL = MallocPNode();
	int v;
	while(1)
	{
		oldL = SR -> L;
		v = ((node*)(GetRealAddr((int)oldL-> ptr)))-> value;
		//if true empty list
		if(v == NODE_VALUE_SENL)
		{
			//printf("empty list \n");
			return -1;
		}
		
		//if true exeutes physicall deletion
		if(IsDeletedNode(oldL-> ptr))
			deleteRight();
		else 
			
			//if true indicates thet the list is logically empty
			if(v == NODE_VALUE_NULL)
			{
				//if(Dcas(&SR->L, &((node*) ((int)oldL-> ptr))-> value, oldL, v, oldL, v))//20/8
				if(mDcas((void **)&SR->L, (void**)&((node*) ((int)oldL-> ptr))-> value,
							oldL,(void*) v,
							oldL,(void*) v))
				{
					//printf("empty list");
					return -1;
				}
			}else
				{
					newL-> ptr = oldL-> ptr;
					newL-> ptrDel.del = NODE_DELETE_TRUE;
					
					//if true logicall deleted has occurred
					if(mDcas((void**)&SR-> L,(void**) &((node*)((int) oldL-> ptr))-> value,
														oldL, (void*)v, 
														newL,(void*) NODE_VALUE_NULL))//20/8  void** int
						return v;
						
				}
			
	}
}

// 1   pop left execute an logicall deletion
//2   if the node to the right of the SL is already logically deleted then we will physically delete it first
//3   if step 2 occured then we will retry from the bginning of the loop 

int popLeftB()
{
	PointerContainer* oldR;
	PointerContainer* newR = MallocPNode();
	int v;
	while(1)
	{
		oldR = SL -> R;
		v = ((node*)(GetRealAddr((int)oldR-> ptr)))-> value;
		//if true empty list
		if(v == NODE_VALUE_SENR)
		{
			//printf("empty list \n");
			return -1;
		}
		
		//if true exeutes physicall deletion
		
		if(IsDeletedNode(oldR-> ptr))
		{
			deleteLeft();
		}
		else 
			
			//if true indicates thet the list is logically empty
			if(v == NODE_VALUE_NULL)
			{
				if(mDcas((void**)&SL->R, (void**)&((node*)((int) oldR-> ptr))-> value,
							oldR,(void*) v,
							oldR,(void*) v))// 20/8
				{
					//printf("empty list");
					return -1;
				}
			}else
				{
					newR-> ptr = oldR-> ptr;
					newR-> ptrDel.del = NODE_DELETE_TRUE;
			
					
					//int Dcas(void** a1,void** a2,void* v1,void* v2,void* w1,void* w2);
										//if true logicall deleted has occurred
					if(mDcas((void**)&SL-> R, (void**)&((node*)((int) oldR-> ptr))-> value,
										oldR, (void*)v,
										newR, (void*)NODE_VALUE_NULL))//20/8	
						return v;
						
						
				}
			
	}
}


// 1    create and enters a new node to the list
// 2   if the node to the left of the SR is already logically deleted then we will physically delete it first
//3    if step 2 occured then we will retry from the bginning of the loop 

void pushRightB(int v)
{	
	
	PointerContainer* oldL;
	PointerContainer* oldLR = MallocPNode();	
	//create a new node
	PointerContainer* newL = MallocPNode();
	newL-> ptr = (int)NodeMalloc();//20/8
	//newL->ptr = (node*)malloc(sizeof(node));
	//alloction failed memory full
	
	/*if (newL -> ptr == 0)
	{
		printf("Memory is full\n");
		return;
	}*/
	
	newL -> ptrDel.del = NODE_DELETE_FALSE;
	
	while(1)
	{	

		oldL = SR -> L;
		//if true exeutes physicall deletion
		
		if (oldL -> ptrDel.del == NODE_DELETE_TRUE)
			deleteRight();
		else
		{
			
			//the right pointer of the new node will point to SR
			
			(((node*)((int)newL -> ptr))-> R)-> ptr = (int)SR;//20/8
			(((node*)((int)newL -> ptr))-> R)-> ptrDel.del = NODE_DELETE_FALSE;//20/8
			
			//the left pointer of the new node will point to the node to the left of SR
			((node*)((int)newL -> ptr))-> L ->ptr = ((int)oldL->ptr);//20/8
			((node*)((int)newL ->ptr))-> value = v;//20/8
			
			oldLR -> ptr = (int)SR;
			oldLR -> ptrDel.del = NODE_DELETE_FALSE;
			//if true insert has bin complete
			if(mDcas((void**)&SR-> L, (void**)&(((node*)((int)oldL-> ptr))-> R -> ptr),
								oldL, (void*)oldLR -> ptr,
								newL,(void*) newL-> ptr))//20/8
				break;
		}
	}

} 

void pushLeftB(int v)
{
	PointerContainer* oldR;
	PointerContainer* oldRL = MallocPNode();
	
	//create a new node
	PointerContainer* newR = MallocPNode();
	newR-> ptr = (int)NodeMalloc();
	
	//alloction failed memory full
	/*
	if (newR -> ptr == 0)
	{
		printf("Memory is full\n");
		return;
	}
	*/
	newR -> ptrDel.del = NODE_DELETE_FALSE;
	while(1)
	{	
		oldR = SL -> R;
		//if true exeutes physicall deletion
		if (oldR -> ptrDel.del == NODE_DELETE_TRUE)
			deleteLeft();
		else
		{	
			//the right pointer of the new node will point to SR
			(((node*)((int)newR -> ptr))-> L)-> ptr = (int)SL;//20/8
			(((node*)((int)newR -> ptr))-> L)-> ptrDel.del = (int)NODE_DELETE_FALSE;//20/8
				
			//the left pointer of the new node will point to the node to the left of SR
			((node*)((int)newR -> ptr))-> R ->ptr = ((int)oldR->ptr);//20/8
			((node*)((int)newR->ptr))-> value = v;//20/8
			
			oldRL -> ptr = (int)SL;
			oldRL -> ptrDel.del = NODE_DELETE_FALSE;
			
			//if true insert has bin complete
			if(mDcas((void**)&SL-> R,(void**) &((node*)((int)oldR-> ptr))-> L -> ptr,
						oldR, (void*)oldRL -> ptr,
						newR, (void*)newR  -> ptr))//20/8
				break;
		}
	}
	
}


//1    delete execute a deletion of one to two nodes
//2   delete will delete only one node if the node value to the left of the node that shoulde be deleted is not "null"
//3   delete will edlete two nodes if the node value to the left of the node that shoulde be deleted is "null" which indicates that the list should be empted
void deleteRight()
{
	
	PointerContainer* oldL;
	PointerContainer* oldLLR;
	PointerContainer* newR = MallocPNode();
	PointerContainer* newL = MallocPNode();
	node* oldLL;
	PointerContainer* oldR;
	

	while(1)
	{
		oldL = SR -> L;
		//if true the node wos deleted already
		if(oldL-> ptrDel.del == NODE_DELETE_FALSE)
			return;
		oldLL = ((node*)((int)(((node*)((int)GetRealAddr(oldL -> ptr))) -> L -> ptr)));//20/8
		//if true ther is one items that should  be removed 
		if(oldLL -> value != NODE_VALUE_NULL)
		{
			oldLLR = oldLL -> R;
			
			//if true both sides are pointing to the node that should  be removed
			if(GetRealAddr(oldL -> ptr) == GetRealAddr(oldLLR -> ptr))
			{
				//creates two new pointers and swaps them with the old ones
				newR -> ptr = (int)SR;//20/8
				newR -> ptrDel.del = NODE_DELETE_FALSE;
				newL -> ptr = (int)oldLL;//20/8
				newL -> ptrDel.del = NODE_DELETE_FALSE;
				
				//if true physicall delition has bin completed
				if(mDcas((void**)&SR -> L,(void**) &oldLL -> R,
						  oldL, (void*)oldLLR,
						  newL, (void*)newR))//20/8
					return;
			}
		}
		//ther are two null items which indicates that the list should be empted
		else
		{
			oldR = SL -> R;
			//indicates that the node to the right of SL should be deleted as well
			if(oldR -> ptrDel.del == NODE_DELETE_TRUE)
			{
				//creates two new pointers and swaps them with the old ones
				newL -> ptr = (int)SL;
				newL -> ptrDel.del = NODE_DELETE_FALSE;
				newR -> ptr = (int)SR;
				newR -> ptrDel.del = NODE_DELETE_FALSE;
				
				//if true physicall delition has bin completed and the list is empty
				if(mDcas((void**)&SR -> L, (void**)&SL ->R, oldL, (void*)oldR, newL , (void*)newR))//20/8
					return;
					
			}
		
		}
		

	}		
}

//1    delete execute a deletion of one to two nodes
//2   delete will delete only one node if the node value to the right of the node that shoulde be deleted is not "null"
//3   delete will edlete two nodes if the node value to the right of the node that shoulde be deleted is "null" which indicates that the list should be empted
void deleteLeft()
{
	
	
	PointerContainer* oldR;
	PointerContainer* oldRRL;
	PointerContainer* newL = MallocPNode();
	PointerContainer* newR = MallocPNode();
	node* oldRR;
	PointerContainer* oldL;
	

	while(1)
	{
		oldR = SL -> R;
		//if true the node wos deleted already
		if(oldR-> ptrDel.del == NODE_DELETE_FALSE)
			return;
		oldRR = ((node*)((int)(((node*)((int)GetRealAddr(oldR -> ptr))) -> R -> ptr)));//20/8
		//if true ther is one items that should  be removed 
		if(oldRR -> value != NODE_VALUE_NULL)
		{
			oldRRL = oldRR -> L;
				
			//if true both sides are pointing to the node that should  be removed
			if(GetRealAddr(oldR -> ptr) == GetRealAddr(oldRRL -> ptr)) 
			{
				//creates two new pointers and swaps them with the old ones
				newL -> ptr = (int)SL;
				newL -> ptrDel.del = NODE_DELETE_FALSE;
				newR -> ptr = (int)oldRR;
				newR -> ptrDel.del = NODE_DELETE_FALSE;
				
				//if true physicall delition has bin completed
				if(mDcas((void**)&SL -> R, (void**)&oldRR -> L, oldR, (void*)oldRRL, newR, (void*)newL))//20/8
					return;
			}
		}
		//ther are two null items which indicates that the list should be empted
		else
		{
			oldL = SR -> L;
			//indicates that the node to the left of SR should be deleted as well
			if(oldL -> ptrDel.del == NODE_DELETE_TRUE)
			{
				//creates two new pointers and swaps them with the old ones
				newR -> ptr = (int)SR;
				newR -> ptrDel.del = NODE_DELETE_FALSE;
				newL -> ptr = (int)SL;
				newL -> ptrDel.del = NODE_DELETE_FALSE;
				
				//if true physicall delition has bin completed and the list is empty
				if(mDcas((void**)&SL -> R, (void**)&SR ->L, oldR, (void*)oldL, newR , (void*)newL))//20/8
					return;
			} 
		
		}
		

	}		
}















void PrintLB(){
	
	printf("%d <=>",SL -> value);
	node* temp =(node*) ((int)GetRealAddr(SL -> R -> ptr));//20/8
	while(temp -> value != NODE_VALUE_SENR){
		printf("%d",RetInt(temp -> value));
			temp =(node*) ((int)temp -> R -> ptr);//20/8
			printf("<=>");
	}
	printf(" %d",SR -> value);
	printf("\n");
}





void PrintRB(){
	
	printf("%d <=>",SR -> value);
	node* temp =(node*) ((int)GetRealAddr(SR -> L -> ptr));//20/8
	while(temp -> value != NODE_VALUE_SENL){
		printf("%d",RetInt(temp -> value));
			temp =(node*) ((int)temp -> L -> ptr);//20/8
			printf("<=>");
	}
	printf(" %d",SL -> value);
	printf("\n");
}



//int main()
//{
	
//	Based_Initialization();
//	pushLeft(5);
//	pushLeft(4);
//	pushLeft(3);
//	PrintL();
//	//printf("Msb %d \n",CheckLSB(SR -> L ->ptr));
//	popLeft();
//	PrintL();
//	//printf("Msb %d \n",CheckLSB(SR -> L ->ptr));
//	popLeft();
//	PrintL();
//	popRight();
//	PrintL();
//	popRight();
//	PrintL();
//	printf("\n");
//	return 0;
//}
