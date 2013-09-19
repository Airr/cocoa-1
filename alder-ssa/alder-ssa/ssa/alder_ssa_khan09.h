/**
 * This file, alder_ssa_khan09.h, is part of alder-denovo.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-denovo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-denovo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-denovo.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_denovo_alder_ssa_khan09_h
#define alder_denovo_alder_ssa_khan09_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_ssa.h"
#include "gsl_vector_match.h"

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

//typedef struct
//{
//    int64_t depth;
//    int64_t start;
//    int64_t end;
//}
//alder_interval_t;

int alder_ssa_khan09_appendDollarSign(char *S,
                                      const int64_t N,
                                      const int64_t M,
                                      const int64_t K,
                                      int64_t *newN);

void alder_ssa_khan09_computeLCP(const char *S,
                                 const int64_t *SA,
                                 const int64_t *ISA,
                                 int64_t *LCP,
                                 const int64_t N,
                                 const int64_t K);

int alder_ssa_khan09_radixStep(const char *S, const int64_t K,
                               const size_t BUCKETBEGINSIZE,
                               uint8_t *t_new, int64_t *SA,
                               uint16_t *bucketNr_, int64_t *BucketBegin,
                               int64_t l, int64_t r, int64_t h);

int64_t alder_ssa_khan09_bsearch_left(const char *S, const int64_t *SA,
                                      char c, int64_t i, int64_t s, int64_t e);

int64_t alder_ssa_khan09_bsearch_right(const char *S, const int64_t *SA,
                                       char c, int64_t i, int64_t s, int64_t e);

bool alder_ssa_khan09_top_down(const char *S, const int64_t *SA,
                               char c, int64_t i, int64_t *start, int64_t *end);

bool alder_ssa_khan09_search(const char *S, const int64_t *SA, const int64_t N,
                             const char *P, int64_t start, int64_t end);

bool alder_ssa_khan09_top_down_faster(const char *S, const int64_t *SA,
                                      char c, int64_t i,
                                      int64_t *start_, int64_t *end_);

void alder_ssa_khan09_traverse(const char *S, const int64_t *SA,
                               const char *P,
                               int64_t prefix, alder_interval_t *cur,
                               int64_t min_len);

bool alder_ssa_khan09_suffixlink(const int64_t *SA,
                                 const int64_t *ISA,
                                 const int64_t *LCP,
                                 const int64_t K,
                                 const int64_t logN,
                                 const int64_t NKm1,
                                 alder_interval_t *m);

bool alder_ssa_khan09_expand_link(const int64_t *LCP,
                                  const int64_t logN,
                                  const int64_t NKm1,
                                  alder_interval_t *link);

int alder_ssa_khan09_findMEM(const char *S,
                             const int64_t *SA,
                             const int64_t *ISA,
                             const int64_t *LCP,
                             const int64_t N,
                             const int64_t K,
                             const int64_t logN,
                             const int64_t NKm1,
                             int64_t k, const char *P,
                             gsl_vector_match *matches, int64_t min_len, bool print);

void alder_ssa_khan09_collectMEMs(const char *S,
                                  const int64_t *SA,
                                  const int64_t *LCP,
                                  const int64_t N,
                                  const int64_t K,
                                  const char *P, int64_t prefix,
                                  alder_interval_t *mli, alder_interval_t *xmi,
                                  gsl_vector_match *matches, int64_t min_len,
                                  bool print);

void alder_ssa_khan09_find_Lmaximal(const char *S, const int64_t K,
                                    const char *P,
                                    int64_t prefix, int64_t i, int64_t len,
                                    gsl_vector_match *matches, int64_t min_len,
                                    bool print);

int alder_ssa_khan09_MEM(const char *S,
                         const int64_t *SA,
                         const int64_t *ISA,
                         const int64_t *LCP,
                         const int64_t N,
                         const int64_t K,
                         const int64_t logN,
                         const int64_t NKm1,
                         const char *P, gsl_vector_match *matches,
                         int64_t min_len, bool print, int num_threads);

void alder_ssa_khan09_interval_reset(alder_interval_t *o, int64_t e);
int64_t alder_ssa_khan09_interval_size(alder_interval_t *o);
void alder_ssa_khan09_print_match(alder_match_t m);

__END_DECLS


#endif /* alder_denovo_alder_ssa_khan09_h */
