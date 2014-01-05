/**
 * This file, alder_fasta_token.h, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fasta_alder_fasta_token_h
#define alder_fasta_alder_fasta_token_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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

typedef struct alder_fasta_token_struct {
    bstring bfn;
    FILE *fp;
    char *buf;
    size_t bufsize; /* capacity of buf */
    size_t bufi;    /* current position in buf */
    size_t bufend;  /* end of content in buf */
    bool moreSequence;
    bool endOfFile;
} alder_fasta_token_t;

alder_fasta_token_t*
alder_fasta_token_create(const char *fn);

void
alder_fasta_token_destroy(alder_fasta_token_t *o);

int
alder_fasta_token_next(alder_fasta_token_t *o);

bool
alder_fasta_token_hasNext(alder_fasta_token_t *o);

bool
alder_fasta_token_moreSequence(alder_fasta_token_t *o);

__END_DECLS


#endif /* alder_fasta_alder_fasta_token_h */
