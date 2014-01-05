//******************************************************************
//	File name: testPerformance.c
//
//	Description: Perform final performance test
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "commonStruct.h"
#include "testPerformance.h"
#include "cas.h"
#include "dcas.h"
#include "simplerDcas.h"
#include "pointerCont.h"
#include "DCASBased.h"
#include "testEnv.h"
#include "actionFile.h"


void* ThreadAlgoExec(void* desc)
{
	// Given pointer is ActionListHeaderDesc* type
    ActionListHeaderDesc* pActionList = (ActionListHeaderDesc*) desc;
    ActionListDesc* pCurrentAction = &(pActionList->sHead);
    // Get actionlist from invoker - add into the local thread stack pointer to the actionList
    while(pCurrentAction != NULL) 
    {
        switch(pActionList->eAlgoType)
        {
        case DCAS_PROJECT_ALGO_SIMPLE:
            switch(pCurrentAction->node.eActionType) 
            {
            case ACTION_PUSH_LEFT:
                pushLeftS(pCurrentAction->node.unActionArgument);
                break;
            case ACTION_PUSH_RIGHT:
                pushRightS(pCurrentAction->node.unActionArgument);
                break;
            case ACTION_POP_LEFT:
                popLeftS();
                break;
            case ACTION_POP_RIGHT:
                popRightS();
                break;
            default:
                printf("ThreadAlgoExec - Error unknown type of action %d\n", pCurrentAction->node.eActionType); 
            }
            break;
        case DCAS_PROJECT_ALGO_BASE_CONCURRENCT:
            switch(pCurrentAction->node.eActionType) 
            {
            case ACTION_PUSH_LEFT:
                pushLeftB(pCurrentAction->node.unActionArgument);
                break;
            case ACTION_PUSH_RIGHT:
                pushRightB(pCurrentAction->node.unActionArgument);
                break;
            case ACTION_POP_LEFT:
                popLeftB();
                break;
            case ACTION_POP_RIGHT:
                popRightB();
                break;
            default:
                printf("ThreadAlgoExec - Error unknown type of action %d\n", pCurrentAction->node.eActionType); 
            }
            break;
        default:
            printf("ThreadAlgoExec - Error unknown type of algorithm\n");
        }
        pCurrentAction = pCurrentAction->next;
    }
	// for each action take one action and exceute it

	//printf("ThreadNumber: %d woke-up.\n", threadNumber);

	pthread_exit(0);
}



void PreformPerformanceTest(TestDescriptor* tDesc)
{
    int unNumberOfThreads = tDesc->unNumberOfThreads;
    pthread_t pthread_id_arr[unNumberOfThreads];
	int iret_arr[unNumberOfThreads];
    int returnCode = ACTION_RET_GEN_OK;
	int threadCounter = 0;
	FILE* recordFile;
	double dStartTime = 0.0, dEndTime = 0.0;
	char recordMsg[70];
	char fileName[20];
    // Generate ActionList base on TestDescriptor
	ActionListHeaderDesc sActionListHeaderDesc;
	ActionListHeaderDesc* pActionListHeaderDesc;
	
	// Open FileForRecord (note that it will append data)
	sprintf(fileName,"testTimeResult/Test-%d%d%d.csv",tDesc->unTestNumber,
										  tDesc->unNumberOfThreads,
										  tDesc->unNumberOfOperations);
    // Open File for append
	recordFile = fopen(fileName,"a");

	
	// Initiate ActionListHeader
	pActionListHeaderDesc = &sActionListHeaderDesc;
    // Initialize ActionList module
    GenerateInitialization();
	// Calculate real number of operations that needed to be made & generate actionList
	returnCode |= GenerateActionList( tDesc->unNumberOfOperations,
                        tDesc->unPushLeftPer,
                        tDesc->unPushRightPer,
                        tDesc->unPopLeftPer,
                        tDesc->unPopRightPer,
                        pActionListHeaderDesc);
    // Update Alogirthm type into the Descriptor
    pActionListHeaderDesc->eAlgoType = tDesc->eAlgoType;

    if(returnCode == ACTION_RET_GEN_OK) {
        //PrintActionList(&pActionListHeaderDesc->sHead);

        // Perform an initialization of algorithem depends on the algorithem type
        switch(tDesc->eAlgoType) {
        case DCAS_PROJECT_ALGO_SIMPLE:
            InitializationS();
            break;
        case DCAS_PROJECT_ALGO_BASE_CONCURRENCT:
            InitializationB();
            break;
        default:
            returnCode = -1;
        }
		dStartTime = GetCurrentTimeMSC();
		// Thread Handle
		for(threadCounter = 0; threadCounter < unNumberOfThreads; threadCounter++)
		{
			pthread_create(&(pthread_id_arr[threadCounter]),NULL, (void*) ThreadAlgoExec,(void*) pActionListHeaderDesc);
		}

		for(threadCounter = 0; threadCounter < unNumberOfThreads; threadCounter++)
		{
			iret_arr[threadCounter] = pthread_join(pthread_id_arr[threadCounter],NULL);
			//printf("Thread%d return with code %d \n", threadCounter ,iret_arr[threadCounter]);
		}
		dEndTime = GetCurrentTimeMSC();
        // write & display operation duration
        //printf("Op Duration: %f \n", dEndTime - dStartTime);
        switch(tDesc->eAlgoType) 
        {
        case DCAS_PROJECT_ALGO_SIMPLE:
            fprintf(recordFile,"%d,%d,%d,%d,%d,%d,%d,%f", tDesc->unTestNumber
                                               , tDesc->unNumberOfThreads
                                               , tDesc->unNumberOfOperations
                                               , tDesc->unPushLeftPer
                                               , tDesc->unPushRightPer
                                               , tDesc->unPopLeftPer
                                               , tDesc->unPopRightPer
                                               , (dEndTime - dStartTime));
            break;
        case DCAS_PROJECT_ALGO_BASE_CONCURRENCT:
            fprintf(recordFile,",%f\n", dEndTime - dStartTime);
            break;
        default:
            printf("PreformPerformanceTest - Error unknown type of algorithm");
            break;
        }
        
    }
    else
    {
        printf("PreformPerformanceTest: error - GenerateActionList return with error code\n");
    }

	// create as number of <tDesc->unNumberOfThreads> threads and send them actionList. 
	
	// close analysis file
	fclose(recordFile);
	return;
}

