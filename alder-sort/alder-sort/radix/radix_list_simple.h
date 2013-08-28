//
//  radix_list_simple.h
//  alder-sort
//
//  Created by Sang Chul Choi on 8/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_sort_radix_list_simple_h
#define alder_sort_radix_list_simple_h

typedef struct list {
    struct list *next;
    char *data;
} list;

int radix_list_simple_main();
    
#endif
