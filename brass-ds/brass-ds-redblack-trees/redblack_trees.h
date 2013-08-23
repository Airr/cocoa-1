//
//  redblack_trees.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_redblack_trees_h
#define brass_ds_redblack_trees_h

#include <stdint.h>

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

typedef int64_t key_t;

typedef struct tr_n_t {
    key_t             key;
    struct tr_n_t     *left;
    struct tr_n_t     *right;
    enum {red, black} color;
    /* balancing  information */
} tree_node_t;

typedef tree_node_t object_t;

int insert(tree_node_t *tree, key_t new_key, object_t *new_object);
object_t *delete(tree_node_t *tree, key_t delete_key);
tree_node_t *create_tree(void);
void left_rotation(tree_node_t *n);
void right_rotation(tree_node_t *n);
void remove_tree(tree_node_t *tree);
void free_tree();
tree_node_t *make_list(tree_node_t *tree);
void print_tree(tree_node_t *tree);
// create_tree - remove_tree ... free_tree
// We might call a init_tree - free_tree pair as we have create_tree - remove_tree.
// create_tree - remove_tree can be replaced by new_tree - delete_tree.
// init_tree - free_tree can be replaced by initialize_tree - finalize_tree.

__END_DECLS

#endif
