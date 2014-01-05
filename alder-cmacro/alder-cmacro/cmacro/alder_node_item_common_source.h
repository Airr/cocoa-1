/**
 * This file, alder_node_item_common_source.h, is part of alder-rbtree.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-rbtree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-rbtree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-rbtree.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This struct manages memory of homogeneous data.
 */
struct MEMORY_TYPE {
    TYPE **head;
    TYPE *current;
    TYPE *free;
    int size;
    size_t nhead;
};
