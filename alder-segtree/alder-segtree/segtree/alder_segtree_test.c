/**
 * This file, alder_segtree_test.c, is part of alder-segtree.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-segtree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-segtree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-segtree.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_segtree.h"
#include "alder_segtree_test.h"

void alder_segtree_test()
{
    alder_segtree_t *seg = alder_segtree_create();
    
    alder_segtree_destroy(seg);
}