//
//  union_array.c
//  brass-ds
//
//  Created by Sang Chul Choi on 8/21/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "union_array.h"

static const endpoint_t *lastMemory1;
static const endpoint_t *lastMemory2;

int compareEndpoint(const void *a, const void *b)
{
    lastMemory1 = (endpoint_t *)a;
    lastMemory2 = (endpoint_t *)b;
    int64_t x = lastMemory1->key;
    int64_t y = lastMemory2->key;
    if (x < y)
        return -1;
    else if (x > y)
        return 1;
    return 0;
}

endpoint_t *create_union_array(interval_t *I, size_t sizeI)
{
    size_t capacityI = CAPACITYI;
    size_t sizeEndpoint = 0;
    if (sizeI == 0) {
        return NULL;
    }
    endpoint_t *v = malloc(capacityI*sizeof(endpoint_t));
    v[0].key = I[0].b;
    v[0].dir = BEGIN;
    v[1].key = I[0].e;
    v[1].dir = END;
    sizeEndpoint = 2;
    
    for (size_t i = 1; i < sizeI; i++) {
        endpoint_t qb = { .key = I[i].b, .dir = BEGIN };
        endpoint_t qe = { .key = I[i].e, .dir = END };
        long db, de;
        endpoint_t *found = bsearch(&qb, v, sizeEndpoint, sizeof(endpoint_t), compareEndpoint);
        if (found == NULL) {
            db = (lastMemory2 - v);
            if (db == 0) {
                if (qb.key < v[0].key) {
                    db = -1;
                }
            }
        } else {
            //
        }
        found = bsearch(&qe, v, sizeEndpoint, sizeof(endpoint_t), compareEndpoint);
        if (found == NULL) {
            de = (lastMemory2 - v);
            if (de == 0) {
                if (qe.key < v[0].key) {
                    de = -1;
                }
            }
        } else {
            //
        }
        
        
        
//        v[i*2].key = I[i].b;
//        v[i*2].dir = BEGIN;
//        v[i*2+1].key = I[i].e;
//        v[i*2+1].dir = END;
    }
    return v;
}