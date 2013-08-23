//
//  stack.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_stack_h
#define brass_ds_stack_h

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

typedef void* item_t;

typedef struct st_t {
    item_t       item;
    struct st_t *next;
} stack_node_t;

stack_node_t *create_stack(void);
int stack_empty(stack_node_t *st);
void push( item_t x, stack_node_t *st);
item_t pop(stack_node_t *st);
item_t top_element(stack_node_t *st);
void remove_stack(stack_node_t *st);
void free_stack();
item_t next_element(stack_node_t **pst);
int last_element(stack_node_t *st);

__END_DECLS

#endif
