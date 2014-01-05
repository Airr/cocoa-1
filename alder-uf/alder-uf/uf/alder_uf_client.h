/**
 * This file, alder_uf_client.h, is part of alder-uf.
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

#ifndef alder_uf_alder_uf_client_h
#define alder_uf_alder_uf_client_h

#include <stdint.h>
#include "alder_uf_void.h"

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


typedef struct alder_item {
    int64_t a;
    int64_t b;
} item_t;

typedef struct uf_i_t {
    int use; /* use: 0 if the memory is not in use, 1 otherwise. */
    item_t item;
    struct uf_i_t *next;
} alder_item_node_t;

struct alder_item_memory {
    alder_item_node_t **head;
    alder_item_node_t *current;
    alder_item_node_t *free;
    int size;
    size_t nhead;
};

typedef struct alder_uf_tuple {
    alder_uf_t *uf;
    size_t block_size;
    struct alder_item_memory block_item;
} alder_uf_tuple_t;

alder_uf_tuple_t * alder_uf_tuple_create();
void alder_uf_tuple_destroy(alder_uf_tuple_t *ut);
int alder_uf_tuple_makeset(alder_uf_tuple_t *ut, item_t e);
int alder_uf_tuple_union(alder_uf_tuple_t *ut, item_t e1, item_t e2);
int alder_uf_tuple_find(alder_uf_tuple_t *ut, item_t *e);
int alder_uf_tuple_same(alder_uf_tuple_t *ut, item_t e1, item_t e2, int *s);
int alder_uf_tuple_count(alder_uf_tuple_t *ut);
item_t alder_uf_tuple_set_k(alder_uf_tuple_t *ut, int k);
int alder_uf_tuple_delete(alder_uf_tuple_t *ut, item_t e);
int alder_uf_tuple_enumerate(alder_uf_tuple_t *ut, item_t e);
void alder_uf_tuple_printSet(alder_uf_tuple_t *ut, FILE *fp);
void alder_uf_tuple_print(alder_uf_tuple_t *ut, FILE *fp, int i);


int alder_uf_client_test1();

__END_DECLS


#endif /* alder_uf_alder_uf_client_h */
