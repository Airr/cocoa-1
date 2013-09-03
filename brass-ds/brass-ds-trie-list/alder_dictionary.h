//
//  alder_dictionary.h
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_alder_dictionary_h
#define brass_ds_alder_dictionary_h

#include "trie_tree_list.h"

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

typedef trie_node_t alder_dictionary_t;

alder_dictionary_t * alder_dictionary_initialize();
int64_t alder_dictionary_add(trie_node_t *t, char *s);
void alder_dictionary_finalize(trie_node_t *t);

__END_DECLS


#endif
