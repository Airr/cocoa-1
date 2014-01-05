/**
 * This file, alder_node_stack_header.h, is part of alder-stack.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-stack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-stack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-stack.  If not, see <http://www.gnu.org/licenses/>.
*/

typedef struct STACK_TYPE_STRUCT {
    BASE **base;
    BASE **top;
    int size;
    struct STACK_TYPE_STRUCT *prev;
} STACK_TYPE;

STACK_TYPE * FUNCTION(alder_stack_create)(int size);
int FUNCTION(alder_stack_empty)(STACK_TYPE *s);
void FUNCTION(alder_stack_push)(STACK_TYPE *s, BASE *x);
BASE* FUNCTION(alder_stack_pop)(STACK_TYPE *s);
BASE* FUNCTION(alder_stack_top)(STACK_TYPE *s);
void FUNCTION(alder_stack_destroy)(STACK_TYPE *s);
