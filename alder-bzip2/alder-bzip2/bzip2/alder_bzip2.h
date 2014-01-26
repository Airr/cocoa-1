/**
 * This file, alder_bzip2.h, is part of alder-bzip2.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-bzip2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-bzip2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-bzip2.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_bzip2_alder_bzip2_h
#define alder_bzip2_alder_bzip2_h

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

void alder_bzip2_decompress(const char *srcfn, const char *dstfn);

__END_DECLS


#endif /* alder_bzip2_alder_bzip2_h */
