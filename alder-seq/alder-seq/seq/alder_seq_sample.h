/**
 * This file, alder_seq_sample.h, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_wordtable_alder_seq_sample_h
#define alder_wordtable_alder_seq_sample_h

#include <stdint.h>
#include "bstrlib.h"
#include "bstrmore.h"

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

bstring alder_seq_sample_sequence(size_t n);
int alder_seq_sample(const char *fn, size_t n_seq, size_t l_seq);
struct bstrList* alder_seq_sample_bstrList(size_t n_seq, size_t l_seq);

__END_DECLS


#endif /* alder_wordtable_alder_seq_sample_h */
