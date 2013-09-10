/**
 * This file, alder_ssa.h, is part of alder-ssa.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-ssa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-ssa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-ssa.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_ssa_alder_ssa_h
#define alder_ssa_alder_ssa_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_gsl.h"
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

// depth : [start...end]
typedef struct
{
    int64_t depth;
    int64_t start;
    int64_t end;
}
alder_interval_t;

// Stores the LCP array in an unsigned char (0-255).  Values larger
// than or equal to 255 are stored in a sorted array.
// Simulates a vector<int> LCP;
typedef struct
{
    size_t idx;
    int val;
}
alder_lcp_t;

/** Sparse suffix array
 *
 */
typedef struct
{
    // (N) N-th order in memory allocation.
    int64_t K;    ///< Sparse suffix array stores every K-th suffix.
    int64_t N;    ///< Length of the text with dollar signs.
    int64_t logN; ///< = floor[log_2 N] - convenient members.
    int64_t NKm1; ///< = N/K-1 - convenient members.
    int64_t *SA;  ///< Suffix array.          (2)
    int64_t *ISA; ///< Inverse suffix array.  (3)
    int64_t *LCP; ///< Longest common prefix. (4)
    alder_fasta_t *fS;  ///< Data information (1)
    char     *S;  ///< (No ownership) Pointer to a string in fS
} alder_sparse_sa_t;

alder_sparse_sa_t *alder_sparse_sa_alloc_file (const struct bstrList *ref, const int64_t K);

void alder_sparse_sa_free (alder_sparse_sa_t *o);

// Modified Kasai et all for LCP computation.
void alder_computeLCP(alder_sparse_sa_t *o);

// Radix sort required to construct transformed text for sparse SA construction.
int alder_radixStep(alder_sparse_sa_t *o, uint8_t *t_new, int64_t *SA,
                    uint16_t *bucketNr_, int64_t *BucketBegin,
                    int64_t l, int64_t r, int64_t h);

void alder_print_match(alder_match_t m);

// Binary search for left boundry of interval.
int64_t alder_bsearch_left(alder_sparse_sa_t *o, char c, int64_t i, int64_t s, int64_t e);
// Binary search for right boundry of interval.
int64_t alder_bsearch_right(alder_sparse_sa_t *o, char c, int64_t i, int64_t s, int64_t e);

// Simple suffix array search.
bool alder_search(alder_sparse_sa_t *o, const char *P, int64_t start, int64_t end);

// Simple top down traversal of a suffix array.
bool alder_top_down(alder_sparse_sa_t *o, char c, int64_t i, int64_t *start, int64_t *end);
bool alder_top_down_faster(alder_sparse_sa_t *o, char c, int64_t i,
                           int64_t *start_, int64_t *end_);

// Traverse pattern P starting from a given prefix and interval
// until mismatch or min_len characters reached.
void alder_traverse(alder_sparse_sa_t *o, const char *P, 
                    int64_t prefix, alder_interval_t *cur,
                    int64_t min_len);

// Simulate a suffix link.
bool alder_suffixlink(alder_sparse_sa_t *o, alder_interval_t *m);

// Expand ISA/LCP interval. Used to simulate suffix links.
bool alder_expand_link(alder_sparse_sa_t *o, alder_interval_t *link); 

// Given a position i in S, finds a left maximal match of minimum
// length within K steps.
void alder_find_Lmaximal(alder_sparse_sa_t *o, const char *P,
                         int64_t prefix, int64_t i, int64_t len,
                         gsl_vector_match *matches, int64_t min_len,
                         bool print);

// Given an interval where the given prefix is matched up to a
// mismatch, find all MEMs up to a minimum match depth.
void alder_collectMEMs(alder_sparse_sa_t *o,
                       const char *P, int64_t prefix,
                       alder_interval_t *mli, alder_interval_t *xmi,
                       gsl_vector_match *matches, int64_t min_len,
                       bool print);

// Find all MEMs given a prefix pattern offset k.
int alder_findMEM(alder_sparse_sa_t *o, int64_t k, const char *P,
                  gsl_vector_match *matches, int64_t min_len, bool print);

// Find Maximal Exact Matches (MEMs)
int alder_MEM(alder_sparse_sa_t *o, const char *P, gsl_vector_match *matches,
              int64_t min_len, bool print, int num_threads);

// adler_interval_t
void alder_interval_reset(alder_interval_t *o, int64_t e);
int64_t alder_interval_size(alder_interval_t *o);

__END_DECLS


#endif /* alder_ssa_alder_ssa_h */
