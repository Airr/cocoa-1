/**
 * This file, alder_cigar_option.h, is part of alder-cigar.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-cigar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-cigar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-cigar.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_cigar_alder_cigar_option_h
#define alder_cigar_alder_cigar_option_h

#include "bstrlib.h"

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

typedef struct {
    struct bstrList *infile;
    struct bstrList *outfile;
    bstring logfilename;
} alder_cigar_option_t;

int alder_cigar_option_init(alder_cigar_option_t *o, struct gengetopt_args_info *a);
void alder_cigar_option_free(alder_cigar_option_t *o);

__END_DECLS


#endif /* alder_cigar_alder_cigar_option_h */
