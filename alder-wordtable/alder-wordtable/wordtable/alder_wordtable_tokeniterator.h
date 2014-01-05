/**
 * This file, alder_wordtable_tokeniterator.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_tokeniterator_h
#define alder_wordtable_alder_wordtable_tokeniterator_h

#include <stdint.h>
#include <stdbool.h>

#include "alder_wordtable_kfmdefaults.h"
#include "alder_fileseq_token.h"

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

typedef struct alder_wordtable_tokeniterator_struct {
    
    alder_fileseq_token_t *fileseq;
    
    /* splitter */
    bool isBuffered;
    size_t remaining;
    size_t pos;
    int *buffer;
    size_t bufferLength;
} alder_wordtable_tokeniterator_t;

alder_wordtable_tokeniterator_t*
alder_wordtable_tokeniterator_create(const char *fn);

alder_wordtable_tokeniterator_t*
alder_wordtable_tokeniterator_createWithBstring(bstring bseq);

void
alder_wordtable_tokeniterator_setWithBstring(alder_wordtable_tokeniterator_t *o,
                                             bstring bseq);

void
alder_wordtable_tokeniterator_destroy(alder_wordtable_tokeniterator_t *o);

bool
alder_wordtable_tokeniterator_hasNext(alder_wordtable_tokeniterator_t *o);
int
alder_wordtable_tokeniterator_next(alder_wordtable_tokeniterator_t *o);

bool
alder_wordtable_tokeniterator_moreSequence(alder_wordtable_tokeniterator_t *o);

void
alder_wordtable_tokeniterator_set(alder_wordtable_tokeniterator_t *o,
                                  size_t maxlength,
                                  size_t bufferLength);

//void
//alder_wordtable_tokeniterator_set(alder_wordtable_tokeniterator_t *o,
//                                  size_t maxlength);

bool
alder_wordtable_tokeniterator_hasMore(alder_wordtable_tokeniterator_t *o);

void
alder_wordtable_tokeniterator_reset(alder_wordtable_tokeniterator_t *o,
                                    size_t maxlength);


__END_DECLS


#endif /* alder_wordtable_alder_wordtable_tokeniterator_h */
