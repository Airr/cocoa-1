/**
 * This file, alder_rng.h, is part of alder-rng.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-rng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-rng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-rng.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_rng_alder_rng_h
#define alder_rng_alder_rng_h

#include <stdint.h>
#include "rngs.h"
#include "rvgs.h"

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

uint64_t alder_rng_uint64(uint64_t m);

__END_DECLS

#endif /* alder_rng_alder_rng_h */
