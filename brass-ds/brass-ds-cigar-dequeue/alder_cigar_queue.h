//
//  alder_cigar_dequeue.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_alder_cigar_queue_h
#define brass_ds_alder_cigar_queue_h

typedef struct alder_cigar_item_struct {
    int n;
    int t;
    // 0: match (M)
    // 1: insertion (I)
    // 2: deletion(D)
    // 3: N
    // 4: S
    // 5: H
    // 6: P
    // 7: =
    // 8: X
} alder_cigar_item_t;

typedef struct alder_cigar_qu_t
{
    alder_cigar_item_t item;
    struct alder_cigar_qu_t *next;
} alder_cigar_queue_t;

void free_node();

alder_cigar_queue_t * alder_cigar_queue_create ();
int alder_cigar_queue_empty (alder_cigar_queue_t * qu);
void alder_cigar_enqueue (alder_cigar_item_t x, alder_cigar_queue_t * qu);
alder_cigar_item_t alder_cigar_dequeue (alder_cigar_queue_t * qu);
alder_cigar_item_t alder_cigar_front_element (alder_cigar_queue_t * qu);
void alder_cigar_remove_queue (alder_cigar_queue_t * qu);
size_t alder_cigar_size (alder_cigar_queue_t * qu);
size_t alder_cigar_strsize (alder_cigar_queue_t * qu);



#endif
