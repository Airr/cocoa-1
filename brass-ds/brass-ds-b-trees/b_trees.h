//
//  b_trees.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_b_trees_h
#define brass_ds_b_trees_h

#include "types.h"

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

#define B 3

typedef struct tr_n_t {
    int degree;
    int height;
    key_t key[B];
    struct tr_n_t * next[B];
    /* possibly other information */
} tree_node_t;

typedef tree_node_t object_t;

__END_DECLS

#endif
