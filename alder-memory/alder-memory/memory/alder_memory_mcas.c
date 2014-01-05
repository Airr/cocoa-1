/**
 * This file, alder_memory_mcas.c, is part of alder-memory.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-memory is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-memory is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-memory.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "alder_memory_mcas.h"

#pragma mark header

typedef struct alder_memory_node_struct {
    int mm_ref; /* initially 1 */
    struct alder_memory_node_struct *mm_next;
    int v;
} alder_memory_node_t;

typedef union alder_memory_linkorpointer_union {
    alder_memory_node_t **pnode;
    alder_memory_node_t *node;
} alder_memory_linkorpointer_t;

int alder_memory_mcas_initialize();
int alder_memory_mcas_finalize();
alder_memory_node_t* alder_memory_mcas_alloc();
void alder_memory_mcas_free(alder_memory_node_t* node);
alder_memory_node_t* alder_memory_mcas_fixref(alder_memory_node_t* node,
                                              int fix);
bool alder_memory_mcas_CompareAndSwapLink(alder_memory_node_t **link,
                                          alder_memory_node_t *old,
                                          alder_memory_node_t *new);
alder_memory_node_t* alder_memory_mcas_DeRefLink(alder_memory_node_t **link);
void alder_memory_mcas_ReleaseRef(alder_memory_node_t *node);
void alder_memory_mcas_HelpDeRef(alder_memory_node_t **link);

#pragma mark static
static void FAA(int *ptr, int value);
static void* SWAPvoid(void **ptr, void *value);
static alder_memory_node_t* SWAP(alder_memory_node_t **ptr, alder_memory_node_t *value);
static bool CAS(alder_memory_node_t **ptr, alder_memory_node_t *oldval, alder_memory_node_t *newval);
static bool CASvoid(void **ptr, void *oldval, void *newval);
static bool CASint(int *ptr, int oldval, int newval);

static void FAA(int *ptr, int value)
{
    __sync_fetch_and_add(ptr, value);
}

//static alder_memory_node_t* SWAP(alder_memory_node_t **ptr, alder_memory_node_t *value)
static void* SWAPvoid(void **ptr, void *value)
{
    return __sync_lock_test_and_set(ptr, value);
}

static alder_memory_node_t* SWAP(alder_memory_node_t **ptr, alder_memory_node_t *value)
{
    return __sync_lock_test_and_set(ptr, value);
}

static bool CAS(alder_memory_node_t **ptr, alder_memory_node_t *oldval, alder_memory_node_t *newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

static bool CASvoid(void **ptr, void *oldval, void *newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

static bool CASint(int *ptr, int oldval, int newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

#pragma mark global

#define NR_THREADS 10
#define ALDER_MEMORY_MAX_TRY 5

int currentFreeList;
alder_memory_node_t *freeList[NR_THREADS*2];
int helpCurrent;
alder_memory_node_t *annAlloc[NR_THREADS];
void *annReadAddr[NR_THREADS][NR_THREADS];
int annIndex[NR_THREADS];
int annBusy[NR_THREADS][NR_THREADS];

#pragma mark local
//int threadId;
//alder_memory_node_t *n1;
//alder_memory_node_t *n2;
//alder_memory_node_t *node;
//int gindex;
//int id;

#pragma mark fuctions

/* This function dereferences a given link (or &node or pointer to pointer).
 * It returns the corresponding address of a node with reference count
 * incremented.
 * 
 * What to do with the union?
 * D1~D3: Announce the link in a free shared variable.
 *        Choose index such that annBusy[threadId][index]=0.
 *        So, I would choose an index that is not busy in the current thread.
 *        Set the annIndex to the index.
 *        Set the announce address to the link.
 *        I still do not know what I am doing???
 *
 * D4: Dereference the link.
 * D5: Increase the reference count of the corresponding node.
 * D6: Remove the announcement, and "simultaneously detect a possible answer
 *     from helping."
 * D7: An anser was received.
 * D8: Decrement the reference count of the possibly reclaimed node by calling
 *     "ReleaseRef."
 * D9: Use the answer (n1) for returning.
 * D10:
 */
alder_memory_node_t* alder_memory_mcas_DeRefLink(alder_memory_node_t **link)
{
    /* Local variables */
    int threadId = -1;
    assert(!(threadId < 0));
    void *n1; /* an answer */
    alder_memory_node_t *node; /* return */
    int idx;
    
    /* D1 */
    // D1. Choose index such that annBusy[threadId][index]=0
    for (idx = 0; idx < NR_THREADS; idx++) {
        if (annBusy[threadId][idx] == 0) {
            break;
        }
    }
    assert(idx != NR_THREADS);
    /* D2 */
    annIndex[threadId] = idx;
    /* D3 */
    annReadAddr[threadId][idx] = (void*)link;
    /* D4 */
    node = *link;
    /* D5 */
    if (node != NULL) FAA(&node->mm_ref, 2);
    /* D6??? */
    n1 = SWAPvoid(&annReadAddr[threadId][idx], NULL);
    /* D7 */
    if (n1 != (void*)link) {
        /* D8 */
        if (node != NULL) alder_memory_mcas_ReleaseRef(node);
        /* D9 */
        node = (alder_memory_node_t*)n1;
    }
    /* D10 */
    return node;
}

/* This function decrements the reference count of a given node.
 * If all references to the node has been removed, the node will eventually be 
 * reclaimed (by either this operation  or by pending concurrent ReleaseRef
 * operations).
 *
 * R1: Decrement the reference count.
 * R2: Detect whether the reference count is 0, and try to agree on whether
 *     this function should relaim the node.
 * R3: Release all references from links that is contained in the structure of
 *     this node.
 * R4: Reclaim the node.
 */
void alder_memory_mcas_ReleaseRef(alder_memory_node_t *node)
{
    /* R1 */
    FAA(&node->mm_ref, -2);
    /* R2 */
    if (node->mm_ref == 0 && CASint(&node->mm_ref, 0, 1)) {
        /* R3 */
        /* R3. Recursively call ReleaseRef for all the references held by the node. */
        /* I may not need this line because the node might not have any reference. */
        /* R4 */
        alder_memory_mcas_free(node);
    }
}

/* This function helps concurrent DeRefLink by giving them recent results of
 * dereferencing the corresponding given link.
 * 
 * H1~H3: Check the announcement variables to detect if the given link matches
 *        the announcement.
 * H4: Increase the busy count to make sure that this announcement variable
 *     cannot be reused for subsequent announcements.
 * H5: Get the recent dereference result of the link.
 * H6: Check if it fails to answer the announcement usin CAS.
 * H7: Decrement the reference count of the node.
 * H8: Release the claim on the announcing variable by decrementing the busy
 *     count.
 */
void alder_memory_mcas_HelpDeRef(alder_memory_node_t **link)
{
    /* Local variables */
    alder_memory_node_t *node;
    int idx;
    
    /* H1 */
    for (int id = 0; id < NR_THREADS-1; id++) {
        /* H2 */
        idx = annIndex[id];
        /* H3 */
        if (annReadAddr[id][idx] == (void*)link) {
            /* H4 */
            FAA(&annBusy[id][idx], 1);
            /* H5 */
            node = alder_memory_mcas_DeRefLink(link);
            /* H6 */
            if (!CASvoid(&annReadAddr[id][idx], (void*)link, (void*)node)) {
                /* H7 */
                if (node != NULL) {
                    alder_memory_mcas_ReleaseRef(node);
                }
            }
            /* H8 */
            FAA(&annBusy[id][idx], -1);
        }
    }
}

/* This function must be called before calling other functions in this module.
 */
int alder_memory_mcas_initialize()
{
    /* FIXME: Initially freeList[0] points to the linked list of all available Nodes */
    currentFreeList = 0;
    helpCurrent = 0;
    
//    freeList [NR_THREADS*2];
    memset(freeList, 0, sizeof(freeList));
    memset(annAlloc, 0, sizeof(annAlloc));
    for (int i = 0; i < NR_THREADS; i++) {
        for (int j = 0; j < NR_THREADS; j++) {
            annBusy[i][j] = 0;
        }
    }
    
    return 0;
}

int alder_memory_mcas_finalize()
{
    return 0;
}

alder_memory_node_t* alder_memory_mcas_fixref(alder_memory_node_t* node,
                                              int fix)
{
    FAA(&node->mm_ref,fix);
    return node;
}

/* This function returns a newly allocated node with a reference count
 * indicating one active reference.
 * All threads share a variable to announce the need of a free node.
 * Before a successful CAS attempt to retrieve a node from the free-list,
 * The alloc and free operation has to help another thread.
 * The specific process to help is incremented in a round-robin fashion,
 * so that eventually every process will have some help.
 * In order to avoid conflicts with concurrent alloc operations, all alloc
 * operations operate on the same free-list, thus always leaving one free-lis
 * free of conflicts for the corresponding free operation.
 *
 * A1~A2: Choose a specific thread to help.
 * A3: Repeat checking whether the current `thread was already helped.
 * A4: Check whether the current thread was already helped.
 *     SWAP(&annAlloc[threadId], NULL) 
 *     -> A = SWAP(&B,C) means A = B, B = C.
 *     -> Reset the announcement variable.
 *     return alder_memory_mcas_fixref(SWAP(&annAlloc[threadId], NULL), -1)
 *     -> Return the node with adjustment of the reference count.
 * A5: The current thread is not yet helped.
 * A6: Try to retrieve the first node in the active free-list.
 * A7: Check if the current free-list is empty, and, if so, change the active
 *     free-list to the next free-list in order.
 * A8: The active free-list is not empty.
 * A9: Increase the reference count of the first node in the free-list.
 * A10: Try to remove the node from the free-list.
 * A11: Decide whether to use the removed node, or to give it to the thread
 *      targeted for helping.
 * A12: ...
 * A13~A15: The targeted thread has got the removed node, and the algorithm
 *          makdes another try to allocate a new node.
 * A16 (and A14): We know that the targeted thread has been helped in some way.
 *                So, increase the helpCurrent variable so that other threads
 *                will eventually get helped by subsequent AllocNode or
 *                FreeNode operation invocations.
 * A17: Return the node with adjusted reference count if the retrieved node
 *      was not given to the targed thread. So, we take this node.
 * A18: Call ReleaseRef on the node that failed to be retrieved.
 */
alder_memory_node_t* alder_memory_mcas_alloc()
{
    /* Local variables */
    int threadId = -1;
    assert(!(threadId < 0));
    alder_memory_node_t *node = NULL;
    int tries = 0;

    /* A1 */
    bool helped = false;
    /* A2 */
    int helpId = helpCurrent;
    /* A3 */
    while (true) {
        tries++;
        if (tries > ALDER_MEMORY_MAX_TRY) {
            // Memory is exhausted.
            // Allocate the memory more.
            assert(0);
        }
        /* A4 */
        if (annAlloc[threadId] != NULL) {
            return alder_memory_mcas_fixref((alder_memory_node_t*)SWAP(&annAlloc[threadId], NULL), -1);
        }
        /* A5 */
        int current = currentFreeList;
        /* A6 */
        node = freeList[current];
        /* A7 */
        if (node == NULL) {
            CASint(&currentFreeList, current, (current+1)%(NR_THREADS*2));
        }
        /* A8 */
        else {
            /* A9 */
            FAA(&node->mm_ref, 2);
            /* A10 */
            if (CAS(&freeList[current], node, node->mm_next)) {
                /* A11 */
                /* If the thread to be helped has not been helped, */
                if (!helped && annAlloc[helpId] == NULL) {
                    /* A12 */
                    if (CAS(&annAlloc[helpId], NULL, node)) {
                        /* A13 */
                        helped = true;
                        /* A14 */
                        CASint(&helpCurrent, helpId, (helpId+1)%NR_THREADS);
                        /* A15 */
                        continue;
                    }
                }
                /* A16 */
                CASint(&helpCurrent, helpId, (helpId+1)%NR_THREADS);
                /* A17 */
                return alder_memory_mcas_fixref(node, -1);
            }
            /* A18 */
            else {
                alder_memory_mcas_ReleaseRef(node);
            }
        }
    }
}

/* This function frees the given node.
 * Before conflicting with CAS attempts on the free-list, each free operation
 * "ALSO (just like alloc operation)" has to possibly help another thread with 
 * allocation.  In order to avoid conflicts with concurrent free operations,
 * each thread manages two separate free-lists.
 *
 * F1: Select a thread to eventually help.
 * F2: Increase helpCurrent in a round robin fashion so that 
 *     other threads will eventually get helped by AllocNode or FreeNode.
 * F3:
 * F4: Decide which of the two free-lists is likely not to conflict with
 *     concurrent AllocNode invocation.
 * F5: ...
 * F6: ...
 * F7: Repeat inserting the node to the free-list.
 * F8:
 * F9: Break (finish the function) if the insertion succeeds.
 * F10: Choose another index if the insertion fails.
 */
void alder_memory_mcas_free(alder_memory_node_t* node)
{
    /* Local variables */
    int threadId = -1;
    assert(!(threadId < 0));
    int idx;
    
    /* F1 */
    int helpId = helpCurrent;
    /* F2 */
    CASint(&helpCurrent, helpId, (helpId+1)%NR_THREADS);
    /* F3 */
    if (CAS(&annAlloc[helpId], NULL, node)) {
        // The targeted thread has been helped.
        // This function finishes.
    }
    /* F3? */
    else
    {
        /* F4 */
        int current = currentFreeList;
        /* F5 */
        if (current <= threadId || current > (NR_THREADS+threadId)) {
            idx = NR_THREADS + threadId;
        }
        /* F6 */
        else {
            idx = threadId;
        }
        /* F7 */
        while (true) {
            /* F8 */
            node->mm_next = freeList[idx];
            /* F9 */
            if (CAS(&freeList[idx], node->mm_next, node)) {
                break;
            }
            /* F10 */
            idx = (idx+NR_THREADS)%(NR_THREADS*2);
        }
    }
}

bool alder_memory_mcas_CompareAndSwapLink(alder_memory_node_t **link,
                                          alder_memory_node_t *old,
                                          alder_memory_node_t *new)
{
    if (CAS(link, old, new)) {
        alder_memory_mcas_HelpDeRef(link);
        return true;
    }
    return false;
}

int alder_memory_mcas()
{
    return 0;
}
