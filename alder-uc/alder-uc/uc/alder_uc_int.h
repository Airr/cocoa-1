/**
 * This file, alder_uc_int.h, is part of alder-uc.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uc.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_uc_alder_uc_int_h
#define alder_uc_alder_uc_int_h

#include <stdint.h>
#include "alder_uc.h"
#include "alder_cmacro.h"

#include "alder_node_item_int_source.h"

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

typedef struct alder_uc_int_struct {
    alder_uc_t *uc;
    struct alder_item_int_memory_struct *bint;
} alder_uc_int_t;

alder_uc_int_t * alder_uc_int_create();
void alder_uc_int_destroy(alder_uc_int_t *uc);
int alder_uc_int_set_create(alder_uc_int_t *uc, int setid);
int alder_uc_int_set_insert(alder_uc_int_t *uc, int setid, int elementid);
int alder_uc_int_set_find(alder_uc_int_t *uc, int setid);
int alder_uc_int_set_union(alder_uc_int_t *uc, int seti, int setj);
int alder_uc_int_set_copy(alder_uc_int_t *uc, int seti, int setj);
int alder_uc_int_element_create(alder_uc_int_t *uc, int elementid);
int alder_uc_int_element_insert(alder_uc_int_t *uc);
int alder_uc_int_element_find(alder_uc_int_t *uc);
int alder_uc_int_element_union(alder_uc_int_t *uc);
int alder_uc_int_element_copy(alder_uc_int_t *uc);
void alder_uc_int_test();

__END_DECLS


#endif /* alder_uc_alder_uc_int_h */
