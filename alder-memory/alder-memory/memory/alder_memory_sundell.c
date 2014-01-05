/**
 * This file, alder_memory_sundell.c, is part of alder-memory.
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
#include "alder_cmacro.h"
#include "alder_atomic.h"
#include "alder_type.h"
#include "alder_memory_sundell.h"

typedef alder_type_sundell_descriptor_t descriptor_t;

/* atomic */
#define FAA alder_atomic_FAAint
#define CASint alder_atomic_bool_CASint
#define CASdesc alder_atomic_CASdescriptor
#define CASword alder_atomic_CASword
#define SWAPdesc alder_atomic_SWAPdescriptor
#define SWAPword alder_atomic_SWAPword

/* type */
#define IS_REFERENCE alder_type_sundell_IS_REFERENCE
#define GET_DESCRIPTOR alder_type_sundell_GET_DESCRIPTOR

/* memory */
#define DeRefLink alder_memory_sundell_DeRefLink
#define HelpDeRef alder_memory_sundell_HelpDeRef
#define ReleaseRef alder_memory_sundell_ReleaseRef
#define AllocNode alder_memory_sundell_AllocNode
#define FreeNode alder_memory_sundell_FreeNode

#pragma mark static

static void alder_memory_sundell_FreeNode(descriptor_t* node, int threadId);
static descriptor_t* alder_memory_sundell_fixref(descriptor_t* node,
                                          int fix);

#pragma mark global

#define ALDER_MEMORY_MAX_TRY 5

static int currentFreeList;
static int helpCurrent;
static descriptor_t **annAlloc;
static word_t **annReadAddr;
static int *annIndex;
static int **annBusy;
static int sn_thread;
static int sn_index;
/* This would contain pointers to valid memory. */
static descriptor_t **freeList;
/* This contains all of the valid memory. */
static descriptor_t **headList = NULL;
static size_t headListSize = 0;
/* A fixed size of block for each free list in freeList. */
static size_t blocksize = 10;

#pragma mark freelist

/* This function initializes the headList.
 *
 * blocksize: number of descriptors for k-th freeList
 * k: k-th freeList
 */
static int headList_alloc(size_t k, size_t n_word)
{
    assert(freeList[k] == NULL);
    headListSize++;
    descriptor_t **t = realloc(headList, headListSize * sizeof(*t));
    if (t == NULL) {
        headListSize--;
        return 1;
    }
    headList = t;
    headList[headListSize - 1] = malloc(blocksize * sizeof(descriptor_t));
    if (headList[headListSize - 1] == NULL) {
        
        return 1;
    }
    // Allocate indices.
    descriptor_t *f = headList[headListSize - 1];
    memset(headList[headListSize - 1], 0, sizeof(descriptor_t)*blocksize);
    for (int i = 0; i < blocksize; i++) {
        f->mm_ref = 1;
        f->words = malloc(sizeof(*f->words)*n_word);
        if (f->words == NULL) {
            return 1;
        }
        memset(f->words, 0, sizeof(*f->words)*n_word);
        f++;
    }
    
    // Add the allocated memory to the freeList.
    f = headList[headListSize - 1];
    for (int i = 0; i < blocksize; i++) {
        f->mm_next = freeList[k];
        freeList[k] = f;
        f++;
    }
    return 0;
}

static void headList_free()
{
    for (size_t i = 0; i < headListSize; i++) {
        for (size_t j = 0; j < sn_index; j++) {
            XFREE(headList[i][j].words);
        }
        XFREE(headList[i]);
    }
    XFREE(headList);
}

#pragma mark init and final

/* This function must be called before calling other functions in this module.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_memory_sundell_initialize(int n_thread, int n_index)
{
    sn_thread = n_thread;
    sn_index = n_index;
    currentFreeList = 0;
    helpCurrent = 0;
    freeList = NULL;
    annAlloc = NULL;
    annIndex = NULL;
    annReadAddr = NULL;
    annBusy = NULL;
    
    freeList = malloc(sizeof(*freeList)*n_thread*2);
    annAlloc = malloc(sizeof(*annAlloc)*n_thread);
    annIndex = malloc(sizeof(*annIndex)*n_thread);
    if (freeList == NULL ||
        annAlloc == NULL ||
        annIndex == NULL) {
        alder_memory_sundell_finalize(n_thread, n_index);
        return 1;
    }
    memset(freeList, 0, sizeof(*freeList)*n_thread*2);
    // Allocate the real memory here.
    for (int i = 0; i < n_thread*2; i++) {
        headList_alloc(i, n_index);
    }
    memset(annAlloc, 0, sizeof(*annAlloc)*n_thread);
    memset(annIndex, 0, sizeof(*annIndex)*n_thread);
    
    annReadAddr = malloc(sizeof(*annReadAddr)*n_thread);
    if (annReadAddr == NULL) {
        alder_memory_sundell_finalize(n_thread, n_index);
        return 1;
    }
    memset(annReadAddr, 0, sizeof(*annReadAddr)*n_thread);
    for (int i = 0; i < n_thread; i++) {
        annReadAddr[i] = malloc(sizeof(*annReadAddr[i])*n_index);
        if (annReadAddr[i] == NULL) {
            alder_memory_sundell_finalize(n_thread, n_index);
            return 1;
        }
    }
    
    annBusy = malloc(sizeof(*annBusy)*n_thread);
    if (annBusy == NULL) {
        alder_memory_sundell_finalize(n_thread, n_index);
        return 1;
    }
    memset(annBusy, 0, sizeof(*annBusy)*n_thread);
    for (int i = 0; i < n_thread; i++) {
        annBusy[i] = malloc(sizeof(*annBusy[i])*n_index);
        if (annBusy[i] == NULL) {
            alder_memory_sundell_finalize(n_thread, n_index);
            return 1;
        }
    }
    
    return 0;
}

int alder_memory_sundell_finalize(int n_thread, int n_index)
{
    headList_free();
    XFREE(freeList);
    XFREE(annAlloc);
    XFREE(annIndex);
    if (annReadAddr != NULL) {
        for (int i = 0; i < n_thread; i++) {
            XFREE(annReadAddr[i]);
        }
        XFREE(annReadAddr);
    }
    if (annBusy != NULL) {
        for (int i = 0; i < n_thread; i++) {
            XFREE(annBusy[i]);
        }
        XFREE(annBusy);
    }
    return 0;
}

#pragma mark mcas

/* This function returns the content at address. 
 * 
 * The content at the read memory word can be either a value or a reference.
 * If it is a content, it returns the value of it.  If it was a reference,
 * it returns the reference with a pointer to the corresponding descriptor. 
 *
 * CHANGE: I made changes to the function. First, the line 14 is moved to the
 * inside of the if-statement just above. Second, FAA is replaced by DeRefLink
 * when dereferencing happens, and replaced by ReleaseRef when releasing the
 * memory. 
 *
 * 2: Choose first index >= 0 such that nnBusy[hreadID][index] == 0.
 * 3: Announce that I will use access the index.
 * 4: ...
 * 5: Read the content of the memory.
 * 6: Test if the value is a pointer to a descriptor.
 * 7: Fetch the descriptor of the reference.
 * 8: Increase the reference count by dereferencing the ref.
 * 9: Check whether one of the other threads has changed the announed value.
 * 10: Oops! One of the other threads has changed the announced address.
 *     I cannot use the address, and have to use the new announed address.
 * 11: Put the ref back by releasing its reference.
 * 12: Use the replaced reference.
 * 13: Increase the reference count by dereferencing the ref.
 */
word_t alder_memory_sundell_WFRead(word_t *address,
                                   alder_type_sundell_descriptor_t **desc,
                                   int threadID)
{
    word_t ref;
    int idx = -1;
    /* 2 */
    for (int i = 0; i < sn_index; i++) {
        assert(annBusy[threadID][i] == 0 || annBusy[threadID][i] == 1);
        if (annBusy[threadID][i] == 0) {
            idx = i;
            break;
        }
    }
    assert(idx >= 0);
    /* 3 */
    annIndex[threadID] = idx;
    /* 4 */
    annReadAddr[threadID][idx] = (word_t)address;
    
    /* 5 */
    ref = *address;
    /* 6 */
    if (IS_REFERENCE(ref)) {
        /* 7 */
        *desc = GET_DESCRIPTOR(ref);
        /* 8 */
//        *desc = DeRefLink(desc, threadID);
        FAA(&(*desc)->mm_ref,2);
        /* 9 */
        word_t refRep = SWAPword(&annReadAddr[threadID][idx], 0);
        /* 10 */
        if (refRep != (word_t)address) {
            /* 11 */
            if (IS_REFERENCE(ref)) {
//                ReleaseRef(*desc, threadID);
                FAA(&(*desc)->mm_ref,-2);
            }
            /* 12 */
            ref = refRep;
            /* NEW14 */
//            if (IS_REFERENCE(ref)) {
//                *desc = GET_DESCRIPTOR(ref);
//                *desc = DeRefLink(desc, threadID);
////                FAA(&(*desc)->mm_ref,1);
//            }
        }
        /* 14 */
        if (IS_REFERENCE(ref)) *desc = GET_DESCRIPTOR(ref);
    }
    return ref;
}

#pragma mark fuctions

/* This function dereferences a given link (or &node or pointer to pointer).
 * It returns the corresponding address of a node with reference count
 * incremented.
 *
 * QUESTION: I do not know what annBusy, annIndex, and annReadAddr do for me.
 *
 * D4 and D5 are the parts where I dereference and increase the count.
 * D1~D3: Announce something.
 * D6~D9: Make sure that nothing happened during D4 and D5. If something
 *        happened, I use one in the announcement (or annReadAddr).
 *
 * D1~D3: Announce the link in a free shared variable.
 *        Choose index such that annBusy[threadId][index]=0.
 *        So, I would choose an index that is not busy in the current thread.
 *        Set the annIndex to the index.
 *        Set the announce address to the link.
 *        I still do not know what I am doing???
 *
 * D4: Dereference the link.
 * D5: Increase the reference count (+2) of the corresponding node.
 *     Dereferencing NULL is okay because I check (node != NULL).
 *     A NULL pointer dereferencing does not increase the reference count.
 * D6: Remove the announcement, and "simultaneously detect a possible answer
 *     from helping."
 * D7: An anser was received.
 * D8: Decrement the reference count of the possibly reclaimed node by calling
 *     "ReleaseRef."
 * D9: Use the answer (n1) for returning.
 * D10:
 */
descriptor_t* alder_memory_sundell_DeRefLink(descriptor_t **link, int threadId)
{
    /* Local variables */
    assert(!(threadId < 0));
    word_t n1; /* an answer */
    descriptor_t *node; /* return */
    int idx;
    
    /* D1 */
    for (idx = 0; idx < sn_index; idx++) {
        if (annBusy[threadId][idx] == 0) {
            break;
        }
    }
    assert(idx != sn_index);
    /* D2 */
    annIndex[threadId] = idx;
    /* D3 */
    annReadAddr[threadId][idx] = (word_t)link;
    /* D4 */
    node = *link;
    /* D5 */
    if (node != NULL) FAA(&node->mm_ref, 2);
    /* D6 */
    n1 = SWAPword(&annReadAddr[threadId][idx], 0);
    /* D7 */
    if (n1 != (word_t)link) {
        /* D8 */
        if (node != NULL) ReleaseRef(node, threadId);
        /* D9 */
        node = (descriptor_t*)n1;
    }
    /* D10 */
    return node;
}

/* This function decrements the reference count of a given node.
 * If all references to the node has been removed, the node will eventually be
 * reclaimed (by either this operation or by pending concurrent ReleaseRef
 * operations).
 *
 * R1: Decrement the reference count (-2).
 * R2: Detect whether the reference count is 0, and try to agree on whether
 *     this function should relaim the node.
 *     node->mm_ref == 0 -> This seems redundant because the following CAS
 *     checks whether the value is 0 or not. However, it could be more 
 *     efficient because CAS tends to be costly. Checking if the ref count
 *     is zero would be cheaper than calling CAS because the failure in the
 *     first statement (or node->mm_ref == 0) would not allow the evaluation
 *     of the next CAS statement.
 * R3: Release all references from links that is contained in the structure of
 *     this node.
 * R4: Reclaim the node.
 */
void alder_memory_sundell_ReleaseRef(descriptor_t *node, int threadId)
{
    /* R1 */
    FAA(&node->mm_ref, -2);
    /* R2 */
    if (node->mm_ref == 0 && CASint(&node->mm_ref, 0, 1)) {
        /* R3 */
        /* R3. Recursively call ReleaseRef for all the references held by the node. */
        /* I may not need this line because the node might not have any reference. */
        /* R4 */
        FreeNode(node, threadId);
    }
}

/* This function helps concurrent DeRefLink by giving them recent results of
 * dereferencing the corresponding given link.
 *
 * H1~H3: Check the announcement variables to detect if the given link matches
 *        the announcement for each thread.
 * H4: Increase the busy count to make sure that this announcement variable
 *     cannot be reused for subsequent announcements.
 * H5: Get the recent dereference result of the link.
 * H6: Check if it fails to answer the announcement usin CAS.
 * H7: Decrement the reference count of the node.
 * H8: Release the claim on the announcing variable by decrementing the busy
 *     count.
 */
void alder_memory_sundell_HelpDeRef(descriptor_t **link, int threadId)
{
    /* Local variables */
    descriptor_t *node;
    int idx;
    
    /* H1 */
    for (int id = 0; id < sn_thread; id++) {
        /* H2 */
        idx = annIndex[id];
        /* H3 */
        if (annReadAddr[id][idx] == (word_t)link) {
            /* H4 */
            FAA(&annBusy[id][idx], 1);
            /* H5 */
            node = DeRefLink(link, threadId);
            /* H6 */
            if (!CASword(&annReadAddr[id][idx], (word_t)link, (word_t)node)) {
                /* H7 */
                if (node != NULL) ReleaseRef(node, threadId);
            }
            /* H8 */
            FAA(&annBusy[id][idx], -1);
        }
    }
}

static descriptor_t*
alder_memory_sundell_fixref(descriptor_t* node,
                            int fix)
{
    FAA(&node->mm_ref,fix);
    return node;
}

/* This function returns a newly allocated node with a reference count
 * indicating one active reference.
 *
 * NOTE: "A solution that has been used by Valois and others, is to keep the
 * free nodes in a linked list structure (i.e., free list) and update the head
 * pointer using CAS. One problem with this approach is that during an attempt
 * to remove the first item in the list, the CAS operation cannot verify if the
 * mm_next field of the node has been updated (as could happen if the node was
 * removed and then later re-inserted) while updating the head pointer. As we
 * are using reference counting for garbage collection, we can increase the
 * reference count directly before reading the mm_next field and thus make sure
 * that the node cannot be reclaimed." (FROM SUNDELL05).
 *
 * NOTE: "The key ideas of our solution are to force the operations to work on
 * different parts of the global free list and use helping.  All threads share
 * a variable to announce the need of a free node. Before a successful CAS
 * attempt to retrieve a node from the free-list, The alloc and free operation
 * has to help another thread.  The specific thread to help is incremented in
 * a round-robin fashion, so that eventually every thread will have some help.
 * In order to avoid conflicts with concurrent alloc operations, all alloc
 * operations operate on the same free-list, thus always leaving one free-list
 * free of conflicts for the corresponding free operation." (FROM SUNDELL05).
 *
 * WARN: I need to make sure that the memory allocated is enough.
 *
 * A1~A2: Choose a specific thread to help.
 * A3: Repeat checking whether the current `thread was already helped.
 * A4: Check whether the current thread was already helped.
 *     What I mean by "helped" : annAlloc[threadId] != NULL.
 *     SWAP(&annAlloc[threadId], NULL)
 *     -> A = SWAP(&B,C) means A = B, B = C.
 *     -> Reset the announcement variable.
 *     return alder_memory_sundell_fixref(SWAP(&annAlloc[threadId], NULL), -1)
 *     -> Return the node with adjustment of the reference count.
 * A5: The current thread is not yet helped.
 * A6: Try to retrieve the first node in the active free-list.
 * A7: Check if the current free-list is empty, and, if so, change the active
 *     free-list to the next free-list in order.
 * A8: The active free-list is not empty.
 * A9: Increase the reference count of the first node in the free-list.
 * A10: Try to remove the node from the free-list.
 * A11: Now, I retrieved a node from the free list.
 *      Decide whether to use the removed node, or to give it to the thread
 *      targeted for helping.
 * A12: ...
 * A13~A15: The targeted thread has got the removed node, and the algorithm
 *          makdes another try to allocate a new node.
 * A16 (and A14): We know that the targeted thread has been helped in some way.
 *                So, increase the helpCurrent variable so that other threads
 *                will eventually get helped by subsequent AllocNode or
 *                FreeNode operation invocations.
 * A16: The targeted thread has been helped by some other thread.
 * A17: Return the node with adjusted reference count if the retrieved node
 *      was not given to the targed thread. So, we take this node.
 * A18: Call ReleaseRef on the node that failed to be retrieved.
 */
descriptor_t* alder_memory_sundell_AllocNode(int threadId)
{
    assert(!(threadId < 0));
    descriptor_t *node = NULL;
    int tries = 0;
    
    /* A1 */
    bool helped = false;
    /* A2 */
    int helpId = helpCurrent;
    /* A3 */
    while (true) {
        tries++;
        bool is_empty_freeList = true;
        for (int i = 0; i < sn_thread*2; i++) {
            if (freeList[i] != NULL) {
                is_empty_freeList = false;
                break;
            }
        }
        assert(!is_empty_freeList);
        /* A4 */
        if (annAlloc[threadId] != NULL) {
            return alder_memory_sundell_fixref(SWAPdesc(&annAlloc[threadId], NULL), -1);
        }
        /* A5 */
        int current = currentFreeList;
        /* A6 */
        node = freeList[current];
        /* A7 */
        if (node == NULL) {
            CASint(&currentFreeList, current, (current+1)%(sn_thread*2));
        }
        /* A8 */
        else {
            /* A9 */
            FAA(&node->mm_ref, 2);
            /* A10 */
            if (CASdesc(&freeList[current], node, node->mm_next)) {
                /* A11 */
                /* If the thread to be helped has not been helped, */
                if (!helped && annAlloc[helpId] == NULL) {
                    /* A12 */
                    if (CASdesc(&annAlloc[helpId], NULL, node)) {
                        /* A13 */
                        helped = true;
                        /* A14 */
                        CASint(&helpCurrent, helpId, (helpId+1)%sn_thread);
                        /* A15 */
                        continue;
                    }
                }
                /* A16 */
                CASint(&helpCurrent, helpId, (helpId+1)%sn_thread);
                /* A17 */
                return alder_memory_sundell_fixref(node, -1);
            }
            /* A18 */
            else {
                ReleaseRef(node, threadId);
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
 * F3: The reference count that is about to be deallocated
 *     must be 1. The reference count must be incremented by 2
 *     before put it to announcement variable, annAlloc.
 *     It is possible just after annAlloc[helpId] is set to node,
 *     other threads could pick it up to do something.
 *     In AllocNode, the node from annAlloc would have
 *     the reference count decremented by 1. So, the free node
 *     in annAlloc must have the reference count of 3.
 *     In the ELSE statement, the reference count must
 *     be back to 1. It is because this node will be in 
 *     the free list not the announcement variable, annAlloc.
 * F4: Decide which of the two free-lists is likely not to conflict with
 *     concurrent AllocNode invocation.
 * F5: ...
 * F6: ...
 * F7: Repeat inserting the node to the free-list.
 * F8:
 * F9: Break (finish the function) if the insertion succeeds.
 * F10: Choose another index if the insertion fails.
 */
static void alder_memory_sundell_FreeNode(descriptor_t* node, int threadId)
{
    /* Local variables */
    assert(!(threadId < 0));
    int idx;
    
    /* F1 */
    int helpId = helpCurrent;
    /* F2 */
    CASint(&helpCurrent, helpId, (helpId+1)%sn_thread);
    /* F3 */
    assert(node->mm_ref == 1);
    FAA(&node->mm_ref, 2);
    if (CASdesc(&annAlloc[helpId], NULL, node)) {
        /* The targeted thread has been helped. */
        /* This function finishes. */
        /* No code */
    }
    /* F3 */
    else
    {
        assert(sn_thread > 1);
        FAA(&node->mm_ref, -2);
        /* F4 */
        int current = currentFreeList;
        /* F5 */
        if (current <= threadId || current > (sn_thread+threadId)) {
            idx = sn_thread + threadId;
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
            if (CASdesc(&freeList[idx], node->mm_next, node)) {
                break;
            }
            /* F10 */
            idx = (idx+sn_thread)%(sn_thread*2);
        }
    }
}

bool alder_memory_sundell_CompareAndSwapLink(descriptor_t **link,
                                             descriptor_t *old,
                                             descriptor_t *new,
                                             int threadId)
{
    if (CASdesc(link, old, new)) {
        HelpDeRef(link,threadId);
        return true;
    }
    return false;
}

int alder_memory_sundell()
{
    return 0;
}
