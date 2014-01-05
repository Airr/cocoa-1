/**
 * This file, alder_dictionary.h, is part of alder-dictionary.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-dictionary is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-dictionary is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-dictionary.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_dictionary_alder_dictionary_h
#define alder_dictionary_alder_dictionary_h

#include <stddef.h>
#include "alder_cmacro.h"
#include "alder_node_item_tree_source.h"

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

enum {
    ALDER_DICTIONARY_SUCCESS = 0,
    ALDER_DICTIONARY_ERROR = 1,
    ALDER_DICTIONARY_ERROR_MEMORY = 2,
    ALDER_DICTIONARY_ERROR_DUPLICATE = 3,
    ALDER_DICTIONARY_ERROR_NOTEXIST = 4
};

typedef struct alder_dictionary_struct {
    alder_tree_node_t *tree;
    alder_comparison_func *compare;
    alder_print_func *print;
    struct alder_tree_memory_struct *block;
} alder_dictionary_t;

alder_dictionary_t *
alder_dictionary_create (alder_comparison_func *compare, alder_print_func *print);
void alder_dictionary_destroy (alder_dictionary_t *o);
void * alder_dictionary_find (alder_dictionary_t *o, void *query_key);
int alder_dictionary_insert (alder_dictionary_t *o, void *new_object);
int alder_dictionary_insert2 (alder_dictionary_t *o, void *new_key, void *new_object);
void * alder_dictionary_delete (alder_dictionary_t *o, void *delete_key);
void alder_dictionary_print (alder_dictionary_t *o, FILE *fp);

__END_DECLS

#endif /* alder_dictionary_alder_dictionary_h */
