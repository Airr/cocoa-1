/**
 * This file, alder_fileseq_token.h, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fileseq_alder_fileseq_token_h
#define alder_fileseq_alder_fileseq_token_h

#include <stdint.h>
#include "bstrlib.h"
#include "alder_fasta_token.h"

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

typedef struct alder_fileseq_token_struct {
    int type;
    /* FASTA */
    alder_fasta_token_t *fasta;
    /* BSEQ */
    bstring bseq;
    int bseqI;
} alder_fileseq_token_t;

alder_fileseq_token_t*
alder_fileseq_token_create(const char *fn);

alder_fileseq_token_t*
alder_fileseq_token_createWithBstring(bstring bseq);

void
alder_fileseq_token_setWithBstring(alder_fileseq_token_t *o,
                                   bstring bseq);

void
alder_fileseq_token_destroy(alder_fileseq_token_t *o);

int
alder_fileseq_token_next(alder_fileseq_token_t *o);

bool
alder_fileseq_token_hasNext(alder_fileseq_token_t *o);

bool
alder_fileseq_token_moreSequence(alder_fileseq_token_t *o);

__END_DECLS


#endif /* alder_fileseq_alder_fileseq_token_h */
