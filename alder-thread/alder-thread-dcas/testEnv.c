//******************************************************************
//	File name: testEnv.c
//
//	Description: Test Enviroment (Function and special structs)
//
//	Modification list:
//	(Date:DD.MMM.YYYY) "-" DefectNo ":" (Fname.L) " " Description
//	23.Aug.2008 - #DEF#15 : Separate main test to test according their functionality
//******************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "testEnv.h"

void TestA(int id, int res)
{
	if (res)
		printf("TestNo.%d - \033[32mSuccedded!\033[0m\n",id);
	else
		printf("TestNo.%d - \033[31mFailed!\033[0m\n",id);
}

void WriteToThreadLog(const char* message)
{
	FILE* thread_log = (FILE*) pthread_getspecific(thread_log_key);
	fprintf(thread_log, "%s\n", message);
}


void CloseThreadLog( void* thread_log)
{
	fclose((FILE*) thread_log);
	printf("Closing File..\n");
}

// Get the current time in miliseconds
double GetCurrentTimeMSC()
{
	struct timeval timeStr;
	gettimeofday(&timeStr, NULL);
	return timeStr.tv_sec+(timeStr.tv_usec/1000000.0);
}
