//
//  stack.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_stack_h
#define brass_ds_stack_h

#include "../brass-ds/common.h"

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

typedef struct st_t {
    item_t *base;
    item_t  *top;
    int size;
    struct st_t *previous;
} stack_t;

__END_DECLS


#endif
