/*
 * actionFile.c
 *
 *  Created on: Oct 14, 2008
 *      Author: Dotanitis
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "commonStruct.h"
#include "actionFile.h"


UINT32 GenerateActionList(	IN UINT32 numberOfOperations,
                          IN UINT32 pushLeftPer,
                          IN UINT32 pushRightPer,
                          IN UINT32 popLeftPer,
                          IN UINT32 popRightPer,
                          OUT ActionListHeaderDesc* actionListHeaderDesc)
{
	ActionDesc* actionDescP = &(actionListHeaderDesc->sHeader);
	ActionListDesc* actionListP = &(actionListHeaderDesc->sHead);
	int rc = ACTION_RET_GEN_NOT_OK;
	int startNumber = 0;
    
	// Check that the number of probabilities are 100 in summary.
	if (numberOfOperations == 0 ||
        (pushLeftPer + pushRightPer + popLeftPer + popRightPer) != 100)
	{
		printf("Error - GenerateActionList input mismatched\n");
		return ACTION_RET_GEN_NOT_OK;
	}
    
	// Return: actionDesc structure with the actual number of operation of pops and pushes.
	rc = CalculateActionNumbers(numberOfOperations, pushLeftPer, pushRightPer, popLeftPer, popRightPer, actionDescP);
	if (rc != ACTION_RET_GEN_OK)
	{
        printf("Error - CalculateActionNumbers error code returned from function GenerateActions\n");
        return ACTION_RET_GEN_NOT_OK;
	}
    
	//PrintActionDesc(actionDescP);
    
	startNumber = GenerateStartNumber(numberOfOperations);
    
	rc = GenerateActions(actionDescP, numberOfOperations, startNumber, actionListP);
    //printf("return pointer: %x \n",actionListP);									/*TODO: Debug only*/
    
	if (rc != ACTION_RET_GEN_OK)
	{
        printf("Error - GenerateActionList error code returned from function GenerateActions\n");
        return ACTION_RET_GEN_NOT_OK;
	}
	// Print list of actions
    //PrintActionList(actionListP);
	return ACTION_RET_GEN_OK;
}

void GenerateInitialization()
{
	unStartNumber = 0;
}

UINT32 GenerateStartNumber(int incNumber)
{
	unStartNumber += incNumber;
	return unStartNumber;
}

UINT32 GenerateActions(			IN ActionDesc* actionsStr,
                       IN UINT32 numberOfOperations,
                       IN UINT32 startNumber,
                       OUT ActionListDesc* actionListP)
{
	UINT32 opNum[NUMBER_OF_OPERATIONS] = {0, 0, 0, 0};
	UINT32 nextOp = 0;
	UINT32 currentNumber = startNumber;
	UINT32 actionNumber = 0;
	ActionListDesc* tail = NULL;
	UINT32 rc = ACTION_RET_GEN_NOT_OK;
	//ActionDesc* actionDescP = NULL;
	UINT32 operationCnt = 0;
    
	if (actionsStr == NULL)
	{
		return ACTION_RET_GEN_NOT_OK;
	}
    
	// Initialize random seed:
	srand( time(NULL) );
    
	// Variable initialization store variables locally
	opNum[ACTION_PUSH_LEFT] 	= actionsStr->unNumberPushR;
	opNum[ACTION_PUSH_RIGHT] 	= actionsStr->unNumberPushL;
	opNum[ACTION_POP_LEFT] 		= actionsStr->unNumberPopL;
	opNum[ACTION_POP_RIGHT]		= actionsStr->unNumberPopR;
    
	for (operationCnt = 0; operationCnt < numberOfOperations; operationCnt++)
	{
		// Do this operations $numberOfOperations times
		// Generate number [0,3]
		do
		{
			nextOp = rand() % NUMBER_OF_OPERATIONS;
		}while (opNum[nextOp] == 0);
        
		// Call AddOperation that update actionListP
		// This method will add a record into the list and add it using the nextOp number
		// Given as an argument.
		rc = AddOperation(
                          opNum, /* array of the 4 operations that can be made */
                          actionNumber /* the number of operation */,
                          nextOp /* actual operation to be made */,
                          currentNumber /* number given as an argument */,
                          actionListP /* the list data stucture */,
                          &tail /* points into the last node of the list */);
        
        //printf("%d:Return value of actionP and tail: (%x, %x) \n",
        //        actionNumber,actionListP, tail);
        
		if (rc != ACTION_RET_GEN_OK)
		{
		    printf("ERROR: return value is not ok (GenerateActions)\n");
			return ACTION_RET_GEN_NOT_OK;
		}
        
		// increment actionNumber and currentNumber
		actionNumber++;
		currentNumber++;
		// decrement the operation from array
		opNum[nextOp]--;
	}
    
	//actionListP =
    
    /*
     switch(nextOp)
     {
     case ACTION_PUSH_LEFT:
     break;
     case ACTION_PUSH_RIGHT:
     break;
     case ACTION_POP_LEFT:
     break;
     case ACTION_POP_RIGHT:
     break;
     default:
     printf("Error - GenerateActions got unexpected value in nextOp variable");
     break;
     }
     */
	return ACTION_RET_GEN_OK;
}

UINT32 AddOperation(
                    IN UINT32* opNumArr,
                    IN UINT32 actionNumber,
                    IN UINT32 nextOp,
                    IN UINT32 currentNumber,
                    OUT ActionListDesc* actionListP,
                    OUT ActionListDesc** tail)
{
	ActionListDesc* tailP = *tail;
	ActionListDesc* tempListP = NULL;
    
	if (tailP == NULL)
	{
		actionListP->node.unActionNumber = actionNumber;
		actionListP->node.unActionArgument = currentNumber;
		actionListP->node.eActionType = nextOp;
        
		actionListP->next = NULL;
		// Set tail pointer to pointer on actionListP
        
		*tail = actionListP;
	}
	else
	{
		tempListP = malloc(sizeof(ActionListDesc));
		if (NULL == tempListP)
		{
			return ACTION_RET_GEN_NOT_OK;
		}
        
		// Fill tempListP
		tempListP->node.unActionNumber = actionNumber;
		tempListP->node.unActionArgument = currentNumber;
		tempListP->node.eActionType = nextOp;
		tempListP->next = NULL;
        
		// Set Tail to point on tempListP
		tailP->next = tempListP;
		*tail = tailP->next;
	}
    //Update the actionListP
	//actionListP = tempListP;
    
	return ACTION_RET_GEN_OK;
}

void PrintActionDesc(IN ActionDesc* actionsStr)
{
	printf("Number of PushLeft:%d\n",actionsStr->unNumberPushL);
	printf("Number of PushRight:%d\n",actionsStr->unNumberPushR);
	printf("Number of PopLeft:%d\n",actionsStr->unNumberPopL);
	printf("Number of PopRight:%d\n",actionsStr->unNumberPopR);
	return;
}

void PrintActionList(ActionListDesc* list)
{
    ActionListDesc* listTravler;
    if (NULL == list)
    {
        printf("Empty list.\n");
        return;
    }
    listTravler = list;
 	printf("ActionNumber,ActionType,ActionArgument\n");
    do
    {
        printf("%d,%d,%d\n",listTravler->node.unActionNumber,
               listTravler->node.eActionType,
               listTravler->node.unActionArgument);
        listTravler = listTravler->next;
    }while (listTravler != NULL);
	return;
}

UINT32 CalculateActionNumbers(IN UINT32 oNum,
                              IN UINT32 PLPer,
                              IN UINT32 PRPer,
                              IN UINT32 POLPer,
                              IN UINT32 PORPer,
                              OUT ActionDesc* actionDesc)
{
	if (actionDesc == NULL)
		return ACTION_RET_GEN_NOT_OK;
	
	actionDesc->unNumberPushL 	= oNum * PLPer/100.0;
	actionDesc->unNumberPushR 	= oNum * PRPer/100.0;
	actionDesc->unNumberPopL 	= oNum * POLPer/100.0;
	actionDesc->unNumberPopR	= oNum * PORPer/100.0;
    
	if ((actionDesc->unNumberPopL + actionDesc->unNumberPopR + actionDesc->unNumberPushL + actionDesc->unNumberPushR) != oNum)
	{
		return ACTION_RET_GEN_NOT_OK;
	}
	else
	{
		return ACTION_RET_GEN_OK;
	}
}




