/**
 * This file, alder_thread_argument.c, is part of alder-thread.
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

#include <stdio.h>
#include <pthread.h>
#include "alder_thread_argument.h"

#define NUM_THREADS 5

struct thread_data{
    int  thread_id;
    int  sum;
    char *message;
};

struct thread_data thread_data_array[NUM_THREADS];

void *PrintHello(void *threadarg)
{
    struct thread_data *my_data = (struct thread_data *) threadarg;
    int tid = my_data->thread_id;
    int sum = my_data->sum;
    printf("Hello World! It's me, thread #%d - sum %d!\n", tid, sum);
    pthread_exit(NULL);
}

int alder_thread_argument()
{
    pthread_t threads[NUM_THREADS];
    int rc;
    int t;
    for(t=0; t<NUM_THREADS; t++){
        printf("In main: creating thread %d\n", t);
        thread_data_array[t].thread_id = t;
        thread_data_array[t].sum = t + 10;
        rc = pthread_create(&threads[t], NULL, PrintHello, (void *) &thread_data_array[t]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return(-1);
        }
    }

    return 0;
}