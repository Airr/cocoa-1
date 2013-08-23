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

typedef struct interval_struct {
    key_t b;
    key_t e;
} interval_t;

typedef struct tr_n_t {
    key_t             key;
    struct tr_n_t     *left;
    struct tr_n_t     *right;
    struct tr_n_t     *min;
    struct tr_n_t     *max;
    struct tr_n_t     *start;
    struct tr_n_t     *end;
    struct tr_n_t     *left_cross;
    struct tr_n_t     *right_cross;
    struct tr_n_t     *transfer;
    struct tr_n_t     *next_down;
    struct tr_n_t     *succ;
    struct tr_n_t     *L;
    struct tr_n_t     *R;
    enum {red, black} color;
//    enum {left, right} side;
    
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
tree_node_t *create_union_tree(interval_t *I, size_t sizeI);
tree_node_t *make_list_leaves(tree_node_t *tree);

__END_DECLS

#endif
