/**
 * This file, main.c, is part of alder-thread-mcas-wf.
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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "alder_mcas_wf.h"


int num_threads;
int alder_thread_mcas_wf();
static void *mcas_worker(void *t);
#define X_SIZE 10
uint64_t *x;

int main(int argc, char * argv[])
{
    num_threads = 1;
    x = malloc(sizeof(*x)* X_SIZE);
    mcas_init(num_threads);
    
    /* Last thing that main() should do */
    alder_thread_mcas_wf();
    
    free(x);
    pthread_exit(NULL);
    return 0;
}

#pragma mark main

int alder_thread_mcas_wf()
{
    /* Create buffers */
    int n_thread = num_threads;
    for (int i = 0; i < X_SIZE; i++) {
        x[i] = 0;
    }
    
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    pthread_attr_t attr;
    
    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_thread; i++) {
        pthread_create(&threads[i], &attr, mcas_worker, (void*)i);
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < n_thread; i++) {
        pthread_join(threads[i], NULL);
    }
    printf ("Main(): Waited on %d threads. Done. \n", n_thread);
    for (int i = 0; i < X_SIZE; i++) {
        fprintf(stdout, "[%d] %llu\n", i, x[i]);
    }
    
    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    free(threads);
    
    return 0;
}

static void *mcas_worker(void *t)
{
    int i_worker = (int)t;
#define VALUE_SIZE 3
    uint64_t *oldA = malloc(sizeof(*oldA) * VALUE_SIZE);
    uint64_t *newA = malloc(sizeof(*newA) * VALUE_SIZE);
    uint64_t *resA = malloc(sizeof(*resA) * VALUE_SIZE);
    
    printf("Starting worker(%d): thread.\n", i_worker);
    for (int i = 0; i < 10000; i++) {
        BOOL succeeded = 0;
        memcpy(resA, oldA, sizeof(*resA) * VALUE_SIZE);
        resA[0]++;
//        while (succeeded == 0) {
        while (memcmp(resA, oldA, sizeof(*resA) * VALUE_SIZE)) {
            oldA[0] = (uint64_t)mcas_read_strong((void*)(x+2));
            oldA[1] = (uint64_t)mcas_read_strong((void*)(x+3));
            oldA[2] = (uint64_t)mcas_read_strong((void*)(x+4));
            memcpy(resA, oldA, sizeof(*resA) * VALUE_SIZE);
            // To use LSB
//            uint64_t new0 = old0 + 0x02;
//            uint64_t new1 = old1 + 0x02;
            // To use MSB
            newA[0] = oldA[0] + 1;
            newA[1] = oldA[1] + 2;
            newA[2] = oldA[2] + 1;
//            succeeded = call_mcas2(3, x + 2, oldA, newA);
            succeeded = call_mcas3(VALUE_SIZE, x + 2, oldA, newA, resA);
            
//            succeeded = call_mcas(1,
//                                  (void**)x, (void*)oldA[0], (void*)newA[0]);
//            succeeded = call_mcas(2,
//                                  (void**)x, (void*)oldA[0], (void*)newA[0],
//                                  (void**)(x + 1), (void*)oldA[1], (void*)newA[1]);
//            succeeded = call_mcas(2,
//                                  (void**)&x[0], (void*)old0, (void*)new0,
//                                  (void**)&x[1], (void*)old1, (void*)new1);
            
        }
    }
    printf("worker(%d): exiting\n", i_worker);
    free(resA);
    free(oldA);
    free(newA);
    pthread_exit(NULL);
}