/**
 * This file, alder_encode.h, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_encode_alder_encode_h
#define alder_encode_alder_encode_h

#include <stdint.h>
#include "alder_encode_kmer.h"
#include "alder_encode_number.h"
#include "alder_encode_pack.h"

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

enum {
    ALDER_ENCODE_SUCCESS,
    ALDER_ENCODE_ERROR
};
int alder_encode_uint64_to_8char(char *dest, uint64_t source);
int alder_encode_8char_to_uint64(uint64_t *dest, char *source);

__END_DECLS

#endif /* alder_encode_alder_encode_h */
