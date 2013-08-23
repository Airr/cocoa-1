//
//  balanced_tree.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_balanced_tree_h
#define brass_ds_balanced_tree_h

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

typedef struct tr_n_t {
    key_t         key;
    struct tr_n_t *left;
    struct tr_n_t *right;
    int           height;
    /* balancing  information */
} tree_node_t;

typedef tree_node_t object_t;

int insert(tree_node_t *tree, key_t new_key, object_t *new_object);
tree_node_t *create_tree(void);
void left_rotation(tree_node_t *n);
void right_rotation(tree_node_t *n);
object_t *find(tree_node_t *tree, key_t query_key);
object_t *findWithRecursion(tree_node_t *tree, key_t query_key);
int insert_search_trees(tree_node_t *tree, key_t new_key, object_t *new_object);
object_t *delete(tree_node_t *tree, key_t delete_key);
void remove_tree(tree_node_t *tree);
void free_tree();

__END_DECLS


#endif
