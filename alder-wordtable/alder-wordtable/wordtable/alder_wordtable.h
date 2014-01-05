/**
 * This file, alder_wordtable.h, is part of alder-wordtable.
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
#ifndef alder_wordtable_alder_wordtable_h
#define alder_wordtable_alder_wordtable_h

#include "alder_wordtable_tokenset.h"
#include "alder_wordtable_kfmindex.h"
#include "alder_wordtable_sequenceiterator.h"
#include "alder_wordtable_bigbitarray.h"
#include "alder_wordtable_bitpackedcumulativearray.h"
#include "alder_wordtable_bitpackedlist.h"
#include "alder_wordtable_positionlist.h"
#include "alder_wordtable_positionvaluelist.h"
#include "alder_wordtable_bits.h"
#include "alder_wordtable_kfmmerger.h"

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

alder_wordtable_kfmindex_t*
alder_wordtable_kFMindexFromFastaFileUsingAggregator(const char *fn,
                                                     size_t order,
                                                     size_t blocksize);


alder_wordtable_kfmindex_t*
alder_wordtable_kFMindexFromFastaFile(const char *fn, size_t order);

alder_wordtable_kfmindex_t*
alder_wordtable_kFMindexFromFastqFile(const char *fn, size_t order,
                                      size_t quality, size_t blocksize);

#pragma mark test

void
alder_wordtable_sequenceiterator_test(struct bstrList *fn);

void
alder_wordtable_bigbitarray_test();

void
alder_wordtable_positionlist_test();

void
alder_wordtable_positionvaluelist_test();

void
alder_wordtable_bitpackedlist_test();

void
alder_wordtable_bitpackedcumulativearray_test();

void
alder_wordtable_bits_test();

void
alder_wordtable_tokeniterator_test();

void
alder_wordtable_intblocklist_test();

void
alder_wordtable_inedgelist_test();

void
alder_wordtable_inedgedata_test();

void
alder_wordtable_kfmindex_test();

void
alder_wordtable_kfminterval_test();

void
alder_wordtable_kfmmerge_test();

void
alder_wordtable_kfmprune_test();

void
alder_wordtable_kfmaggregator_test(struct bstrList *infile,
                                   const char *fn);
void
alder_wordtable_kfmio_test(const char *fn);

void
alder_wordtable_kfmsim_test();

void
alder_wordtable_kfmcheck_test();

void
alder_wordtable_kfmfind_test();

__END_DECLS

#endif /* alder_wordtable_alder_wordtable_h */
