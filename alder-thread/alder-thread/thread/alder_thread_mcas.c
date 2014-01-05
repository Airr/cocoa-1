/**
 * This file, alder_thread_mcas.c, is part of alder-thread.
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

#include <gc.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "alder_cmacro.h"
#include "alder_thread_mcas.h"

#define GCX_MALLOC malloc
//#define GCX_MALLOC GC_MALLOC

static void *worker(void *t);

static void Complete(alder_thread_mcas_rdcss_t *d);
static bool isDescriptor(uint64_t val);
static bool isMCASDescriptor(uint64_t val);


static uint64_t x[4];

#pragma mark main

int alder_thread_mcas()
{
    fprintf(stdout, "%s\n", __FUNCTION__);
    
    /* Create buffers */
    int n_thread = 2;
    for (int i = 0; i < 4; i++) {
        x[i] = 0;
    }
    
    
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    pthread_attr_t attr;
    
    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_thread; i++) {
        pthread_create(&threads[i], &attr, worker, (void*)(intptr_t)i);
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < n_thread; i++) {
        pthread_join(threads[i], NULL);
    }
    printf ("Main(): Waited on %d threads. Done. \n", n_thread);
    for (int i = 0; i < 4; i++) {
        fprintf(stdout, "[%d] %llu\n", i, x[i]);
    }
    
    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    XFREE(threads);
    
    return 0;
}

#pragma mark Thread

static void *worker(void *t)
{
    int i_worker = (int)t;
    printf("Starting worker(%d): thread.\n", i_worker);
//    alder_thread_mcas_rdcss_t *rdcss = malloc(sizeof(*rdcss));
//    assert(rdcss != NULL);
    alder_thread_mcas_t *mcas;
//    alder_thread_mcas_t *mcas = malloc(sizeof(*mcas));
//    uint64_t *old = malloc(sizeof(*old)*2);
//    uint64_t *n = malloc(sizeof(*n)*2);
//    mcas->s = 2;
//    mcas->old = old;
//    mcas->n = n;
//    mcas->rdcss = rdcss;
    
    for (int i = 0; i < 1000; i++) {
        bool succeeded = false;
        while (succeeded == false) {
            
            uint64_t old1 = alder_thread_mcas_MCASRead(&x[0]);
            uint64_t old2 = alder_thread_mcas_MCASRead(&x[1]);
            
            // Create mcas.
            mcas = GCX_MALLOC(sizeof(*mcas));
            mcas->status = ALDER_THREAD_MCAS_UNDECIDED;
            mcas->addr = x;
            mcas->s = 2;
            mcas->old = GCX_MALLOC(sizeof(*mcas->old)*2);
            mcas->n = GCX_MALLOC(sizeof(*mcas->n)*2);
            mcas->old[0] = old1;
            mcas->old[1] = old2;
            mcas->n[0] = old1 + 1;
            mcas->n[1] = old2 + 1;
            succeeded = alder_thread_mcas_MCAS(mcas);
            //
            
        }
    }
    
    printf("worker(%d): exiting\n", i_worker);
//    XFREE(rdcss);
//    XFREE(old);
//    XFREE(n);
//    XFREE(mcas);
    pthread_exit(NULL);
}

#pragma mark MCAS

bool alder_thread_mcas_MCAS(alder_thread_mcas_t *cd)
{
    uint64_t cd64 = (uint64_t)cd | ALDER_THREAD_MCAS_DESCRIPTOR_MCAS;
    if (cd->status == ALDER_THREAD_MCAS_UNDECIDED) {
        uint64_t status_undecided = ALDER_THREAD_MCAS_UNDECIDED;
        uint64_t status = ALDER_THREAD_MCAS_SUCCEEDED;
        int i_continue = 0;
        for (int i = 0; (i < cd->s) && (status == ALDER_THREAD_MCAS_SUCCEEDED); i++) {
            if (i_continue == 1) {
                i--;
                i_continue = 0;
            }
            
            alder_thread_mcas_rdcss_t *d = GCX_MALLOC(sizeof(*d));
            d->a1 = (uint64_t*)&cd->status;
            d->o1 = ALDER_THREAD_MCAS_UNDECIDED;
            d->a2 = &cd->addr[i];
            d->o2 = cd->old[i];
            d->n2 = cd64;
            uint64_t val64 = alder_thread_mcas_RDCSS(d);
//            free(d); d = NULL;
            if (isMCASDescriptor(val64)) {
                if (val64 != cd64) {
                    alder_thread_mcas_t *val = (alder_thread_mcas_t *)(val64 & ALDER_THREAD_MCAS_MAKE_DESCRIPTOR);
                    alder_thread_mcas_MCAS(val);
                    i_continue = 1;
                    continue;
                }
            } else if (val64 != cd->old[i]) {
                status = ALDER_THREAD_MCAS_FAILED;
            }
        }
        __sync_val_compare_and_swap((uint64_t*)(&cd->status), status_undecided, status);
    }
    bool succeeded = (cd->status == ALDER_THREAD_MCAS_SUCCEEDED);
    for (int i = 0; i < cd->s; i++) {
        uint64_t new_or_old;
        if (succeeded == true) {
            new_or_old = cd->n[i];
        } else {
            new_or_old = cd->old[i];
        }
        __sync_val_compare_and_swap(&cd->addr[i], cd64, new_or_old);
    }
    
    return succeeded;
}

uint64_t alder_thread_mcas_MCASRead(uint64_t *addr)
{
    uint64_t r64;
    do {
        r64 = alder_thread_mcas_RDCSSRead(addr);
        if (isMCASDescriptor(r64)) {
            alder_thread_mcas_t *r = (alder_thread_mcas_t *)(r64 & ALDER_THREAD_MCAS_MAKE_DESCRIPTOR);
            alder_thread_mcas_MCAS(r);
        }
    } while (isMCASDescriptor(r64));
    return r64;
}

static bool isMCASDescriptor(uint64_t val)
{
    if (val & ALDER_THREAD_MCAS_DESCRIPTOR_MCAS) {
        return true;
    } else {
        return false;
    }
}

#pragma mark DCSS

static bool isDescriptor(uint64_t val)
{
    if (val & ALDER_THREAD_MCAS_DESCRIPTOR_DCSS) {
        return true;
    } else {
        return false;
    }
}

uint64_t alder_thread_mcas_RDCSS(alder_thread_mcas_rdcss_t *d)
{
    uint64_t r64;
    uint64_t d64 = (uint64_t)d | ALDER_THREAD_MCAS_DESCRIPTOR_DCSS;
    do {
        r64 = __sync_val_compare_and_swap(d->a2, d->o2, d64);
        if (isDescriptor(r64)) {
            alder_thread_mcas_rdcss_t *r = (alder_thread_mcas_rdcss_t *)(r64 & ALDER_THREAD_MCAS_MAKE_DESCRIPTOR);
            Complete(r);
        }
    } while (isDescriptor(r64));
    if (r64 == d->o2) {
        Complete(d);
    }
    return r64;
}

uint64_t alder_thread_mcas_RDCSSRead(uint64_t *addr)
{
    uint64_t r64;
    do {
        r64 = *addr;
        if (isDescriptor(r64)) {
            alder_thread_mcas_rdcss_t *r = (alder_thread_mcas_rdcss_t *)(r64 & ALDER_THREAD_MCAS_MAKE_DESCRIPTOR);
            Complete(r);
        }
    } while (isDescriptor(r64));
    
    return r64;
}

static void Complete(alder_thread_mcas_rdcss_t *d)
{
    uint64_t v = *(d->a1);
    uint64_t d64 = (uint64_t)d | ALDER_THREAD_MCAS_DESCRIPTOR_DCSS;
    if (v == d->o1) {
        __sync_val_compare_and_swap(d->a2, d64, d->n2);
    } else {
        __sync_val_compare_and_swap(d->a2, d64, d->o2);
    }
}
