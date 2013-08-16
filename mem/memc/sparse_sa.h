//
//  sparse_sa.h
//  mem
//
//  Created by Sang Chul Choi on 8/13/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef mem_sparse_sa_h
#define mem_sparse_sa_h

#include <stdint.h>
#include <stdbool.h>
#include <gsl/gsl_vector.h>

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
    uint64_t depth;
    uint64_t start;
    uint64_t end;
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

// Match find by findMEM.
typedef struct
{
    uint64_t ref;   // position in reference sequence
    uint64_t query; // position in query
    uint64_t len;   // length of match
}
alder_match_t;

// This needs implementation.
typedef struct
{
    size_t size;
    size_t stride;
    unsigned long *data;
    gsl_block_ulong *block;
    int owner;
}
alder_vector_match;

typedef struct
{
    char *descr;
    gsl_vector_ulong startpos;
    bool _4column;

    uint64_t K;
    uint64_t N;
    uint64_t logN;
    uint64_t NKm1;
    char *S;
    uint64_t *SA;
    uint64_t *ISA;
    uint64_t *LCP;
}
alder_sparse_sa_t;

// alder_sparse_sa_t
alder_sparse_sa_t *alder_sparse_sa_alloc (const char *ref, uint64_t K);
void alder_sparse_sa_free (alder_sparse_sa_t *o);
void query_thread(alder_sparse_sa_t *o, const char *P);

// Modified Kasai et all for LCP computation.
void alder_computeLCP(alder_sparse_sa_t *o);

// Radix sort required to construct transformed text for sparse SA construction.
int alder_radixStep2(alder_sparse_sa_t *o, uint64_t *t_new, uint64_t *SA,
                    uint64_t *bucketNr_, uint64_t *BucketBegin,
                    uint64_t l, uint64_t r, uint64_t h);
int alder_radixStep(alder_sparse_sa_t *o, uint64_t *t_new, uint64_t *SA,
                    long *bucketNr_, long *BucketBegin, long l, long r, long h);


// Prints match to cout.
//void print_match(match_t m);
//void print_match(match_t m, vector<match_t> &buf); // buffered version
//void print_match(string meta, vector<match_t> &buf, bool rc); // buffered version
void alder_print_match(alder_match_t m);

// Binary search for left boundry of interval.
uint64_t alder_bsearch_left(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t s, uint64_t e);
// Binary search for right boundry of interval.
uint64_t alder_bsearch_right(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t s, uint64_t e);

// Simple suffix array search.
bool alder_search(alder_sparse_sa_t *o, const char *P, uint64_t start, uint64_t end);

// Simple top down traversal of a suffix array.
bool alder_top_down(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t *start, uint64_t *end);
bool alder_top_down_faster(alder_sparse_sa_t *o, char c, uint64_t i,
                           uint64_t *start_, uint64_t *end_);

// Traverse pattern P starting from a given prefix and interval
// until mismatch or min_len characters reached.
void alder_traverse(alder_sparse_sa_t *o, const char *P, 
                    uint64_t prefix, alder_interval_t *cur,
                    uint64_t min_len);

// Simulate a suffix link.
bool alder_suffixlink(alder_sparse_sa_t *o, alder_interval_t *m);

// Expand ISA/LCP interval. Used to simulate suffix links.
bool alder_expand_link(alder_sparse_sa_t *o, alder_interval_t *link); 

// Given a position i in S, finds a left maximal match of minimum
// length within K steps.
void alder_find_Lmaximal(alder_sparse_sa_t *o, const char *P,
                         uint64_t prefix, uint64_t i, uint64_t len,
                         alder_vector_match *matches, uint64_t min_len, 
                         bool print);

// Given an interval where the given prefix is matched up to a
// mismatch, find all MEMs up to a minimum match depth.
void alder_collectMEMs(alder_sparse_sa_t *o,
                       const char *P, uint64_t prefix,
                       alder_interval_t *mli, alder_interval_t *xmi,
                       alder_vector_match *matches, uint64_t min_len,
                       bool print);

// Find all MEMs given a prefix pattern offset k.
int alder_findMEM(alder_sparse_sa_t *o, uint64_t k, const char *P,
                  alder_vector_match *matches, uint64_t min_len, bool print);

// Find Maximal Exact Matches (MEMs)
int alder_MEM(alder_sparse_sa_t *o, const char *P, alder_vector_match *matches,
              uint64_t min_len, bool print, int num_threads);

// adler_interval_t
void alder_interval_reset(alder_interval_t *o, uint64_t e);
uint64_t alder_interval_size(alder_interval_t *o);

__END_DECLS

#endif
