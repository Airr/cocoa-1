/**
 * This file, alder_number.h, is part of alder-number.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-number is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-number is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-number.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_number_alder_number_h
#define alder_number_alder_number_h

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

typedef struct alder_number_struct {
    uint64_t *n;
    size_t s;
} alder_number_t;

alder_number_t * alder_number_create(size_t size);
void alder_number_destroy(alder_number_t *o);

__END_DECLS

#endif /* alder_number_alder_number_h */
