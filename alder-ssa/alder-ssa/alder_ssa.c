/**
 * This file, alder_ssa.c, is part of alder-ssa.
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

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include "alder_logger.h"
#include "alder_fasta.h"
#include "alder_divsufsort.h"
#include "alder_ssa.h"

static int64_t sMaxK = 3; ///< This must be smaller than sSizeCapacity in "alder_fasta.rl"

#define BUCKETBEGINSIZE 256
#define MAX(a,b)  ((a) > (b) ? (a) : (b))

// Builds a suffix array using a fasta file with sequences.
alder_sparse_sa_t *alder_sparse_sa_alloc_file (const struct bstrList *ref,
                                               const int64_t K)
{
    assert(K == 2 || K== 3);
    assert(sizeof(size_t) == sizeof(int64_t));
    
    alder_sparse_sa_t *sparseSA = malloc(sizeof(alder_sparse_sa_t));
    if (sparseSA == NULL) {
        logc_logWarning(ERROR_LOGGER, "cannot a sparse suffix array.");
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Set sparseSA->fS, and initialize others K, N, logN, NKm1.
    ///////////////////////////////////////////////////////////////////////////
    int64_t tSeq = 0;
    int64_t tBase = 0;
    alder_fasta_list_length(ref, &tSeq, &tBase);
    sparseSA->fS = alder_fasta_list_alloc(ref, tSeq, tBase, K);
    if (sparseSA->fS == NULL) {
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    sparseSA->S = sparseSA->fS->data;
    int64_t S_length = sparseSA->fS->numberOfBase;
    logc_log(MAIN_LOGGER, LOG_FINE, "Number of files=%d", ref->qty);
    logc_log(MAIN_LOGGER, LOG_FINE, "Number of sequences=%lld", tSeq);
    logc_log(MAIN_LOGGER, LOG_FINE, "Number of bases=%lld", tBase);
    
    // Increase string length so divisible by K.
    // Don't forget to count $ termination character.
    int64_t appendK = S_length % K == 0 ? K : K - S_length % K;
    if (sparseSA->fS->sizeCapacity < S_length + appendK + 1) {
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed: increase sSizeCapacity"
                      "in alder_fasta.rl",
                      GSL_ENOMEM, NULL);
    }
    assert(appendK > 0);
    for(int64_t i = 0; i < appendK; i++) {
        sparseSA->fS->data[sparseSA->fS->numberOfBase + i] = '$';
    }
    sparseSA->fS->data[sparseSA->fS->numberOfBase + appendK] = '\0';
    sparseSA->fS->sizeOfDataWithDollar = sparseSA->fS->numberOfBase + appendK;
    sparseSA->K = K;
    sparseSA->N = sparseSA->fS->sizeOfDataWithDollar;
    int64_t N = sparseSA->N;
    sparseSA->logN = (int64_t)ceil(log2((double)(N/K)));
    sparseSA->NKm1 = N/K-1;
    assert(N%K==0);
    if (K == 0 || K == 1 || K > sMaxK) {
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Build SA
    ///////////////////////////////////////////////////////////////////////////
    assert(1 < K && K < 4);
    uint16_t bucketNr = 1;
    int64_t *intSA = malloc((N/K+1)*sizeof(int64_t));
    if (intSA == NULL) {
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    for(int64_t i = 0; i < N/K+1; i++) intSA[i] = i; // Init SA.
    uint8_t *t_new = malloc((N/K+1)*sizeof(uint8_t));
    if (t_new == NULL) {
        free(intSA);
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    int64_t *BucketBegin = malloc(BUCKETBEGINSIZE * sizeof(int64_t));
    if (BucketBegin == NULL) {
        free(t_new);
        free(intSA);
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    ////////////////////////////////////////////////
    // Radix sort.
    ////////////////////////////////////////////////
    // intSA: size of N/K+1 initialized 0 to N/K.
    // t_new: could be the result?
    // bucketNr: ???
    // BucketBegin: some temp we may not need this.
    // l: 0
    // r: N/K-1
    // h: 0
    // What are these l, r, h?
    alder_radixStep(sparseSA, t_new, intSA, &bucketNr, BucketBegin, 0, N/K-1, 0);
    assert(intSA[N/K] == N/K);
    t_new[N/K] = 0; // Terminate new integer string.
    free(BucketBegin);
    logc_log(MAIN_LOGGER, LOG_FINE, "bucketNr=%d", bucketNr);
    logc_log(MAIN_LOGGER, LOG_FINE, "N/K=%lld", N/K);
#ifdef MAIN_LOGGER
    for (size_t i = 0; i < N/K + 1; i++) {
        logc_log(MAIN_LOGGER, LOG_FINEST,
                 "t_new and intSA [%zd] %d\t%lld",
                 i, (int)t_new[i], intSA[i]);
    }
#endif
    
    ////////////////////////////////////////////////
    // Replace this with libdivsufsort64.
    ////////////////////////////////////////////////
    // t_new is the input
    // intSA is the suffix array of t_new.
    // size of intSA is N/K.
    // 0 .. bucketNr - 1
    // Makes suffix array p of x. x becomes inverse of p. p and x are both of size
    // n+1. Contents of x[0...n-1] are integers in the range l...k-1. Original
    // contents of x[n] is disregarded, the n-th symbol being regarded as
    // end-of-string smaller than all other symbols.
    //
    // void suffixsort(int *x, int *p, int n, int k, int l)
    // suffixsort(t_new, intSA_int, (int)(N/K), bucketNr_int, 0);
    //
    // t_new is uint8_t
    // intSA is int64_t
    // N/K is the size of the arrays.
    // N/K+1 makes more sense; 0 at N/K position is used as the terminator.
    // because SA takes values from 1-st position not from 0-th position.
    // and the original source mentioned that the last character is
    // the terminator. suffixsort seems to mention x[n] or the last
    // character is disregarded, or ithe n-th symbol being regarded as
    // end-of-string smaller than all other symbols.
    // N/K was the original input;
    saint_t status = divsufsort64(t_new, intSA, N/K+1);
    if (status != 0) {
        fprintf(stderr, "error: divsufsort64\n");
    }
#ifdef MAIN_LOGGER
    logc_log(MAIN_LOGGER, LOG_FINE, "divsufsort64 is called");
    for (size_t i = 0; i < N/K + 1; i++) {
        logc_log(MAIN_LOGGER, LOG_FINEST,
                 "intSA [%zd] %lld",
                 i, intSA[i]);
    }
#endif
    free(t_new);
    
    ////////////////////////////////////////////////
    // Translate suffix array: set sparseSA->SA.
    ////////////////////////////////////////////////
    sparseSA->SA = malloc((N/K)*sizeof(int64_t));
    if (sparseSA->SA == NULL) {
        free(intSA);
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    for (int64_t i = 0; i < N/K; i++) {
        sparseSA->SA[i] = intSA[i+1] * K;
    }
    free(intSA);
#ifdef MAIN_LOGGER
    logc_log(MAIN_LOGGER, LOG_FINEST,
             "SA is computed using intSA (deleted)");
    for (size_t i = 0; i < N/K; i++) {
        logc_log(MAIN_LOGGER, LOG_FINEST,
                 "SA [%zd] %lld",
                 i, sparseSA->SA[i]);
    }
#endif
    
    ///////////////////////////////////////////////////////////////////////////
    // Build ISA using sparse SA.
    ///////////////////////////////////////////////////////////////////////////
    sparseSA->ISA = malloc((N/K)*sizeof(int64_t));
    if (sparseSA->ISA == NULL) {
        free(sparseSA->SA);
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    for (int64_t i = 0; i < N/K; i++) {
        sparseSA->ISA[sparseSA->SA[i]/K] = i;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Initialize LCP : SA + ISA -> LCP
    ///////////////////////////////////////////////////////////////////////////
    sparseSA->LCP = malloc((N/K)*sizeof(int64_t));
    if (sparseSA->LCP == NULL) {
        free(sparseSA->ISA);
        free(sparseSA->SA);
        alder_fasta_list_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    alder_computeLCP(sparseSA);
    
#ifdef MAIN_LOGGER
    logc_log(MAIN_LOGGER, LOG_FINE, "ISA is computed using SA.");
    for (size_t i = 0; i < N/K; i++) {
        logc_log(MAIN_LOGGER, LOG_FINEST,
                 "ISA [%zd] %lld",
                 i, sparseSA->ISA[i]);
    }
    for (size_t i = 0; i < N/K; i++) {
        logc_log(MAIN_LOGGER, LOG_FINEST,
                 "LCP [%zd] %lld",
                 i, sparseSA->LCP[i]);
    }
#endif
    
    return sparseSA;
}




void alder_sparse_sa_free (alder_sparse_sa_t *o)
{
    alder_fasta_list_free(o->fS);
//    free(o->S);
    free(o->SA);
    free(o->ISA);
    free(o->LCP);
    free(o);
}

// Uses the algorithm of Kasai et al 2001 which was described in
// Manzini 2004 to compute the LCP array. Modified to handle sparse
// suffix arrays and inverse sparse suffix arrays.
void alder_computeLCP(alder_sparse_sa_t *o)
{
    int64_t K = o->K;
    int64_t N = o->N;
    char *S = o->S;
    int64_t *SA = o->SA;
    int64_t *ISA = o->ISA;
    int64_t *LCP = o->LCP;
    
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


int alder_radixStep(alder_sparse_sa_t *o, uint8_t *t_new, int64_t *SA,
                    uint16_t *bucketNr_, int64_t *BucketBegin,
                    int64_t l, int64_t r, int64_t h)
{
    // Note: SA is a local variable not o->SA.
    char *S          = o->S;
    int64_t K        = o->K;
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
                alder_radixStep(o, t_new, SA, &bucketNr, BucketBegin, beg, end, h+1); // recursive refinement
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
int64_t alder_bsearch_left(alder_sparse_sa_t *o, char c, int64_t i, int64_t s, int64_t e)
{
    char *S = o->S;
    int64_t *SA = o->SA;
    
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
int64_t alder_bsearch_right(alder_sparse_sa_t *o, char c, int64_t i, int64_t s, int64_t e)
{
    char *S = o->S;
    int64_t *SA = o->SA;

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
bool alder_top_down(alder_sparse_sa_t *o, char c, int64_t i, int64_t *start, int64_t *end)
{
    int64_t *SA = o->SA;
    char *S = o->S;
    
    if(c < S[SA[*start]+i]) return false;
    if(c > S[SA[*end]+i]) return false;
    int64_t l = alder_bsearch_left(o, c, i, *start, *end);
    int64_t l2 = alder_bsearch_right(o, c, i, *start, *end);
    *start = l;
    *end = l2;
    return l <= l2;
}

// Top down traversal of the suffix array to match a pattern.  NOTE:
// NO childtab as in the enhanced suffix array (ESA).
bool alder_search(alder_sparse_sa_t *o, const char *P, int64_t start, int64_t end)
{
    int64_t N = o->N;
    int64_t P_length = strlen(P);
    
    start = 0; end = N - 1;
    int64_t i = 0;
    while(i < P_length) {
        if(alder_top_down(o, P[i], i, &start, &end) == false) {
            return false;
        }
        i++;
    }
    return true;
}


// Traverse pattern P starting from a given prefix and interval
// until mismatch or min_len characters reached.
// Return values: cur
void alder_traverse(alder_sparse_sa_t *o, const char *P,
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
        bool isSuccess = alder_top_down_faster(o, P[prefix+cur->depth], cur->depth, &start, &end);
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


// Given SA interval apply binary search to match character c at
// position i in the search string. Adapted from the C++ source code
// for the wordSA implementation from the following paper: Ferragina
// and Fischer. Suffix Arrays on Words. CPM 2007.
bool alder_top_down_faster(alder_sparse_sa_t *o, char c, int64_t i,
                           int64_t *start_, int64_t *end_)
{
    int64_t start = *start_;
    int64_t end = *end_;
    char *S = o->S;
    int64_t *SA = o->SA;
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


// Suffix link simulation using ISA/LCP heuristic.
bool alder_suffixlink(alder_sparse_sa_t *o, alder_interval_t *m)
{
    int64_t K = o->K;
    int64_t *ISA = o->ISA;
    int64_t *SA = o->SA;
    
    m->depth -= K;
    if( m->depth <= 0) return false;
    m->start = ISA[SA[m->start] / K + 1];
    m->end = ISA[SA[m->end] / K + 1];
    return alder_expand_link(o, m);
}

bool alder_expand_link(alder_sparse_sa_t *o, alder_interval_t *link) {
    int64_t logN = o->logN;
    int64_t *LCP = o->LCP;
    int64_t NKm1 = o->NKm1;
    
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
int alder_findMEM(alder_sparse_sa_t *o, int64_t k, const char *P,
                  gsl_vector_match *matches, int64_t min_len, bool print)
{
    int64_t K = o->K;
    int64_t N = o->N;
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
        alder_traverse(o, P, prefix, &mli, min_lenK);    // Traverse until minimum length matched.
        if(mli.depth > xmi.depth) xmi = mli;
        if(mli.depth <= 1)
        {
            alder_interval_reset(&mli, N/K-1);
            alder_interval_reset(&xmi, N/K-1);
            prefix+=K;
            continue;
        }
        
        if(mli.depth >= min_lenK) {
            alder_traverse(o, P, prefix, &xmi, P_length); // Traverse until mismatch.
            
            // Using LCP info to find MEM length.
            alder_collectMEMs(o, P, prefix, &mli, &xmi, matches, min_len, print);
            
            // When using ISA/LCP trick, depth = depth - K. prefix += K.
            prefix+=K;
            bool isSuccess = alder_suffixlink(o, &mli);
            if( isSuccess == false )
            {
                alder_interval_reset(&mli, N/K-1);
                alder_interval_reset(&xmi, N/K-1);
                continue;
            }
            alder_suffixlink(o, &xmi);
        }
        else {
            // When using ISA/LCP trick, depth = depth - K. prefix += K.
            prefix+=K;
            bool isSuccess = alder_suffixlink(o, &mli);
            if( isSuccess == false )
            {
                alder_interval_reset(&mli, N/K-1);
                alder_interval_reset(&xmi, N/K-1);
                continue;
            }
            xmi = mli;
        }
    }
    return GSL_SUCCESS;
}


// Use LCP information to locate right maximal matches. Test each for
// left maximality.
void alder_collectMEMs(alder_sparse_sa_t *o,
                       const char *P, int64_t prefix,
                       alder_interval_t *mli, alder_interval_t *xmi,
                       gsl_vector_match *matches, int64_t min_len,
                       bool print)
{
    int64_t *LCP = o->LCP;
    int64_t *SA = o->SA;
    int64_t N = o->N;
    int64_t K = o->K;
    
    // All of the suffixes in xmi's interval are right maximal.
    for(int64_t i = xmi->start; i <= xmi->end; i++)
    {
        alder_find_Lmaximal(o, P, prefix, SA[i], xmi->depth, matches, min_len, print);
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
                alder_find_Lmaximal(o, P, prefix, SA[xmi->start], xmi->depth, matches, min_len, print);
            }
            // Find RMEMs to the right, check their left maximality.
            while(xmi->end+1 < N/K && LCP[xmi->end+1] >= xmi->depth) {
                xmi->end++;
                alder_find_Lmaximal(o, P, prefix, SA[xmi->end], xmi->depth, matches, min_len, print);
            }
        }
    }
}


// Finds left maximal matches given a right maximal match at position i.
void alder_find_Lmaximal(alder_sparse_sa_t *o, const char *P,
                         int64_t prefix, int64_t i, int64_t len,
                         gsl_vector_match *matches, int64_t min_len,
                         bool print)
{
    int64_t K = o->K;
    char *S = o->S;
    
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

int alder_MEM(alder_sparse_sa_t *o, const char *P, gsl_vector_match *matches,
              int64_t min_len, bool print, int num_threads)
{
    assert(num_threads == 1);
    int64_t K = o->K;
    assert(min_len >= K);
    for(int64_t k = 0; k < K; k++)
    {
        alder_findMEM(o, k, P, matches, min_len, print);
    }
    return GSL_SUCCESS;
}

void alder_interval_reset(alder_interval_t *o, int64_t e)
{
    o->start = 0; o->end = e; o->depth = 0;
}

int64_t alder_interval_size(alder_interval_t *o)
{
    return o->end - o->start + 1;
}

void alder_print_match(alder_match_t m)
{
    printf("  %llu %llu %llu\n", m.ref, m.query, m.len);
}