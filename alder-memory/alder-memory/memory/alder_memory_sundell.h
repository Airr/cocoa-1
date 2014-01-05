/**
 * This file, alder_memory_sundell.h, is part of alder-memory.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-memory is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-memory is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-memory.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_memory_alder_memory_sundell_h
#define alder_memory_alder_memory_sundell_h

#include <stdint.h>
#include "alder_type.h"

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

int alder_memory_sundell();
int alder_memory_sundell_initialize(int n_thread, int n_index);
int alder_memory_sundell_finalize(int n_thread, int n_index);
word_t alder_memory_sundell_WFRead(word_t *address,
                                   alder_type_sundell_descriptor_t **desc,
                                   int threadID);
alder_type_sundell_descriptor_t* alder_memory_sundell_AllocNode(int threadId);
alder_type_sundell_descriptor_t*
alder_memory_sundell_DeRefLink(alder_type_sundell_descriptor_t **link,
                               int threadId);
void alder_memory_sundell_ReleaseRef(alder_type_sundell_descriptor_t *node,
                                     int threadId);
void alder_memory_sundell_HelpDeRef(alder_type_sundell_descriptor_t **link,
                                    int threadId);
bool alder_memory_sundell_CompareAndSwapLink(alder_type_sundell_descriptor_t **link,
                                             alder_type_sundell_descriptor_t *old,
                                             alder_type_sundell_descriptor_t *new,
                                             int threadId);

__END_DECLS


#endif /* alder_memory_alder_memory_sundell_h */
