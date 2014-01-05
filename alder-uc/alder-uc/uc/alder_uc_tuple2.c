/**
 * This file, alder_uc_tuple2.c, is part of alder-uc.
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

#include <stdio.h>
#include "alder_uc_tuple2.h"

#define ALDER_BASE_ITEM_TUPLE2_NODE
#include "alder_node_memory_template_on.h"
#include "alder_node_memory_source.h"
#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_ITEM_TUPLE2_NODE


alder_uc_tuple2_t * alder_uc_tuple2_create()
{
    alder_uc_tuple2_t *uc = malloc(sizeof(*uc));
    if (uc == NULL) {
        return NULL;
    }
    uc->block = create_node_item_tuple2();
    if (uc->block == NULL) {
        XFREE(uc);
        return NULL;
    }
    
    return uc;
}

void alder_uc_tuple2_destroy(alder_uc_tuple2_t *uc)
{
    
}

int alder_uc_tuple2_set_create(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_set_insert(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_set_find(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_set_union(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_set_copy(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_element_create(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_element_insert(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_element_find(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_element_union(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}

int alder_uc_tuple2_element_copy(alder_uc_tuple2_t *uc)
{
    
    return ALDER_UC_SUCCESS;
}
