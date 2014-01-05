/**
 * This file, alder_thread_stack.c, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <errno.h>
#include <string.h>
#include "alder_thread_stack.h"

#include <pthread.h>
#include <stdio.h>
#define NTHREADS 4
#define N 1000
#define MEGEXTRA 1000000

pthread_attr_t attr;

void *dowork(void *threadid)
{
    double A[N][N];
    int i,j;
    long tid;
    size_t mystacksize;
    
    pthread_attr_getstacksize (&attr, &mystacksize);
    tid = (long)threadid;
    printf("Thread %ld: stack size = %li bytes \n", tid, mystacksize);
    for (i=0; i<N; i++)
        for (j=0; j<N; j++)
            A[i][j] = ((i*j)/3.452) + (N-i);
    pthread_exit(NULL);
}

int alder_thread_stack()
{
    pthread_t threads[NTHREADS];
    size_t stacksize;
    int rc;
    long t;
    
    rc = pthread_attr_init(&attr);
    assert(rc == 0);
    rc = pthread_attr_getstacksize (&attr, &stacksize);
    assert(rc == 0);
    printf("Default stack size = %li\n", stacksize);
    stacksize = sizeof(double)*N*N+MEGEXTRA;
    printf("Amount of stack needed per thread = %li\n",stacksize);
    rc = pthread_attr_setstacksize (&attr, stacksize);
    if (rc != 0) {
        fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__,
                strerror(errno));
        assert(rc == 0);
    }
    assert(rc == 0);
    printf("Creating threads with stack size = %li bytes\n",stacksize);
    for(t=0; t<NTHREADS; t++){
        rc = pthread_create(&threads[t], &attr, dowork, (void *)t);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return 1;
        }
    }
    printf("Created %ld threads.\n", t);
    return 0;
}