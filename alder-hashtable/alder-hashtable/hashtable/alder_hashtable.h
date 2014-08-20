/**
 * This file, alder_hashtable.h, is part of alder-hashtable.
 *
 * Copyright 2013 by Sang Chul Choi
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
#ifndef alder_hashtable_alder_hashtable_h
#define alder_hashtable_alder_hashtable_h

#include "alder_hashtable_mcas.h"
#include "alder_hashtable_mcas1.h"

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

int alder_hashtable00();
int alder_hashtable01();

__END_DECLS

#endif /* alder_hashtable_alder_hashtable_h */
