/**
 * This file, alder_uf_void.h, is part of alder-uf.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uf.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_uf_alder_uf_void_h
#define alder_uf_alder_uf_void_h

#include <stdio.h>
#include <stdint.h>
#include "alder_cmacro.h"
#include "alder_node_item_tree_source.h"
#include "alder_node_item_uf_source.h"

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

/* ALDER_ERROR: unrecoverable fatal error.
 */
enum {
    ALDER_SUCCESS = 0,
    ALDER_ERROR = 1,
    ALDER_ERROR_MEMORY = 2,
    ALDER_ERROR_DUPLICATE = 3,
    ALDER_ERROR_NOTEXIST = 4
};

enum {
    ALDER_UF_SUCCESS = 0,
    ALDER_UF_ERROR = 1,
    ALDER_UF_ERROR_MEMORY = 2,
    ALDER_UF_ERROR_DUPLICATE = 3,
    ALDER_UF_ERROR_NOTEXIST = 4
};

#if 0
typedef struct alder_uf_n_t
{
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
    int rank;
    int degree;
    void *item;
    union {
        struct alder_uf_n_t *up;
        struct alder_uf_n_t *next;
    };
    struct alder_uf_n_t *down;
    struct alder_uf_n_t *left;
    struct alder_uf_n_t *right;
    struct alder_uf_n_t *list;
} alder_uf_node_t;

//typedef alder_uf_node_t object_t;

typedef struct alder_tr_n_t
{
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
//    key_t key;
    void* key;
    union {
        struct alder_tr_n_t *left;
        struct alder_tr_n_t *next;
    };
    struct alder_tr_n_t *right;
    int height;
} alder_tree_node_t;

/* Memory-management for alder_uf_node_t
 */
struct alder_uf_memory {
    alder_uf_node_t **head;
    alder_uf_node_t *current;
    alder_uf_node_t *free;
    int size;
    size_t nhead;
};

/* Memory-management for alder_tree_node_t
 */
struct alder_tree_memory {
    alder_tree_node_t **head;
    alder_tree_node_t *current;
    alder_tree_node_t *free;
    int size;
    size_t nhead;
};
#endif





typedef struct alder_uf {
    alder_tree_node_t *tree;
    alder_uf_node_t *uf;
    alder_comparison_func *alder_compare;
    alder_print_func *alder_print;
    
    size_t block_size;
    struct alder_uf_memory_struct block_uf;
    struct alder_tree_memory_struct block_tree;
} alder_uf_t;

alder_uf_t * alder_uf_create(alder_comparison_func *,
                             alder_print_func *);
void alder_uf_destroy(alder_uf_t *uf);

int alder_uf_makeset(alder_uf_t *uf, void *e);
int alder_uf_union(alder_uf_t *uf, void *e1, void *e2);
int alder_uf_find(alder_uf_t *uf, void *e, void **r);
int alder_uf_item(alder_uf_t *uf, void *e, void **r);
int alder_uf_same(alder_uf_t *uf, void *e1, void *e2, int *s);
int alder_uf_count(alder_uf_t *uf);
void *alder_uf_set_k(alder_uf_t *uf, int k);
int alder_uf_delete(alder_uf_t *uf, void *e);
int alder_uf_enumerate(alder_uf_t *uf, void *e);
int alder_uf_set_count(alder_uf_t *uf, void *e);
void *alder_uf_another(alder_uf_t *uf, void *e);
void alder_uf_printSet(alder_uf_t *uf, FILE *fp);
void alder_uf_print(alder_uf_t *uf, FILE *fp, int i);
void *alder_uf_root(alder_uf_t *uf, void *e);


__END_DECLS


#endif /* alder_uf_alder_uf_void_h */
