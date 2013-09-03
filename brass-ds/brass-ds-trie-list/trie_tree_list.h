//
//  trie_tree_list.h
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_trie_tree_list_h
#define brass_ds_trie_tree_list_h

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

typedef struct trie_n_t
{
    char this_char;
    int64_t n;
    struct trie_n_t *next;
    struct trie_n_t *list;
    /* possibly additional information */
} trie_node_t;

typedef struct trie_n_t object_t;
//typedef struct object_struct {
//    int64_t n;
//} object_t;

trie_node_t *get_node();
void return_node(trie_node_t *node);
void free_node();

trie_node_t * create_trie (void);
object_t * find (trie_node_t * trie, char *query_string);
int insert (trie_node_t * trie, char *new_string, object_t * new_object);
object_t * delete (trie_node_t * trie, char *delete_string);
void remove_trie (trie_node_t * trie);

__END_DECLS

#endif
