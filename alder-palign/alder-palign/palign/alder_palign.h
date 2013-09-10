/**
 * This file, alder_palign.h, is part of alder-palign.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-palign is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-palign is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-palign.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_palign_alder_palign_h
#define alder_palign_alder_palign_h

#include "alder_interval.h"
#include "alder_gsl.h"
#include "alder_vector.h"
#include "gsl_vector_sam.h"
#include "alder_fasta.h"

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

gsl_vector_sam * alder_palign_align_in_sam(alder_vector_palign_t *palign,
                                           const alder_fasta_t *fS,
                                           const char *queryName);

#if 0
int alder_palign(const char *s, interval_t I, gsl_vector_match *mem, alder_fasta_t *fS);
#endif 

__END_DECLS

#endif /* alder_palign_alder_palign_h */
