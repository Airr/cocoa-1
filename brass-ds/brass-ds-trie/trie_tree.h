//
//  trie_tree.h
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_trie_tree_h
#define brass_ds_trie_tree_h

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

typedef struct trie_n_t {
    struct trie_n_t   *next[256];
    /* possibly additional information*/
} trie_node_t;

__END_DECLS

#endif
