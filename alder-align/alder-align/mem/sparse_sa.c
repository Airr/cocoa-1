//
//  sparse_sa.c
//  mem
//
//  Created by Sang Chul Choi on 8/13/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
//#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <gsl/gsl_errno.h>
#include <assert.h>
#include <nglogc/log.h>
#include "alder_logger.h"
#include "sparse_sa.h"
#include "alder-align/fasta/alder_fasta.h"
#include "/Users/goshng/usr/local/include/divsufsort64.h"

static int64_t sMaxK = 3; ///< This must be smaller than sSizeCapacity in "alder_fasta.rl"

#define BUCKETBEGINSIZE 256
#define MAX(a,b)  ((a) > (b) ? (a) : (b))

// LS suffix sorter (integer alphabet).
// The function is defined in qsufsort.c.
void suffixsort(int *x, int *p, int n, int k, int l);

alder_sparse_sa_t *alder_sparse_sa_alloc(const char *ref, int64_t K)
{
    assert(sizeof(size_t) == sizeof(uint64_t));
    
    alder_sparse_sa_t *sparseSA = malloc(sizeof(alder_sparse_sa_t));
    if (sparseSA == NULL)
    {
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    sparseSA->K = K;
    sparseSA->S = strdup(ref);
    if (sparseSA->S == NULL)
    {
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    size_t S_length = strlen(sparseSA->S);
    
    // Increase string length so divisible by K.
    // Don't forget to count $ termination character.
    if(S_length % K != 0) {
        uint64_t appendK = K - S_length % K ;
        char *tmp = realloc(sparseSA->S, (S_length + appendK + 1)*sizeof(char));
        if (tmp == NULL)
        {
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        sparseSA->S = tmp;
        for(uint64_t i = 0; i < appendK; i++)
        {
            strcat(sparseSA->S, "$");
        }
    }
    // Make sure last K-sampled characeter is this special character as well!!
    S_length = strlen(sparseSA->S);
    char *tmp = realloc(sparseSA->S, (S_length + K + 1)*sizeof(char));
    if (tmp == NULL)
    {
        free(sparseSA->S);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    sparseSA->S = tmp;
    // Append "special" end character. Note: It must be lexicographically less.
    for(long i = 0; i < K; i++)
    {
        strcat(sparseSA->S, "$");
    }
    uint64_t N = strlen(sparseSA->S);
    sparseSA->N = N;
    
    if(K > 1) {
        uint64_t bucketNr = 1;
//        long bucketNr = 1;
        uint64_t *intSA = malloc((N/K+1)*sizeof(uint64_t));
//        int *intSA = new int[N/K+1];
        if (intSA == NULL)
        {
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for(uint64_t i = 0; i < N/K; i++) intSA[i] = i; // Init SA.
        uint64_t *t_new = malloc((sparseSA->N/K+1)*sizeof(uint64_t));
//        int* t_new = new int[N/K+1];
        if (t_new == NULL)
        {
            free(intSA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        uint64_t *BucketBegin = malloc(BUCKETBEGINSIZE * sizeof(uint64_t));
//        long* BucketBegin = new long[256]; // array to save current bucket beginnings
        if (BucketBegin == NULL)
        {
            free(t_new);
            free(intSA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        
        alder_radixStep(sparseSA, t_new, intSA, &bucketNr, BucketBegin, 0, N/K-1, 0); // start radix sort
        
        t_new[N/K] = 0; // Terminate new integer string.
        free(BucketBegin);
//        delete[] BucketBegin;
        
        // Suffix sort integer text.
        fprintf(stderr, "# suffixsort()\n");
        int bucketNr_int = (int)bucketNr;
        int *t_new_int = malloc((sparseSA->N/K+1)*sizeof(int));
        if (t_new_int == NULL)
        {
            free(t_new);
            free(intSA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for (size_t i = 0; i < N/K+1; i++) {
            assert(t_new[i] <= INT_MAX);
            t_new_int[i] = (int)t_new[i];
        }
        int *intSA_int = malloc((sparseSA->N/K+1)*sizeof(int));
        if (intSA_int == NULL)
        {
            free(t_new_int);
            free(t_new);
            free(intSA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for (size_t i = 0; i < N/K+1; i++) {
            assert(intSA[i] <= INT_MAX);
            intSA_int[i] = (int)intSA[i];
        }
        suffixsort(t_new_int, intSA_int, (int)(N/K), bucketNr_int, 0);
        fprintf(stderr, "# DONE suffixsort()\n");
        for (size_t i = 0; i < sparseSA->N/K+1; i++) {
            intSA[i] = (uint64_t)intSA_int[i];
        }
        free(intSA_int);
        free(t_new_int);
        
        free(t_new);
//        delete[] t_new;
        
        // Translate suffix array.
        sparseSA->SA = malloc((N/K)*sizeof(uint64_t));
//        SA.resize(N/K);
        if (sparseSA->SA == NULL) {
            free(intSA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for (size_t i=0; i<sparseSA->N/K; i++)
        {
            sparseSA->SA[i] = intSA[i+1] * K;
        }
        free(intSA);
//        delete[] intSA;
        
        // Build ISA using sparse SA.
        sparseSA->ISA = malloc((sparseSA->N/K)*sizeof(uint64_t));
//        ISA.resize(N/K);
        if (sparseSA->ISA == NULL) {
            free(sparseSA->SA);
            free(sparseSA->S);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for (size_t i=0; i<N/K; i++)
        {
            sparseSA->ISA[sparseSA->SA[i]/K] = i;
        }
//        for(long i = 0; i < N/K; i++) { ISA[SA[i]/K] = i; }
    }
    // NOT SURE WHETHER WE NEED K = 1 CASE.
    assert(K > 1);
    
    fprintf(stderr, "N=%llu\n", sparseSA->N);
    
    // Adjust to "sampled" size.
    sparseSA->logN = (uint64_t)ceil(log(N/K) / log(2.0));
//    logN = (long)ceil(log(N/K) / log(2.0));
    
    sparseSA->LCP = malloc((N/K)*sizeof(uint64_t));
//    LCP.resize(N/K);
    if (sparseSA->LCP == NULL) {
        free(sparseSA->ISA);
        free(sparseSA->SA);
        free(sparseSA->S);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    fprintf(stderr, "N/K=%llu\n", N/K);
//    cerr << "N/K=" << N/K << endl;
    // Use algorithm by Kasai et al to construct LCP array.
    alder_computeLCP(sparseSA);  // SA + ISA -> LCP
//    alder_initLCP(sparseSA);
    
    sparseSA->NKm1 = N/K-1;
    return sparseSA;
}

// Builds a suffix tree using a fasta file with sequences.
alder_sparse_sa_t *alder_sparse_sa_alloc_file (const char *ref, int64_t K)
{
    assert(sizeof(size_t) == sizeof(int64_t));
    
    alder_sparse_sa_t *sparseSA = malloc(sizeof(alder_sparse_sa_t));
    if (sparseSA == NULL) {
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    // Initialize member fS
    sparseSA->fS = alder_fasta_alloc(ref);
    if (sparseSA->fS == NULL) {
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    sparseSA->S = sparseSA->fS->data;
    int64_t S_length = sparseSA->fS->numberOfBase;
    
    // Increase string length so divisible by K.
    // Don't forget to count $ termination character.
    uint64_t appendK = S_length % K == 0 ? K : K - S_length % K;
    if (sparseSA->fS->sizeCapacity < S_length + appendK + 1) {
        alder_fasta_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed: increase sSizeCapacity"
                      "in alder_fasta.rl",
                      GSL_ENOMEM, NULL);
    }
    assert(appendK > 0);
    for(uint64_t i = 0; i < appendK; i++) {
        sparseSA->fS->data[sparseSA->fS->numberOfBase + i] = '$';
    }
    sparseSA->fS->data[sparseSA->fS->numberOfBase + appendK] = '\0';
    sparseSA->fS->sizeOfDataWithDollar = sparseSA->fS->numberOfBase + appendK;
    
    // Initialize K, N, logN, NKm1.
    sparseSA->K = K;
    sparseSA->N = sparseSA->fS->sizeOfDataWithDollar;
    uint64_t N = sparseSA->N;
    sparseSA->logN = (uint64_t)ceil(log2((double)(N/K)));
    sparseSA->NKm1 = N/K-1;
    assert(N%K==0);
    
    assert(K > 1);
    if (K == 0 || K == 1 || K > sMaxK) {
        alder_fasta_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    // Initialize SA, and ISA
    if(K > 1) {
        uint16_t bucketNr = 1;
        int64_t *intSA = malloc((N/K+1)*sizeof(int64_t));
        if (intSA == NULL) {
            alder_fasta_free(sparseSA->fS);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for(uint64_t i = 0; i <= N/K; i++) intSA[i] = i; // Init SA.
        uint8_t *t_new = malloc((N/K+1)*sizeof(uint8_t));
        if (t_new == NULL) {
            free(intSA);
            alder_fasta_free(sparseSA->fS);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        int64_t *BucketBegin = malloc(BUCKETBEGINSIZE * sizeof(int64_t));
        if (BucketBegin == NULL) {
            free(t_new);
            free(intSA);
            alder_fasta_free(sparseSA->fS);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        
        ////////////////////////////////////////////////
        // Replace this with libdivsufsort.
        ////////////////////////////////////////////////
        // intSA: size of N/K initialized 0 to N/K-1.
        // t_new: could be the result?
        // bucketNr: ???
        // BucketBegin: some temp we may not need this.
        // l: 0
        // r: N/K-1
        // h: 0
        // What are these l, r, h?
        alder_radixStep(sparseSA, t_new, intSA, &bucketNr, BucketBegin, 0, N/K-1, 0); // start radix sort
        ////////////////////////////////////////////////
        // Replace this with libdivsufsort.
        ////////////////////////////////////////////////
        assert(intSA[N/K] == N/K);
        t_new[N/K] = 0; // Terminate new integer string.
        free(BucketBegin);
        
        logc_log(MAIN_LOGGER, LOG_FINEST, "bucketNr=%d", bucketNr);
        logc_log(MAIN_LOGGER, LOG_FINEST, "N/K=%lld", N/K);
#ifdef MAIN_LOGGER
        for (size_t i = 0; i < N/K + 1; i++) {
            logc_log(MAIN_LOGGER, LOG_FINEST,
                     "t_new and intSA [%zd] %d\t%lld",
                     i, (int)t_new[i], intSA[i]);
        }
#endif
        ////////////////////////////////////////////////
        // Replace this with libdivsufsort.
        ////////////////////////////////////////////////
        // t_new is the input
        // intSA is the suffix array of t_new.
        // size of intSA is N/K.
        // 0 .. bucketNr - 1
/* Makes suffix array p of x. x becomes inverse of p. p and x are both of size
   n+1. Contents of x[0...n-1] are integers in the range l...k-1. Original
   contents of x[n] is disregarded, the n-th symbol being regarded as
   end-of-string smaller than all other symbols.*/
//void suffixsort(int *x, int *p, int n, int k, int l)
        //        suffixsort(t_new, intSA_int, (int)(N/K), bucketNr_int, 0);
        
        // t_new is uint8_t
        // intSA is int64_t
        // N/K is the size of the arrays.
        // N/K+1 makes more sense; 0 at N/K position is used as the terminator.
        //   because SA takes values from 1-st position not from 0-th position.
        //   and the original source mentioned that the last character is
        //   the terminator. suffixsort seems to mention x[n] or the last
        //   character is disregarded, or ithe n-th symbol being regarded as
        //   end-of-string smaller than all other symbols.
        // 

        // N/K is the original input;
        saint_t status = divsufsort64(t_new, intSA, N/K+1);
        if (status != 0) {
            fprintf(stderr, "error: divsufsort64\n");
        }
#ifdef MAIN_LOGGER
        logc_log(MAIN_LOGGER, LOG_FINEST, "divsufsort64 is called");
        for (size_t i = 0; i < N/K + 1; i++) {
            logc_log(MAIN_LOGGER, LOG_FINEST,
                     "intSA [%zd] %lld",
                     i, intSA[i]);
        }
#endif
        ////////////////////////////////////////////////
        // Replace this with libdivsufsort.
        ////////////////////////////////////////////////
        
        ////////////////////////////////////////////////
        // Translate suffix array.
        sparseSA->SA = malloc((N/K)*sizeof(int64_t));
        if (sparseSA->SA == NULL) {
            alder_fasta_free(sparseSA->fS);
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
        
        // Build ISA using sparse SA.
        sparseSA->ISA = malloc((N/K)*sizeof(uint64_t));
        if (sparseSA->ISA == NULL) {
            free(sparseSA->SA);
            alder_fasta_free(sparseSA->fS);
            free(sparseSA);
            GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
        }
        for (int64_t i = 0; i < N/K; i++) {
            sparseSA->ISA[sparseSA->SA[i]/K] = i;
        }
    }

    // Initialize LCP.
    sparseSA->LCP = malloc((N/K)*sizeof(int64_t));
    if (sparseSA->LCP == NULL) {
        free(sparseSA->ISA);
        free(sparseSA->SA);
        alder_fasta_free(sparseSA->fS);
        free(sparseSA);
        GSL_ERROR_VAL("sparse sa alloc failed", GSL_ENOMEM, NULL);
    }
    
    ////////////////////////////////////////////////
    // Replace this with libdivsufsort.
    ////////////////////////////////////////////////
    alder_computeLCP(sparseSA);  // SA + ISA -> LCP
    ////////////////////////////////////////////////
    // Replace this with libdivsufsort.
    ////////////////////////////////////////////////
    
#ifdef MAIN_LOGGER
    logc_log(MAIN_LOGGER, LOG_FINEST, "ISA is computed using SA.");
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

void query_thread(alder_sparse_sa_t *o, const char *P)
{
    gsl_vector_match *matches;
    uint64_t min_len = 3;
    bool print = true;
    int num_threads = 1;
    
    alder_MEM(o, P, matches, min_len, print, num_threads);
    
    if(!print)
    {
//        alder_print_match(o, matches, false);
    }
}


void alder_sparse_sa_free (alder_sparse_sa_t *o)
{
    alder_fasta_free(o->fS);
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


// Implements a variant of American flag sort (McIlroy radix sort).
// Recurse until big-K size prefixes are sorted. Adapted from the C++
// source code for the wordSA implementation from the following paper:
// Ferragina and Fischer. Suffix Arrays on Words. CPM 2007.
int alder_radixStep2(alder_sparse_sa_t *o, uint8_t *t_new, int64_t *SA,
                    long *bucketNr_, long *BucketBegin, long l, long r, long h)
{
    // Note: SA is a local variable not o->SA.
    char *S           = o->S;
    uint64_t K        = o->K;
    long bucketNr = *bucketNr_;
    assert(sizeof(size_t) == sizeof(uint64_t));
    
    if(h >= K) {
        GSL_ERROR("radixStep failed", GSL_EINVAL);
    }
    // first pass: count
    long *Sigma = malloc(BUCKETBEGINSIZE * sizeof(long));
    if (Sigma == NULL) {
        GSL_ERROR("radixStep failed", GSL_ENOMEM);
    }
    memset(Sigma, 0, BUCKETBEGINSIZE * sizeof(long));
    for (long i = l; i <= r; i++) Sigma[ S[ SA[i]*K + h ] ]++; // count characters
    BucketBegin[0] = l;
    for (long i = 1; i < BUCKETBEGINSIZE; i++) {
        BucketBegin[i] = Sigma[i-1] + BucketBegin[i-1];
    } // accumulate
    
    // second pass: move (this variant does *not* need an additional array!)
    unsigned char currentKey = 0;    // character of current bucket
    long end = l-1+Sigma[currentKey]; // end of current bucket
    long pos = l;                     // 'pos' is current position in bucket
    while (1) {
        if (pos > end) { // Reached the end of the bucket.
            if (currentKey == BUCKETBEGINSIZE - 1) break; // Last character?
            currentKey++; // Advance to next characer.
            pos = BucketBegin[currentKey]; // Next bucket start.
            end += Sigma[currentKey]; // Next bucket end.
        }
        else {
            // American flag sort of McIlroy et al. 1993. BucketBegin keeps
            // track of current position where to add to bucket set.
            uint64_t tmp = SA[ BucketBegin[ S[ SA[pos]*K + h ] ] ];
            SA[ BucketBegin[ S[ SA[pos]*K + h] ]++ ] = SA[pos];  // Move bucket beginning to the right, and replace
            SA[ pos ] = tmp; // Save value at bucket beginning.
            if (S[ SA[pos]*K + h ] == currentKey) pos++; // Advance to next position if the right character.
        }
    }
    // recursively refine buckets and calculate new text:
    long beg = l; end = l-1;
    for (long i = 1; i < BUCKETBEGINSIZE; i++) { // step through Sigma to find bucket borders
        end += Sigma[i];
        if (beg <= end) {
            if(h == K-1) {
                for (long j = beg; j <= end; j++) {
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
uint64_t alder_bsearch_left(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t s, uint64_t e)
{
    char *S = o->S;
    uint64_t *SA = o->SA;
    
    if(c == S[SA[s]+i]) return s;
    uint64_t l = s;
    uint64_t r = e;
    while (r > l + 1) {
//    while (r - l > 1) {
        long m = (l+r) / 2;
        if (c <= S[SA[m] + i]) r = m;
        else l = m;
    }
    return r;
}

// Binary search for right boundry of interval.
uint64_t alder_bsearch_right(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t s, uint64_t e)
{
    char *S = o->S;
    uint64_t *SA = o->SA;

    if(c == S[SA[e]+i]) return e;
    uint64_t l = s;
    uint64_t r = e;
    while (r > l + 1) {
//    while (r - l > 1) {
        long m = (l+r) / 2;
        if (c < S[SA[m] + i]) r = m;
        else l = m;
    }
    return l;
}


// Simple top down traversal of a suffix array.
bool alder_top_down(alder_sparse_sa_t *o, char c, uint64_t i, uint64_t *start, uint64_t *end)
{
    uint64_t *SA = o->SA;
    char *S = o->S;
    
    if(c < S[SA[*start]+i]) return false;
    if(c > S[SA[*end]+i]) return false;
    uint64_t l = alder_bsearch_left(o, c, i, *start, *end);
    uint64_t l2 = alder_bsearch_right(o, c, i, *start, *end);
    *start = l;
    *end = l2;
    return l <= l2;
}

// Top down traversal of the suffix array to match a pattern.  NOTE:
// NO childtab as in the enhanced suffix array (ESA).
bool alder_search(alder_sparse_sa_t *o, const char *P, uint64_t start, uint64_t end)
{
    uint64_t N = o->N;
    uint64_t P_length = strlen(P);
    
    start = 0; end = N - 1;
    uint64_t i = 0;
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
                    uint64_t prefix, alder_interval_t *cur,
                    uint64_t min_len)
{
    uint64_t P_length = strlen(P);
    
    if(cur->depth >= min_len) return;
    
    while(prefix+cur->depth < P_length)
    {
        uint64_t start = cur->start;
        uint64_t end = cur->end;
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
bool alder_top_down_faster(alder_sparse_sa_t *o, char c, uint64_t i,
                           uint64_t *start_, uint64_t *end_)
{
    uint64_t start = *start_;
    uint64_t end = *end_;
    char *S = o->S;
    uint64_t *SA = o->SA;
    uint64_t l;
    uint64_t r;
    uint64_t m;
    uint64_t r2=end;
    uint64_t l2=start;
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
    uint64_t K = o->K;
    uint64_t *ISA = o->ISA;
    uint64_t *SA = o->SA;
    
    m->depth -= K;
    if( m->depth <= 0) return false;
    m->start = ISA[SA[m->start] / K + 1];
    m->end = ISA[SA[m->end] / K + 1];
    return alder_expand_link(o, m);
}

bool alder_expand_link(alder_sparse_sa_t *o, alder_interval_t *link) {
    uint64_t logN = o->logN;
    uint64_t *LCP = o->LCP;
    uint64_t NKm1 = o->NKm1;
    
    uint64_t thresh = 2 * link->depth * logN, exp = 0; // Threshold link expansion.
    uint64_t start = link->start;
    uint64_t end = link->end;
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
int alder_findMEM(alder_sparse_sa_t *o, uint64_t k, const char *P,
                  gsl_vector_match *matches, uint64_t min_len, bool print)
{
    uint64_t K = o->K;
    uint64_t N = o->N;
    uint64_t P_length = strlen(P);
    
    if(k >= K)
//    if(k < 0 || k >= K)
    {
        GSL_ERROR("min_len must be greater than or equal to K", GSL_EINVAL);
    }
    // Offset all intervals at different start points.
    uint64_t prefix = k;
    alder_interval_t mli = { .depth = 0, .start = 0, .end = N/K - 1};
    alder_interval_t xmi = { .depth = 0, .start = 0, .end = N/K - 1};
//    interval_t mli(0,N/K-1,0); // min length interval
//    interval_t xmi(0,N/K-1,0); // max match interval
    
    // Right-most match used to terminate search.
    uint64_t min_lenK = min_len - (K-1);
    
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
//        if(mli.depth <= 1) { mli.reset(N/K-1); xmi.reset(N/K-1); prefix+=K; continue; }
        
        if(mli.depth >= min_lenK) {
            alder_traverse(o, P, prefix, &xmi, P_length); // Traverse until mismatch.
            alder_collectMEMs(o, P, prefix, &mli, &xmi, matches, min_len, print); // Using LCP info to find MEM length.
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
//            if( suffixlink(mli) == false ) { mli.reset(N/K-1); xmi.reset(N/K-1); continue; }
//            suffixlink(xmi);
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
//            if( suffixlink(mli) == false ) { mli.reset(N/K-1); xmi.reset(N/K-1); continue; }
            xmi = mli;
        }
    }
    if(print)
    {
//        print_match(match_t(), matches);   // Clear buffered matches.
    }
    return GSL_SUCCESS;
}


// Use LCP information to locate right maximal matches. Test each for
// left maximality.
void alder_collectMEMs(alder_sparse_sa_t *o,
                       const char *P, uint64_t prefix,
                       alder_interval_t *mli, alder_interval_t *xmi,
                       gsl_vector_match *matches, uint64_t min_len,
                       bool print)
{
    int64_t *LCP = o->LCP;
    int64_t *SA = o->SA;
    int64_t N = o->N;
    int64_t K = o->K;
    
    // All of the suffixes in xmi's interval are right maximal.
    for(uint64_t i = xmi->start; i <= xmi->end; i++)
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
                         uint64_t prefix, uint64_t i, uint64_t len,
                         gsl_vector_match *matches, uint64_t min_len,
                         bool print)
{
    int64_t K = o->K;
    char *S = o->S;
    
    // Advance to the left up to K steps.
    for(long k = 0; k < K; k++) {
        // If we reach the end and the match is long enough, print.
        if(prefix == 0 || i == 0) {
            if(len >= min_len) {
                if(print == true)
                {
                    alder_match_t m = { .ref = i, .query = prefix, .len = len };
                    // Add the found match to matches.
                    gsl_vector_match_add_alder_match(matches, m);
//                    alder_print_match(m);
                }
//                else matches.push_back(match_t(i, prefix, len));
            }
            return; // Reached mismatch, done.
        }
        else if(P[prefix-1] != S[i-1]){
            // If we reached a mismatch, print the match if it is long enough.
            if(len >= min_len) {
                if(print == true)
                {
                    alder_match_t m = { .ref = i, .query = prefix, .len = len };
                    gsl_vector_match_add_alder_match(matches, m);
//                    alder_print_match(m);
                }
//                else matches.push_back(match_t(i, prefix, len));
            }
            return; // Reached mismatch, done.
        }
        prefix--; i--; len++; // Continue matching.
    }
}

int alder_MEM(alder_sparse_sa_t *o, const char *P, gsl_vector_match *matches,
              uint64_t min_len, bool print, int num_threads)
{
    assert(num_threads == 1);
    uint64_t K = o->K;
    
    if(min_len < K)
    {
        GSL_ERROR("min_len must be greater than or equal to K", GSL_EINVAL);
    }
    if(num_threads == 1) {
        for(uint64_t k = 0; k < K; k++)
        {
            alder_findMEM(o, k, P, matches, min_len, print);
        }
    }
    return GSL_SUCCESS;
}

void alder_interval_reset(alder_interval_t *o, uint64_t e)
{
    o->start = 0; o->end = e; o->depth = 0;
}

uint64_t alder_interval_size(alder_interval_t *o)
{
    return o->end - o->start + 1;
}

void alder_print_match(alder_match_t m)
{
    printf("  %llu %llu %llu\n", m.ref, m.query, m.len);
}