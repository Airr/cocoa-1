//
//  main.c
//  brass-ds-cigar-dequeue
//
//  Created by Sang Chul Choi on 8/29/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "alder_cigar_queue.h"

int main(int argc, const char * argv[])
{
    alder_cigar_queue_t *q = alder_cigar_queue_create();
    
    alder_cigar_item_t m = { .n = 100, .t = 1 };
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    m.t = 2;
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    m.t = 1;
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    alder_cigar_enqueue(m, q);
    m.t = 3;
    alder_cigar_enqueue(m, q);
    
    size_t n = alder_cigar_strsize(q);
    printf("queue size: %zd\n", n);
    while (!alder_cigar_queue_empty(q)) {
        alder_cigar_item_t x = alder_cigar_dequeue(q);
        printf("t: %d (%d)\n", x.t, x.n);
    }
    alder_cigar_remove_queue(q);
    
    free_node();
    return 0;
}

