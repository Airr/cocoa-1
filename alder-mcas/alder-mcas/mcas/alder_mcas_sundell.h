/**
 * This file, alder_mcas_sundell.h, is part of alder-mcas.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-mcas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-mcas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-mcas.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_mcas_alder_mcas_sundell_h
#define alder_mcas_alder_mcas_sundell_h

#include <stdint.h>
#include "alder_type_sundell.h"

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

word_t alder_mcas_sundell_Read(word_t *address, int threadId);
bool alder_mcas_sundell_CASN(const int N, word_t **address,
                             word_t *oldvalue, word_t *newvalue,
                             int threadID);

__END_DECLS


#endif /* alder_mcas_alder_mcas_sundell_h */
