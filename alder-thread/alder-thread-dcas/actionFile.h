/*
 * actionFile.h
 *
 *  Created on: Oct 14, 2008
 *      Author: Dotanitis
 */

#ifndef ACTIONFILE_H_
#define ACTIONFILE_H_

//#define NULL 0
#define IN
#define OUT
#define UINT32 unsigned int
#define NUMBER_OF_OPERATIONS 4


typedef struct Action_desc{
	UINT32 unNumberPushL;
	UINT32 unNumberPushR;
	UINT32 unNumberPopL;
	UINT32 unNumberPopR;
}ActionDesc;



enum ACTION_TYPE_ENUM
{
	ACTION_MIN_VALUE 	= 0,
	ACTION_PUSH_LEFT 	= 0,
	ACTION_PUSH_RIGHT 	= 1,
	ACTION_POP_LEFT 	= 2,
	ACTION_POP_RIGHT 	= 3,
	ACTION_MAX_VALUE 	= 3
}Action_e;

enum ACTION_RETURN_CODE
{
	ACTION_RET_MIN_VALUE	=-1,
	ACTION_RET_GEN_NOT_OK	=-1,
	ACTION_RET_GEN_OK		=0,
	ACTION_RET_INV_OP		=1,
	ACTION_RET_MAX_VALUE	=1
}RetCode_e;

typedef struct ActionRec_desc{
	UINT32 		unActionNumber;
	UINT32		eActionType;
	UINT32 		unActionArgument;
} ActionRecDesc;

typedef struct ActionList_desc{
	ActionRecDesc node;
	struct ActionList_desc* next;
}ActionListDesc;

typedef struct ActionsListHead_desc{
	ActionDesc sHeader;
    eAlgorithmType eAlgoType;
	ActionListDesc sHead;
}ActionListHeaderDesc;

UINT32 unStartNumber; 

extern UINT32 GenerateActionList(
		IN UINT32 numberOfOperations,
		IN UINT32 pushLeftPer,
		IN UINT32 pushRightPer,
		IN UINT32 popLeftPer,
		IN UINT32 popRightPer,
		OUT ActionListHeaderDesc* actionListHeaderDesc
		);
extern UINT32 GenerateActions(
		IN ActionDesc* actionsStr,
		IN UINT32 numberOfOperations,
		IN UINT32 startNumber,
		OUT ActionListDesc* actionListP);

extern void PrintActionDesc(IN ActionDesc* actionsStr);
extern void GenerateInitialization();
extern UINT32 CalculateActionNumbers(IN UINT32 oNum,
							IN UINT32 PLPer,
							IN UINT32 PRPer,
							IN UINT32 POLPer,
							IN UINT32 PORPer,
							OUT ActionDesc* actionDesc);
extern UINT32 AddOperation(
		IN UINT32* opNumArr,
		IN UINT32 actionNumber,
		IN UINT32 nextOp,
		IN UINT32 currentNumber,
		OUT ActionListDesc* actionListP,
		OUT ActionListDesc** tail);

extern UINT32 GenerateStartNumber(int incNumber);
extern void PrintActionList(ActionListDesc* list);


#endif /* ACTIONFILE_H_ */
