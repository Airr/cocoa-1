/**
 * This file, alder_memory_gidenstam.c, is part of alder-memory.
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

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "alder_type_gidenstam.h"
#include "alder_atomic.h"
#include "alder_memory_gidenstam.h"

#pragma mark global
#define descriptor_t alder_type_gidenstam_descriptor_t
#define CASdescriptor alder_atomic_gidenstam_CASdescriptor
#define FAA alder_atomic_FAAint
#define DeRefLink alder_memory_gidenstam_DeRefLink
#define ReleaseRef alder_memory_gidenstam_ReleaseRef
#define StoreRef alder_memory_gidenstam_StoreRef
#define ReleaseRef alder_memory_gidenstam_ReleaseRef
#define CompareAndSwapRef alder_memory_gidenstam_CompareAndSwapRef
#define TerminateNode alder_memory_gidenstam_TerminateNode
#define CleanUpAll alder_memory_gidenstam_CleanUpAll
#define CleanUpLocal alder_memory_gidenstam_CleanUpLocal
#define Scan alder_memory_gidenstam_Scan

static int sn_thread = 1;
static int sn_index = 2;
static int s_THREADSHOLD_C = -1;
static int s_THREADSHOLD_S = -1;
alder_type_gidenstam_descriptor_t ***HP;
alder_type_gidenstam_descriptor_t ***DL_Nodes;
int **DL_Claims;
bool **DL_Done;
int **DL_Nexts;

int alder_memory_gidenstam()
{
    return 0;
}

int alder_memory_gidenstam_initialize(int n_thread, int n_index)
{
    return 0;
}

int alder_memory_gidenstam_finalize()
{
    return 0;
}

descriptor_t* alder_memory_gidenstam_DeRefLink(descriptor_t **link)
{
    int threadId = -1;
    assert(threadId >= 0);
    int idx = -1;
    for (int i = 0; i < sn_index; i++) {
        if (HP[threadId][i] == NULL) {
            idx = i;
            break;
        }
    }
    while (true) {
        descriptor_t *node = *link;
        HP[threadId][idx] = node;
        if (*link == node) {
            return node;
        }
    }
}

void alder_memory_gidenstam_ReleaseRef(descriptor_t *node)
{
    int threadId = -1;
    assert(threadId >= 0);
    int idx = -1;
    for (int i = 0; i < sn_index; i++) {
        if (HP[threadId][i] == node) {
            idx = i;
            break;
        }
    }
    HP[threadId][idx] = NULL;
}

bool alder_memory_gidenstam_CompareAndSwapRef(descriptor_t **link,
                                              descriptor_t *old,
                                              descriptor_t *new)
{
    if (CASdescriptor(link,old,new)) {
        if (new != NULL) {
            FAA(&new->mm_ref,1);
            new->mm_trace = false;
        }
        if (old != NULL) {
            FAA(&old->mm_ref,-1);
        }
        return true;
    }
    return false;
}

void alder_memory_gidenstam_StoreRef(descriptor_t **link,
                                     descriptor_t *node)
{
    descriptor_t *old = *link;
    *link = node;
    if (node != NULL) {
        FAA(&node->mm_ref,1);
        node->mm_trace = false;
    }
    if (old != NULL) {
        FAA(&old->mm_ref,-1);
    }
}

descriptor_t* alder_memory_gidenstam_NewNode()
{
    descriptor_t *node = NULL;
    node = malloc(sizeof(*node));
    node->mm_ref = 0; node->mm_trace = false; node->mm_del = false;
    
    int threadId = -1;
    assert(threadId >= 0);
    int idx = -1;
    for (int i = 0; i < sn_index; i++) {
        if (HP[threadId][i] == NULL) {
            idx = i;
            break;
        }
    }
    HP[threadId][idx] = node;
    return node;
}

void alder_memory_gidenstam_DeleteNode(descriptor_t *node)
{
    int dlist = -1;
    int dcount = -1;
    assert(dlist >= 0);
    assert(dcount >= 0);
    
    ReleaseRef(node);
    node->mm_del = true; node->mm_trace = false;
    
    int threadId = -1;
    assert(threadId >= 0);
    int idx = -1;
    for (int i = 0; i < sn_index; i++) {
        if (HP[threadId][i] == NULL) {
            idx = i;
            break;
        }
    }
    
    DL_Done[threadId][idx] = false;
    DL_Nodes[threadId][idx] = node;
    DL_Nexts[threadId][idx] = dlist;
    dlist = idx; dcount = dcount + 1;
    while (true) {
        if (dcount == s_THREADSHOLD_C) {
            CleanUpLocal();
        }
        if (dcount >= s_THREADSHOLD_S) {
            Scan();
        }
        if (dcount == s_THREADSHOLD_C) {
            CleanUpAll();
        } else {
            break;
        }
    }
}

void alder_memory_gidenstam_TerminateNode(descriptor_t *node,
                                          bool concurrent)
{
    if (!concurrent) {
        for (int x; /* all x where link[x] of node is reference-counted */) {
            StoreRef(node->link[x],NULL);
        }
    } else {
        for (int x; /* all x where link[x] of node is reference-counted */) {
            descriptor_t *node1 = NULL;
            do {
                node1 = node->link[x];
            } while (!CompareAndSwapRef(&node->link[x],node1,NULL));
        }
    }
}

void alder_memory_gidenstam_CleanUpNode(descriptor_t *node)
{
    descriptor_t *node1 = NULL;
    descriptor_t *node2 = NULL;
    for (int x; /* all x where link[x] of node is reference-counted */) {
    retry:
        node1 = DeRefLink(&node->link[x]);
        if (node1 != NULL && node1->mm_del) {
            node2 = DeRefLink(&node1->link[x]);
            CompareAndSwapRef(&node->link[x],node1,node2);
            ReleaseRef(node2);
            ReleaseRef(node1);
            goto retry;
        }
        ReleaseRef(node1);
    }
}


void alder_memory_gidenstam_CleanUpLocal()
{
    int threadId = -1;
    int dlist = -1;
    assert(threadId >= 0);
    assert(dlist >= 0);
    descriptor_t *node = NULL;
    int idx = dlist;
    while (idx != 0) {
        node = DL_Nodes[threadId][idx];
        CleanUpNode(node);
        idx = DL_Nexts[threadId][idx];
    }
}

void alder_memory_gidenstam_CleanUpAll()
{
    descriptor_t *node = NULL;
    for (int thread = 0; thread < sn_thread; thread++) {
        for (int idx = 0; idx < s_THREADSHOLD_C; idx++) {
            node = DL_Nodes[thread][idx];
            if (node != NULL && !DL_Done[thread][idx]) {
                FAA(&DL_Claims[thread][idx],1);
                if (node == DL_Nodes[thread][idx]) {
                    CleanUpNode(node);
                }
                FAA(&DL_Claims[thread][idx],-1);
            }
        }
    }
}

void alder_memory_gidenstam_Scan()
{
    descriptor_t **plist = NULL;
    int threadId = -1;
    int dlist = -1;
    int dcount = -1;
    int new_dlist = -1;
    int new_dcount = -1;
    assert(threadId >= 0);
    assert(dlist >= 0);
    assert(dcount >= 0);
    
    descriptor_t *node = NULL;
    int idx = dlist;
    while (idx != 0) {
        node = DL_Nodes[threadId][idx];
        if (node->mm_ref == 0) {
            node->mm_trace = true;
            if (node->mm_ref != 0) {
                node->mm_trace = false;
            }
        }
        idx = DL_Nexts[threadId][idx];
    }
    plist = NULL;
    new_dlist = -1;
    new_dcount = 0;
    for (int thread = 0; thread < sn_thread; thread++) {
        for (int idx = 0; idx < sn_index; idx++) {
            node = HP[thread][idx];
            if (node != NULL) {
                plist = plist <add> node;
            }
        }
    }
    /* Sort and remove dupliates in array list. */
    while (dlist != 0) {
        idx = dlist;
        node = DL_Nodes[threadId][idx];
        dlist = DL_Nexts[threadId][idx];
        if (node->mm_ref == 0 && node->mm_trace && node <not in> plist) {
            DL_Nodes[threadId][idx] = NULL;
            if (DL_Claims[threadId][idx] == 0) {
                TerminateNode(node,false);
                free(node); node = NULL;
                continue;
            }
            TerminateNode(node,true);
            DL_Done[threadId][idx] = true;
            DL_Nodes[threadId][idx] = node;
        }
        DL_Nexts[threadId][idx] = new_dlist;
        new_dlist = idx;
        new_dcount = new_dcount + 1;
    }
    dlist = new_dlist;
    dcount = new_dcount;
}