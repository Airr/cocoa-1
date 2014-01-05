#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
//#include "DCASBased.h" 
#include "simplerDcas.h"
#include "pointerCont.h"

void *print_message_function( char* );

//push Right
void *print_message_function1( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread ";
	
	//write
	                   //size
	if(fwrite(message, 29, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	pushRight(StoreInt(i));
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	}
	
	return 0;
}

//push Left
void *print_message_function2( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	pushLeft(StoreInt(i));
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	//size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	}
	return 0;
}

//pop Left
void *print_message_function3( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	popLeft();
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	}
	return 0;
}
//pop Right
void *print_message_function4( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	popRight();
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}

	
	}
	return 0;
}

//push Right and Left
void *print_message_function5( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	pushRight(i);
	pushLeft(i);
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}

	
	}
	return 0;
}

//pop Right and pop left
void *print_message_function6( char* name)
{
FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	popRight();
	popLeft();
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	}
	return 0;
}

void *print_message_function10( char* name)
{
	FILE* fp; 
   int i;
   //open file
   if((fp = fopen(name,"w+")) == 0)
	{
		printf("Cannot open file.\n");
		exit(1);
	}
	
	char * message = "this is atest using 2 thread";
	
	//write
	                   //size
	if(fwrite(message, 28, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	//print
	int letter;
	while( ( letter = fgetc( fp ) ) != EOF)
		printf("%c",letter);
		
	//clos
	struct timeval tim;
    char messageTime [21];         
	
	for(i = 0; i < 300 ;i++)
	{
	gettimeofday(&tim, NULL);
	double t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	pushLeft(i);
	gettimeofday(&tim, NULL);
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	
	sprintf(messageTime,"action %d : %.6lf\n",i, t2 - t1);
	//printf("\n %s \n",messageTime);
	//write
	                   //size
	if(fwrite(messageTime,21, 1, fp) != 1)
	{
    printf("Write error occurred.\n");
    exit(1);
	}
	
	}
	return 0;
}



// pthread.c - Thread calling in order to run the Dequeue algorithems in parallel 
// This is a skeleton for running the algorithems

int main()
{
	 pthread_t thread1, thread2;//, thread3, thread4, thread5, thread6, thread7, thread8;
	 char *message1 = "Thread 1";
     char *message2 = "Thread 2";
     /* char *message3 = "Thread 3";
     char *message4 = "Thread 4";
	 char *message5 = "Thread 5";
     char *message6 = "Thread 6";
	 char *message7 = "Thread 7";
     char *message8 = "Thread 8"; */
	 int  iret1, iret2;//, iret3, iret4, iret5, iret6, iret7, iret8;
     Initialization();
     
	/* Create independent threads each of which will execute function */
	
     char *Fname1 = "Fthread1.txt";
	 char *Fname2 = "Fthread2.txt";
	 /* char *Fname3 = "Fthread3.txt";
	 char *Fname4 = "Fthread4.txt";
	 char *Fname5 = "Fthread5.txt";
	 char *Fname6 = "Fthread6.txt";
	 char *Fname7 = "Fthread7.txt";
	 char *Fname8 = "Fthread8.txt";
	  */
	 
	 iret1 = pthread_create( &thread1, NULL,(void*) print_message_function1, Fname1);
     iret2 = pthread_create( &thread2, NULL,(void*) print_message_function2, Fname2);
	/*  iret3= pthread_create( &thread3, NULL,(void*) print_message_function3, Fname3);
     iret4= pthread_create( &thread4, NULL,(void*) print_message_function4, Fname4);
	 iret5 = pthread_create( &thread5, NULL,(void*) print_message_function5, Fname5);
     iret6= pthread_create( &thread6, NULL,(void*) print_message_function2, Fname6);
	 iret7 = pthread_create( &thread7, NULL,(void*) print_message_function1, Fname7);
     iret8 = pthread_create( &thread8, NULL,(void*) print_message_function6, Fname8);
	  */
	 
     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     
     //pthread_join( thread8, NULL); 
	 //pthread_join( thread7, NULL);
	 /* pthread_join( thread6, NULL); 
	 pthread_join( thread5, NULL);
	 pthread_join( thread4, NULL); 
	 /pthread_join( thread3, NULL);*/
	 pthread_join( thread2, NULL); 
	 pthread_join( thread1, NULL);
	 
     printf("%s returns: %d\n",message1 ,iret1);
     printf("%s returns: %d\n",message2, iret2);
	 /* printf("%s returns: %d\n",message3 ,iret3);
     printf("%s returns: %d\n",message4, iret4);
	 printf("%s returns: %d\n",message5 ,iret5);
     printf("%s returns: %d\n",message6, iret6);
	 printf("%s returns: %d\n",message7 ,iret7);
     printf("%s returns: %d\n",message8, iret8); */
    PrintL();

	return 0;
	 
	
}
