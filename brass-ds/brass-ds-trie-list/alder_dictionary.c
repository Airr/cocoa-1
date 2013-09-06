/**
 * This file, alder_dictionary.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

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
