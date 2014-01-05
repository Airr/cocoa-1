/**
 * This file, alder_node_function_tuple2_source.h, is part of alder-uc.
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

static int
compare_tuple2(const void *c1, const void *c2)
{
    const alder_item_tuple2_node_t *a1 = c1;
    const alder_item_tuple2_node_t *a2 = c2;
    if (a1->e1 < a2->e1) {
        return -1;
    } else if (a1->e1 > a2->e1) {
        return +1;
    } else {
        if (a1->e2 < a2->e2) {
            return -1;
        } else if (a1->e2 > a2->e2) {
            return +1;
        } else {
            return 0;
        }
    }
}

static void
print_tuple2(FILE *fp, const void *c)
{
    const alder_item_tuple2_node_t *a = c;
    fprintf(fp, "%lld:%lld", a->e1, a->e2);
}
