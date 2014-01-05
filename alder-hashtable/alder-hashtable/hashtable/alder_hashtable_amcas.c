/**
 * This file, alder_hashtable_amcas.c, is part of alder-hashtable.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-hashtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hashtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hashtable.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_hashtable_amcas.h"

struct alder_hashtable_amcas_struct {
};

alder_hashtable_amcas_t*
alder_hashtable_amcas_create()
{
    alder_hashtable_amcas_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    return o;
}

void
alder_hashtable_amcas_destroy(alder_hashtable_amcas_t *o)
{
    if (o != NULL) {
        XFREE(o);
    }
}

void
alder_hashtable_amcas_init(alder_hashtable_amcas_t *o)
{
}

