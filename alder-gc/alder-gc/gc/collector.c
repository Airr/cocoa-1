/* Collector.c - A simple garbage collector.
 * Copyright (C) 2011 Rob King <jking@deadpixi.com>
 *
 * This file is part of DPGC.
 *
 * DPGC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * DPGC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DPGC.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "collector.h"

#ifndef STEP_FACTOR
#define STEP_FACTOR 3
#endif 

enum{
    WHITE,
    GRAY,
    BLACK
};

#define validcollector(obj)                                                 \
    do{                                                                     \
        assert(obj         != NULL);                                        \
        assert(obj->pauses == 0 ? obj->roots   != NULL : 1);                \
        assert(obj->pauses == 0 ? obj->touch   != NULL : 1);                \
        assert(obj->pauses == 0 ? obj->dispose != NULL : 1);                \
    } while(0)

#define validlist(obj)                                                      \
    do{                                                                     \
        assert(obj       != NULL);                                          \
        assert(obj->prev != NULL ? obj->prev->next == obj : 1);             \
        assert(obj->next != NULL ? obj->next->prev == obj : 1);             \
    } while(0)

#define validobj(obj)                                                       \
    do{                                                                     \
        validlist(obj);                                                     \
        assert(obj->color == WHITE || obj->color == GRAY                    \
                                   || obj->color == BLACK);                 \
    } while(0)

#define remove(head, object)                                                \
    do{                                                                     \
        assert(head != NULL);                                               \
        validlist(object);                                                  \
        if(head == object){                                                 \
            head = object->next;                                            \
            if(object->next != NULL)                                        \
                object->next->prev = NULL;                                  \
        } else{                                                             \
            if(object->prev != NULL)                                        \
                object->prev->next = object->next;                          \
                                                                            \
            if(object->next != NULL)                                        \
                object->next->prev = object->prev;                          \
            validlist(head);                                                \
        }                                                                   \
        object->next = NULL;                                                \
        object->prev = NULL;                                                \
    } while(0)

#define insert(head, object)                                                \
    do{                                                                     \
        assert(head != object);                                             \
        validlist(object);                                                  \
        if(head == NULL){                                                   \
            head         = NULL;                                            \
            object->prev = NULL;                                            \
            object->next = NULL;                                            \
            head = object;                                                  \
        } else{                                                             \
            assert(head->prev == NULL);                                     \
            head->prev   = object;                                          \
            object->next = head;                                            \
            object->prev = NULL;                                            \
            head = object;                                                  \
        }                                                                   \
        validlist(head);                                                    \
    } while(0)

Collector *
makecollector(Marker touch, Disposer dispose)
{
    Collector *collector;

    collector = calloc(1, sizeof(Collector));
    if(collector != NULL){
        collector->deferrals = 0;
        collector->pauses    = 1;
    
        collector->touch     = touch;
        collector->dispose   = dispose;
    }

    return collector;
}

int
isroot(Collector *collector, Collectee *obj)
{
    Root *root;

    validcollector(collector);
    validobj(obj);

    root = collector->roots;
    while(root != NULL && root->root != obj)
        root = root->next;

    return root != NULL && root->root == obj;
}

int
addroot(Collector *collector, Collectee *obj)
{
    Root *root;

    validcollector(collector);
    validobj(obj);
    assert(collector->pauses >  0);
    assert(!isroot(collector, obj));

    root = calloc(1, sizeof(Root));
    if(root != NULL){
        obj->color = GRAY;
        insert(collector->grays, obj);
    
        root->root = obj;
        insert(collector->roots, root);
    }

    return root != NULL;
}

void
removeroot(Collector *collector, Collectee *obj)
{
    Root *root;

    validcollector(collector);
    validobj(obj);
    assert(collector->pauses > 0);
    assert(isroot(collector, obj));
    assert(obj->color == GRAY || obj->color == BLACK);
    assert((((char *)obj) - offsetof(struct Root, root)) > 0);

    root = (Root *)(obj - offsetof(struct Root, root));
    validobj(root->root);

    remove(collector->roots, root);

    assert(collector->roots != NULL);
}

void
addobj(Collector *collector, Collectee *obj, Collectee *ref)
{
    validcollector(collector);
    validobj(obj);
    assert(collector->pauses >  0);
    assert(STEP_FACTOR       >= 0);
    assert(obj->prev         == NULL);
    assert(obj->next         == NULL);
    assert(obj->color        == WHITE);

    insert(collector->whites, obj);
    obj->color = WHITE;

    collector->count++;

    if(ref != NULL){
        validobj(ref);
        assert(isroot(collector, ref) || ref->prev != NULL
                                      || ref->next != NULL);

        addref(collector, ref, obj);
    }
        
    stepgc(collector, STEP_FACTOR); 
}

void
recolor(Collector *collector, Collectee *obj, int color)
{
    validcollector(collector);
    validobj(obj);
    assert(color      == WHITE || color == GRAY || color == BLACK);
    assert(obj->color == BLACK ?  color == BLACK                  : 1);
    assert(obj->color == GRAY  ?  color == GRAY || color == BLACK : 1);
    assert(obj->color == WHITE ?  color == GRAY || color == BLACK : 1);

    if(obj->color != color){
        switch(obj->color){
            case WHITE:   remove(collector->whites, obj);     break;
            case GRAY:    remove(collector->grays,  obj);     break;
            case BLACK:   remove(collector->blacks, obj);     break;
        }
    
        switch(color){
            case WHITE:   insert(collector->whites, obj);     break;
            case GRAY:    insert(collector->grays,  obj);     break;
            case BLACK:   insert(collector->blacks, obj);     break;
        }
    
        obj->color = color;
    }
}

void
touchobj(Collector *collector, Collectee *obj)
{
    validcollector(collector);
    validobj(obj);

    if(obj->color == WHITE)
        recolor(collector, obj, GRAY);
}

void
addref(Collector *collector, Collectee *referer, Collectee *referee)
{
    validcollector(collector);
    validobj(referer);
    validobj(referee);
    assert(collector->pauses > 0);

    if(referer->color == GRAY || referer->color == BLACK)
        recolor(collector, referee, GRAY);
}

void
stepgc(Collector *collector, unsigned int steps)
{
    Collectee *next;
    Collectee *obj;
    Root      *root;
    Root      *nextroot;

    validcollector(collector);

    if(collector->pauses > 0)
        collector->deferrals += steps;
    else{
        while(steps > 0 && collector->grays != NULL){
            if(collector->touch(collector, collector->grays))
                recolor(collector, collector->grays, BLACK);
    
            steps--;
        }
    
        if(collector->grays == NULL){
            obj = collector->whites;
            while(obj != NULL){
                assert(!isroot(collector, obj));
                assert(obj->color == WHITE);
    
                next = obj->next;
                remove(collector->whites, obj);
                
                collector->dispose(collector, obj);
                collector->count--;
    
                obj = next;
            }
    
            root = collector->roots;
            while(root != NULL){
                assert(root->root->color == BLACK);
    
                nextroot = root->next;
                remove(collector->blacks, root->root);
                insert(collector->grays,  root->root);
    
                root->root->color = GRAY;
                root = nextroot;
            }
    
            obj = collector->blacks;
            while(obj != NULL){
                assert(obj->color == BLACK);
                
                next = obj->next;
                remove(collector->blacks, obj);
                insert(collector->whites, obj);
    
                obj->color = WHITE;
                obj = next;
            }
        }
    }
}    

void
pausegc(Collector *collector)
{
    validcollector(collector);

    collector->pauses++;
}

void
unpausegc(Collector *collector)
{
    validcollector(collector);
    assert(collector->pauses > 0);

    collector->pauses--;

    if(collector->pauses == 0){
        stepgc(collector, collector->deferrals);
        collector->deferrals = 0;
    }
}

void *
gcalloc(Collector *collector, size_t size)
{
    void *rc;

    validcollector(collector);
    assert(size        >  0);
    assert(STEP_FACTOR >= 0);

    rc = calloc(1, size);
    while(rc == NULL && collector->pauses == 0 && collector->grays != NULL){
        stepgc(collector, STEP_FACTOR);
        rc = calloc(1, size);
    }

    return rc;
}
