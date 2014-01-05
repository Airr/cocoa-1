/**
 * This file, alder_stack_void.h, is part of alder-stack.
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

#define ALDER_BASE_VOID_NODE
#include "alder_node_memory_template_on.h"

#ifndef alder_stack_alder_stack_void_h
#define alder_stack_alder_stack_void_h

#include <stdint.h>

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

#include "alder_node_stack_header.h"

__END_DECLS


#endif /* alder_stack_alder_stack_void_h */

#include "alder_node_memory_template_off.h"
#undef  ALDER_BASE_VOID_NODE