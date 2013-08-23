//
//  interval_trees.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_interval_trees_h
#define brass_ds_interval_trees_h

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

typedef struct object_struct {
    key_t b;
    key_t e;
} object_t;

typedef struct ls_n_t {
    key_t         key;
    struct ls_n_t *left;
    object_t      object;
} list_node_t;

typedef struct tr_n_t {
    key_t         key;
    struct tr_n_t *left;
    struct tr_n_t *right;
    list_node_t   *left_list;
    list_node_t   *right_list;
    /* balancing  information */
} tree_node_t;

list_node_t *blockListGetNode();
void blockListReturnNode(list_node_t *node);
void blockListFreeNode();
void remove_list(list_node_t *node);
tree_node_t *blockTreeGetNode();
void blockTreeReturnNode(tree_node_t *node);
void blockTreeFreeNode();
list_node_t *find_intervals(tree_node_t *tree, key_t query_key);
int insertInterval(tree_node_t *tree, object_t *newIntervals, size_t sizeInterval);
tree_node_t *create_tree(void);
object_t *find(tree_node_t *tree, key_t query_key);
int insert(tree_node_t *tree, key_t new_key, tree_node_t *new_object);
object_t *delete(tree_node_t *tree, key_t delete_key);
void remove_tree(tree_node_t *tree);

void checkListRC();
void checkTreeRC();

__END_DECLS

#endif
