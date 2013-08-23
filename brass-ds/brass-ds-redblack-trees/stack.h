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

void create_stack(void);
void push( item_t x);
item_t pop(void);

void remove_stack(void);

void free_stack();

int stack_empty(void);
item_t top_element(void);

__END_DECLS

#endif
