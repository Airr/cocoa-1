//
//  stack.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_stack_h
#define brass_ds_stack_h

//#include "../brass-ds/common.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

#define TREE

#ifdef TREE
struct tr_n_t;
typedef struct tr_n_t* item_t;
#else
typedef int item_t;
#endif /* TREE */

typedef struct st_t {
    item_t       item;
    struct st_t *next;
} node_t;

typedef node_t stack;

node_t *create_stack(void);
int stack_empty(node_t *st);
void push( item_t x, node_t *st);
item_t pop(node_t *st);
item_t top_element(node_t *st);
void remove_stack(node_t *st);
void free_stack();


__END_DECLS

#endif
