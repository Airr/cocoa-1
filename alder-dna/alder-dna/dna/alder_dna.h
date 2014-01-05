/**
 * This file, alder_dna.h, is part of alder-dna.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-dna is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-dna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-dna.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_dna_alder_dna_h
#define alder_dna_alder_dna_h

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

char alder_dna_int2char(int i);
int alder_dna_char2int(char c);

__END_DECLS


#endif /* alder_dna_alder_dna_h */
