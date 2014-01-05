/**
 * This file, main.c, is part of alder-thread.
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "dcas.h"
#include "commonStruct.h"
#include "mytest1.h"
//#define DARWIN
//#include <unistd.h>
//#include <stdbool.h>
//#include "portable_defns.h"
//#include "mcas.c"

int num_threads;
int alder_mcas_thread_casn();
static void *mcas_worker(void *t);
static uint64_t x[2];
//static uint32_t x[2];

int main(int argc, char * argv[])
{
    num_threads = 4;
    if (argc == 1) {
        alder_mcas_thread_casn();
    } else if (argc == 2) {
        mytest1();
    }
    
    pthread_exit(NULL);
    return 0;
}

#pragma mark main

int alder_mcas_thread_casn()
{
//    mcas_init();
    
    fprintf(stdout, "%s %d %s\n", __FILE__, __LINE__, __FUNCTION__);
    
    /* Create buffers */
    int n_thread = num_threads;
    for (int i = 0; i < 4; i++) {
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
    for (int i = 0; i < 2; i++) {
        fprintf(stdout, "[%d] %llu\n", i, x[i] >> 2);
    }
    
    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    free(threads);
    
    return 0;
}

#pragma mark Thread

static void *mcas_worker(void *t)
{
    int i_worker = (int)t;
    printf("Starting worker(%d): thread.\n", i_worker);
    //    alder_thread_casn_rdcss_t *rdcss = malloc(sizeof(*rdcss));
    //    assert(rdcss != NULL);
    //    alder_thread_casn_t *casn = malloc(sizeof(*casn));
    //    uint64_t *old = malloc(sizeof(*old)*2);
    //    uint64_t *n = malloc(sizeof(*n)*2);
    //    casn->s = 2;
    //    casn->old = old;
    //    casn->n = n;
    //    casn->rdcss = rdcss;
    //    ptst_t *ptst;
    //    ptst = critical_enter();
    for (int i = 0; i < 1000000; i++) {
//    for (int i = 0; i < 100000; i++) {
        bool succeeded = false;
        while (succeeded == false) {
            
            //            casn->status = ALDER_THREAD_CASN_UNDECIDED;
            //            casn->addr = x;
            //            casn->old[0] = alder_thread_casn_CASNRead(&x[0]);
            //            casn->old[1] = alder_thread_casn_CASNRead(&x[1]);
            //            casn->n[0] = casn->old[0] + 1;
            //            casn->n[1] = casn->old[1] + 1;
            //            succeeded = alder_thread_casn_CASN(casn);
            // Change this line.
            
            uint64_t *a = &x[0];
            uint64_t *b = &x[1];
            uint64_t olda = DCASRead(a);
            uint64_t oldb = DCASRead(b);
//           	uint64_t olda = *a >> 2;
//            uint64_t oldb = *b >> 2;
            uint64_t newa = olda + 4;
            uint64_t newb = oldb + 4;
            DCASDescriptor_t* ds = CreateDCASDescriptor(a, olda, newa,
                                                        b, oldb, newb);
            succeeded = CAS2(ds);
            
//            succeeded = mcas(2,
//                             (void**)&x[0], (void*)x[0], (void*)(x[0] + 1),
//                             (void**)&x[1], (void*)x[1], (void*)(x[1] + 1));
        }
    }
    //    critical_exit(ptst);
    
    printf("worker(%d): exiting\n", i_worker);
    //    XFREE(rdcss);
    //    XFREE(old);
    //    XFREE(n);
    //    XFREE(casn);
    pthread_exit(NULL);
}