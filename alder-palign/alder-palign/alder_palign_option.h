/**
 * This file, alder_palign_option.h, is part of alder-palign.
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

#ifndef alder_palign_alder_palign_option_h
#define alder_palign_alder_palign_option_h

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
} alder_palign_t;

int alder_palign_init(alder_palign_t *o, struct gengetopt_args_info *a);
void alder_palign_free(alder_palign_t *o);

__END_DECLS


#endif /* alder_palign_alder_palign_option_h */
