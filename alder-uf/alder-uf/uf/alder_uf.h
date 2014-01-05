/**
 * This file, alder_uf.h, is part of alder-uf.
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
#ifndef alder_uf_alder_uf_h
#define alder_uf_alder_uf_h

#include <stdio.h>

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

typedef int item_t;
typedef int key_t;

typedef struct uf_i_t {
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
    item_t item;
    struct uf_i_t *next;
} item_node_t;

typedef struct uf_n_t
{
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
    int rank;
    int degree;
    item_node_t *item;
    union {
        struct uf_n_t *up;
        struct uf_n_t *next;
    };
    struct uf_n_t *down;
    struct uf_n_t *left;
    struct uf_n_t *right;
    struct uf_n_t *list;
} uf_node_t;

typedef uf_node_t object_t;

typedef struct tr_n_t
{
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
    key_t key;
    union {
        struct tr_n_t *left;
        struct tr_n_t *next;
    };
    struct tr_n_t *right;
    int height;
} tree_node_t;

typedef struct alder_uf {
    tree_node_t *tree;
    uf_node_t *uf;
} alder_uf_t;

alder_uf_t * alder_uf_create();
void alder_uf_destroy(alder_uf_t *uf);
int alder_uf_makeset(alder_uf_t *uf, item_t e);
int alder_uf_union(alder_uf_t *uf, item_t e1, item_t e2);
int alder_uf_find(alder_uf_t *uf, item_t *e);
int alder_uf_same(alder_uf_t *uf, item_t e1, item_t e2, int *s);
int alder_uf_count(alder_uf_t *uf);
item_t alder_uf_set_k(alder_uf_t *uf, int k);
int alder_uf_delete(alder_uf_t *uf, item_t e);
int alder_uf_enumerate(alder_uf_t *uf, item_t e);
void alder_uf_printSet(alder_uf_t *uf, FILE *fp);
void alder_uf_print(alder_uf_t *uf, FILE *fp, int i);
int alder_uf_test0();
int alder_uf_test1();

__END_DECLS

#endif /* alder_uf_alder_uf_h */
