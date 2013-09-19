/**
 * This file, alder_ssa_khan09.c, is part of alder-denovo.
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

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_errno.h>
#include "alder_interval.h"
#include "alder_cmacro.h"
#include "alder_ssa_khan09.h"

/* This function appends dollar signs to a string.
 * S - a string
 * N - the size of string S
 * M - the size of memory for S
 * K - sparseness of a suffix array
 * newN - the new size of string S with appended dollar signs
 * returns 0 on success, -1 otherwise.
 */
int alder_ssa_khan09_appendDollarSign(char *S,
                                      const int64_t N,
                                      const int64_t M,
                                      const int64_t K,
                                      int64_t *newN)
{
    // Increase string length so divisible by K.
    // Don't forget to count $ termination character.
    int64_t appendK = N % K == 0 ? K : K - N % K;
    if (M < N + appendK + 1) {
        return -1;
    }
    for(int64_t i = 0; i < appendK; i++) {
        S[N + i] = '$';
    }
    S[N + appendK] = '\0';
    *newN = N + appendK;
    if ((*newN)%K != 0) {
        return -1;
    }
    return 0;
}

// Uses the algorithm of Kasai et al 2001 which was described in
// Manzini 2004 to compute the LCP array. Modified to handle sparse
// suffix arrays and inverse sparse suffix arrays.
void alder_ssa_khan09_computeLCP(const char *S,
                                 const int64_t *SA,
                                 const int64_t *ISA,
                                 int64_t *LCP,
                                 const int64_t N,
                                 const int64_t K)
                                 
{
//    int64_t K = o->K;
//    int64_t N = o->N;
//    char *S = o->S;
//    int64_t *SA = o->SA;
//    int64_t *ISA = o->ISA;
//    int64_t *LCP = o->LCP;
    
    int64_t h=0;
    for(int64_t i = 0; i < N; i+=K) {
        int64_t m = ISA[i/K];
        if(m==0) LCP[m]=0; // LCP.set(m, 0); // LCP[m]=0;
        else {
            int64_t j = SA[m-1];
            while(i+h < N && j+h < N && S[i+h] == S[j+h])  h++;
            LCP[m] = h; // LCP.set(m, h); //LCP[m] = h;
        }
        if (h > K) {
            h = h - K;
        } else {
            h = 0;
        }
//        h = max(0L, h - K);
    }
}


int alder_ssa_khan09_radixStep(const char *S, const int64_t K,
                               const size_t BUCKETBEGINSIZE,
                               uint8_t *t_new, int64_t *SA,
                               uint16_t *bucketNr_, int64_t *BucketBegin,
                               int64_t l, int64_t r, int64_t h)
{
    // Note: SA is a local variable not o->SA.
//    char *S          = o->S;
//    int64_t K        = o->K;
    uint16_t bucketNr = *bucketNr_;
    assert(1 < K && K <= 4);
    assert(sizeof(size_t) == sizeof(int64_t));

    if(h >= K) {
        GSL_ERROR("radixStep failed", GSL_EINVAL);
    }
    // first pass: count
    int64_t *Sigma = malloc(BUCKETBEGINSIZE * sizeof(int64_t));
    if (Sigma == NULL) {
        GSL_ERROR("radixStep failed", GSL_ENOMEM);
    }
    memset(Sigma, 0, BUCKETBEGINSIZE * sizeof(int64_t));
    //vector<long> Sigma(256, 0); // Sigma counts occurring characters in bucket
    for (size_t i = l; i <= r; i++) {
        Sigma[ S[ SA[i] * K + h ] ]++;
    }
    BucketBegin[0] = l;
    for (size_t i = 1; i < BUCKETBEGINSIZE ; i++) {
        BucketBegin[i] = Sigma[i-1] + BucketBegin[i-1];
    } // accumulate
    
    // second pass: move (this variant does *not* need an additional array!)
    uint8_t currentKey = 0;    // character of current bucket
    int64_t end = l-1+Sigma[currentKey]; // end of current bucket
    int64_t pos = l;                     // 'pos' is current position in bucket
    while (1) {
        if (pos > end) { // Reached the end of the bucket.
            if (currentKey == 255) break; // Last character?
            currentKey++; // Advance to next characer.
            pos = BucketBegin[currentKey]; // Next bucket start.
            end += Sigma[currentKey]; // Next bucket end.
        }
        else {
            // American flag sort of McIlroy et al. 1993. BucketBegin keeps
            // track of current position where to add to bucket set.
            int64_t tmp = SA[ BucketBegin[ S[ SA[pos]*K + h ] ] ];
            SA[ BucketBegin[ S[ SA[pos]*K + h] ]++ ] = SA[pos];  // Move bucket beginning to the right, and replace
            SA[ pos ] = tmp; // Save value at bucket beginning.
            if (S[ SA[pos]*K + h ] == currentKey) pos++; // Advance to next position if the right character.
        }
    }
    // recursively refine buckets and calculate new text:
    assert(l >= 0);
    int64_t beg = l;
    end = l-1;
    for (size_t i = 1; i < BUCKETBEGINSIZE; i++) { // step through Sigma to find bucket borders
        end += Sigma[i];
        if (beg <= end) {
            if(h == K-1) {
                for (size_t j = beg; j <= end; j++) {
                    assert(bucketNr < BUCKETBEGINSIZE);
                    t_new[ SA[j] ] = bucketNr; // set new text
                }
                bucketNr++;
            }
            else {
                alder_ssa_khan09_radixStep(S, K, BUCKETBEGINSIZE, t_new, SA, &bucketNr, BucketBegin, beg, end, h+1); // recursive refinement
            }
            beg = end + 1; // advance to next bucket
        }
    }
    
    // Clean up!
    free(Sigma);
    *bucketNr_ = bucketNr;
    return GSL_SUCCESS;
}

// Binary search for left boundry of interval.
int64_t alder_ssa_khan09_bsearch_left(const char *S, const int64_t *SA,
                                      char c, int64_t i, int64_t s, int64_t e)
{
//    char *S = o->S;
//    int64_t *SA = o->SA;
    
    if(c == S[SA[s]+i]) return s;
    int64_t l = s;
    int64_t r = e;
//    while (r > l + 1) {
    while (r - l > 1) {
        long m = (l+r) / 2;
        if (c <= S[SA[m] + i]) r = m;
        else l = m;
    }
    return r;
}

// Binary search for right boundry of interval.
int64_t alder_ssa_khan09_bsearch_right(const char *S, const int64_t *SA,
                                       char c, int64_t i, int64_t s, int64_t e)
{
//    char *S = o->S;
//    int64_t *SA = o->SA;

    if(c == S[SA[e]+i]) return e;
    int64_t l = s;
    int64_t r = e;
    while (r > l + 1) {
//    while (r - l > 1) {
        long m = (l+r) / 2;
        if (c < S[SA[m] + i]) r = m;
        else l = m;
    }
    return l;
}


// Simple top down traversal of a suffix array.
bool alder_ssa_khan09_top_down(const char *S, const int64_t *SA,
                               char c, int64_t i, int64_t *start, int64_t *end)
{
//    int64_t *SA = o->SA;
//    char *S = o->S;
    
    if(c < S[SA[*start]+i]) return false;
    if(c > S[SA[*end]+i]) return false;
    int64_t l = alder_ssa_khan09_bsearch_left(S, SA, c, i, *start, *end);
    int64_t l2 = alder_ssa_khan09_bsearch_right(S, SA, c, i, *start, *end);
    *start = l;
    *end = l2;
    return l <= l2;
}

// Top down traversal of the suffix array to match a pattern.  NOTE:
// NO childtab as in the enhanced suffix array (ESA).
bool alder_ssa_khan09_search(const char *S, const int64_t *SA, const int64_t N,
                             const char *P, int64_t start, int64_t end)
{
//    int64_t N = o->N;
    int64_t P_length = strlen(P);
    
    start = 0; end = N - 1;
    int64_t i = 0;
    while(i < P_length) {
        if(alder_ssa_khan09_top_down(S, SA, P[i], i, &start, &end) == false) {
            return false;
        }
        i++;
    }
    return true;
}



// Given SA interval apply binary search to match character c at
// position i in the search string. Adapted from the C++ source code
// for the wordSA implementation from the following paper: Ferragina
// and Fischer. Suffix Arrays on Words. CPM 2007.
bool alder_ssa_khan09_top_down_faster(const char *S, const int64_t *SA,
                                      char c, int64_t i,
                                      int64_t *start_, int64_t *end_)
{
    int64_t start = *start_;
    int64_t end = *end_;
//    char *S = o->S;
//    int64_t *SA = o->SA;
    int64_t l;
    int64_t r;
    int64_t m;
    int64_t r2=end;
    int64_t l2=start;
    long vgl;
    bool found = false;
    long cmp_with_first = (long)c - (long)S[SA[start]+i];
    long cmp_with_last = (long)c - (long)S[SA[end]+i];
    
    if(cmp_with_first < 0) {
        l = start+1;
        l2 = start; // pattern doesn't occur!
    }
    else if(cmp_with_last > 0) {
        l = end+1;
        l2 = end;
        // pattern doesn't occur!
    }
    else {
        // search for left border:
        l = start;
        r = end;
        if (cmp_with_first == 0) {
            found = true;
            r2 = r;
        }
        else {
            while (r > l + 1) {
//            while (r - l > 1) {
                m = (l+r) / 2;
                vgl = (long)c - (long)S[SA[m] + i];
                if (vgl <= 0) {
                    if (!found && vgl == 0) {
                        found = true;
                        l2 = m; r2 = r; // search interval for right border
                    }
                    r = m;
                }
                else l = m;
            }
            l = r;
        }
        // search for right border (in the range [l2:r2])
        if (!found) {
            l2 = l - 1; // pattern not found => right border to the left of 'l'
        }
        if (cmp_with_last == 0) {
            l2 = end; // right border is the end of the array
        }
        else {
            while (r2 > l2 + 1) {
//            while (r2 - l2 > 1) {
                assert(r < UINT64_MAX);
                m = (l2 + r2) / 2;
                vgl = (long)c - (long)S[SA[m] + i];
                if (vgl < 0) r2 = m;
                else l2 = m;
            }
        }
    }
    start = l;
    end = l2;
    
    // Return values.
    *start_ = start;
    *end_ = end;
    return l <= l2;
}


// Traverse pattern P starting from a given prefix and interval
// until mismatch or min_len characters reached.
// Return values: cur
void alder_ssa_khan09_traverse(const char *S, const int64_t *SA,
                               const char *P,
                               int64_t prefix, alder_interval_t *cur,
                               int64_t min_len)
{
    int64_t P_length = strlen(P);
    
    if(cur->depth >= min_len) return;
    
    while(prefix+cur->depth < P_length)
    {
        int64_t start = cur->start;
        int64_t end = cur->end;
        // If we reach a mismatch, stop.
        bool isSuccess = alder_ssa_khan09_top_down_faster(S, SA,
                                                          P[prefix+cur->depth],
                                                          cur->depth,
                                                          &start, &end);
        if(isSuccess == false) return;
//        if(top_down_faster(P[prefix+cur.depth], cur.depth, start, end) == false) return;
        
        // Advance to next interval.
        cur->depth += 1;
        cur->start = start;
        cur->end = end;
        
        // If we reach min_len, stop.
        if(cur->depth == min_len) return;
    }
}

// Suffix link simulation using ISA/LCP heuristic.
bool alder_ssa_khan09_suffixlink(const int64_t *SA,
                                 const int64_t *ISA,
                                 const int64_t *LCP,
                                 const int64_t K,
                                 const int64_t logN,
                                 const int64_t NKm1,
                                 alder_interval_t *m)
{
//    int64_t K = o->K;
//    int64_t *ISA = o->ISA;
//    int64_t *SA = o->SA;
    
    m->depth -= K;
    if( m->depth <= 0) return false;
    m->start = ISA[SA[m->start] / K + 1];
    m->end = ISA[SA[m->end] / K + 1];
    return alder_ssa_khan09_expand_link(LCP, logN, NKm1, m);
}

bool alder_ssa_khan09_expand_link(const int64_t *LCP,
                                  const int64_t logN,
                                  const int64_t NKm1,
                                  alder_interval_t *link)
{
//    int64_t logN = o->logN;
//    int64_t *LCP = o->LCP;
//    int64_t NKm1 = o->NKm1;
    
    int64_t thresh = 2 * link->depth * logN, exp = 0; // Threshold link expansion.
    int64_t start = link->start;
    int64_t end = link->end;
    while(LCP[start] >= link->depth) {
        exp++;
        if(exp >= thresh) return false;
        start--;
    }
    while(end < NKm1 && LCP[end+1] >= link->depth) {
        exp++;
        if(exp >= thresh) return false;
        end++;
    }
    link->start = start; link->end = end;
    return true;
}


// For a given offset in the prefix k, find all MEMs.
int alder_ssa_khan09_findMEM(const char *S,
                             const int64_t *SA,
                             const int64_t *ISA,
                             const int64_t *LCP,
                             const int64_t N,
                             const int64_t K,
                             const int64_t logN,
                             const int64_t NKm1,
                             int64_t k, const char *P,
                             gsl_vector_match *matches, int64_t min_len, bool print)
{
//    int64_t K = o->K;
//    int64_t N = o->N;
    int64_t P_length = strlen(P);
    
    if(k < 0 || k >= K)
    {
        GSL_ERROR("min_len must be greater than or equal to K", GSL_EINVAL);
    }
    // Offset all intervals at different start points.
    int64_t prefix = k;
    alder_interval_t mli = { .depth = 0, .start = 0, .end = N/K - 1};
    alder_interval_t xmi = { .depth = 0, .start = 0, .end = N/K - 1};
    
    // Right-most match used to terminate search.
    int64_t min_lenK = min_len - (K-1);
    
    // TO FIND MEMS THE ALGORITHM ADVANCES THROUGH EACH CHARACTER POSITION P
    // IN THE QUERY STRING AND ATTEMPTS TO FIND MEMS OF SUFFICIENT LENGTH THAT
    // BEGIN AT P IN THE QUERY STRING.
    while( prefix <= P_length - (K-k)) {
        alder_ssa_khan09_traverse(S, SA, P, prefix, &mli, min_lenK);    // Traverse until minimum length matched.
        if(mli.depth > xmi.depth) xmi = mli;
        if(mli.depth <= 1)
        {
            alder_ssa_khan09_interval_reset(&mli, N/K-1);
            alder_ssa_khan09_interval_reset(&xmi, N/K-1);
            prefix+=K;
            continue;
        }
        
        if(mli.depth >= min_lenK) {
            alder_ssa_khan09_traverse(S, SA, P, prefix, &xmi, P_length); // Traverse until mismatch.
            
            // Using LCP info to find MEM length.
            alder_ssa_khan09_collectMEMs(S, SA, LCP, N, K, P, prefix, &mli, &xmi, matches, min_len, print);
            
            // When using ISA/LCP trick, depth = depth - K. prefix += K.
            prefix+=K;
            bool isSuccess = alder_ssa_khan09_suffixlink(SA, ISA, LCP,
                                                         K, logN, NKm1,
                                                         &mli);
            if( isSuccess == false )
            {
                alder_ssa_khan09_interval_reset(&mli, N/K-1);
                alder_ssa_khan09_interval_reset(&xmi, N/K-1);
                continue;
            }
            alder_ssa_khan09_suffixlink(SA, ISA, LCP,
                                        K, logN, NKm1,
                                        &xmi);
        }
        else {
            // When using ISA/LCP trick, depth = depth - K. prefix += K.
            prefix+=K;
            bool isSuccess = alder_ssa_khan09_suffixlink(SA, ISA, LCP,
                                                         K, logN, NKm1,
                                                         &mli);
            if( isSuccess == false )
            {
                alder_ssa_khan09_interval_reset(&mli, N/K-1);
                alder_ssa_khan09_interval_reset(&xmi, N/K-1);
                continue;
            }
            xmi = mli;
        }
    }
    return GSL_SUCCESS;
}


// Use LCP information to locate right maximal matches. Test each for
// left maximality.
void alder_ssa_khan09_collectMEMs(const char *S,
                                  const int64_t *SA,
                                  const int64_t *LCP,
                                  const int64_t N,
                                  const int64_t K,
                                  const char *P, int64_t prefix,
                                  alder_interval_t *mli, alder_interval_t *xmi,
                                  gsl_vector_match *matches, int64_t min_len,
                                  bool print)
{
//    int64_t *LCP = o->LCP;
//    int64_t *SA = o->SA;
//    int64_t N = o->N;
//    int64_t K = o->K;
    
    // All of the suffixes in xmi's interval are right maximal.
    for(int64_t i = xmi->start; i <= xmi->end; i++)
    {
        alder_ssa_khan09_find_Lmaximal(S, K, P, prefix, SA[i], xmi->depth, matches, min_len, print);
    }
    
    if(mli->start == xmi->start && mli->end == xmi->end) return;
    
    while(xmi->depth >= mli->depth) {
        // Attempt to "unmatch" xmi using LCP information.
        if(xmi->end+1 < N/K) xmi->depth = MAX(LCP[xmi->start], LCP[xmi->end+1]);
        else xmi->depth = LCP[xmi->start];
        
        // If unmatched XMI is > matched depth from mli, then examine rmems.
        if(xmi->depth >= mli->depth) {
            // Scan RMEMs to the left, check their left maximality..
            while(LCP[xmi->start] >= xmi->depth) {
                xmi->start--;
                alder_ssa_khan09_find_Lmaximal(S, K, P, prefix, SA[xmi->start], xmi->depth, matches, min_len, print);
            }
            // Find RMEMs to the right, check their left maximality.
            while(xmi->end+1 < N/K && LCP[xmi->end+1] >= xmi->depth) {
                xmi->end++;
                alder_ssa_khan09_find_Lmaximal(S, K, P, prefix, SA[xmi->end], xmi->depth, matches, min_len, print);
            }
        }
    }
}


// Finds left maximal matches given a right maximal match at position i.
void alder_ssa_khan09_find_Lmaximal(const char *S, const int64_t K,
                                    const char *P,
                                    int64_t prefix, int64_t i, int64_t len,
                                    gsl_vector_match *matches, int64_t min_len,
                                    bool print)
{
//    int64_t K = o->K;
//    char *S = o->S;
    
    // Advance to the left up to K steps.
    for(long k = 0; k < K; k++) {
        // If we reach the end and the match is long enough, print.
        if(prefix == 0 || i == 0) {
            if(len >= min_len) {
                // Add the found match to matches.
                alder_match_t m = { .ref = i, .query = prefix, .len = len };
                gsl_vector_match_add_alder_match(matches, m);
            }
            return; // Reached mismatch, done.
        }
        else if(P[prefix-1] != S[i-1]){
            // If we reached a mismatch, print the match if it is long enough.
            if(len >= min_len) {
                alder_match_t m = { .ref = i, .query = prefix, .len = len };
                gsl_vector_match_add_alder_match(matches, m);
            }
            return; // Reached mismatch, done.
        }
        prefix--; i--; len++; // Continue matching.
    }
}

int alder_ssa_khan09_MEM(const char *S,
                         const int64_t *SA,
                         const int64_t *ISA,
                         const int64_t *LCP,
                         const int64_t N,
                         const int64_t K,
                         const int64_t logN,
                         const int64_t NKm1,
                         const char *P, gsl_vector_match *matches,
                         int64_t min_len, bool print, int num_threads)
{
    assert(num_threads == 1);
//    int64_t K = o->K;
    assert(min_len >= K);
    for(int64_t k = 0; k < K; k++)
    {
        alder_ssa_khan09_findMEM(S, SA, ISA, LCP, N, K, logN, NKm1,
                                 k, P, matches, min_len, print);
    }
    return GSL_SUCCESS;
}

void alder_ssa_khan09_interval_reset(alder_interval_t *o, int64_t e)
{
    o->start = 0; o->end = e; o->depth = 0;
}

int64_t alder_ssa_khan09_interval_size(alder_interval_t *o)
{
    return o->end - o->start + 1;
}

void alder_ssa_khan09_print_match(alder_match_t m)
{
    printf("  %llu %llu %llu\n", m.ref, m.query, m.len);
}
