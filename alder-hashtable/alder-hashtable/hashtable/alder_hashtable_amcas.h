/**
 * This file, alder_hashtable_amcas.h, is part of alder-hashtable.
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

#ifndef alder_hashtable_alder_hashtable_amcas_h
#define alder_hashtable_alder_hashtable_amcas_h

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

typedef struct alder_hashtable_amcas_struct 
alder_hashtable_amcas_t;

alder_hashtable_amcas_t*
alder_hashtable_amcas_create();

void
alder_hashtable_amcas_destroy(alder_hashtable_amcas_t *o);

void
alder_hashtable_amcas_init(alder_hashtable_amcas_t *o);

__END_DECLS


#endif /* alder_hashtable_alder_hashtable_amcas_h */
