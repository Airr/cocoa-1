//
//  alder_dictionary.c
//  brass-ds
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include "trie_tree_list.h"
#include "alder_dictionary.h"



alder_dictionary_t * alder_dictionary_initialize()
{
    trie_node_t *t = create_trie();
    return t;
}

int64_t alder_dictionary_add(trie_node_t *t, char *s)
{
    trie_node_t *m = find(t, s);
    if (m == NULL) {
        m = get_node();
        m->n = 1;
        insert(t, s, m);
    } else {
        m->n++;
    }
    
    return m->n;
}

void alder_dictionary_finalize(trie_node_t *t)
{
    remove_trie(t);
}
