/**
 * This file, alder_thread_sundell.c, is part of alder-thread.
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
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include "alder_cmacro.h"
#include "alder_mcas.h"
#include "alder_memory.h"

static word_t *x;
static void *worker(void *t);

int alder_thread_sundell()
{
    int n_thread = 2;
    int n_index = 2;
    /* Initialize the Sundell's memory manage. */
    alder_memory_sundell_initialize(n_thread, n_index);
    /* Create buffers */
    x = malloc(sizeof(*x)*n_index);
    for (int i = 0; i < n_index; i++) {
        x[i] = 1;
    }
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_thread; i++) {
        pthread_create(&threads[i], &attr, worker, (void*)((intptr_t)i));
    }
    for (int i = 0; i < n_thread; i++) {
        pthread_join(threads[i], NULL);
    }
    printf ("Main(): Waited on %d threads. Done. \n", n_thread);
    for (int i = 0; i < n_index; i++) {
        fprintf(stdout, "[%d] %llu\n", i, x[i]);
    }
    pthread_attr_destroy(&attr);
    XFREE(threads);
    XFREE(x);
    alder_memory_sundell_finalize(n_thread, n_index);
    return 0;
}

#pragma mark thread

static void *worker(void *t)
{
    int i_worker = (int)t;
    printf("Starting worker(%d): thread.\n", i_worker);
    for (int i = 0; i < 100000; i++) {
        bool succeeded = false;
        while (succeeded == false) {
            uint64_t oldvalue[2];
            uint64_t newvalue[2];
            oldvalue[0] = alder_mcas_sundell_Read(&x[0], i_worker);
            oldvalue[1] = alder_mcas_sundell_Read(&x[1], i_worker);
            newvalue[0] = oldvalue[0] + 1;
            newvalue[1] = oldvalue[1] + 1;
            succeeded = alder_mcas_sundell_CASN(2, &x, oldvalue, newvalue, i_worker);
        }
    }
    printf("worker(%d): exiting\n", i_worker);
    pthread_exit(NULL);
}