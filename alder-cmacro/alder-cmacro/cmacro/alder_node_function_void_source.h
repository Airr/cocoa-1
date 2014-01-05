/**
 * This file, alder_node_function_void_source.h, is part of alder-segtree.
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
static int
compare_void(const void *c1, const void *c2)
{
    const void *a1 = c1;
    const void *a2 = c2;
    if (a1 < a2) {
        return -1;
    } else if (a1 > a2) {
        return +1;
    } else {
        return 0;
    }
}
