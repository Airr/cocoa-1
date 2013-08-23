//
//  search_trees.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_search_trees_h
#define brass_ds_search_trees_h

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

typedef struct tr_n_t {
    key_t         key;
    struct tr_n_t *left;
    struct tr_n_t *right;
    /* balancing  information */
} tree_node_t;

typedef tree_node_t object_t;

tree_node_t *get_node();
void return_node(tree_node_t *node);
void free_node();
tree_node_t *create_tree(void);
void left_rotation(tree_node_t *n);
void right_rotation(tree_node_t *n);
object_t *find(tree_node_t *tree, key_t query_key);
object_t *findWithRecursion(tree_node_t *tree, key_t query_key);
int insert(tree_node_t *tree, key_t new_key, object_t *new_object);
object_t *delete(tree_node_t *tree, key_t delete_key);
void remove_tree(tree_node_t *tree);

__END_DECLS


#endif
