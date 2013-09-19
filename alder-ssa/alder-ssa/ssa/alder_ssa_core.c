/**
 * This file, alder_ssa_core.c, is part of alder-denovo.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "alder_divsufsort.h"
#include "alder_ssa_core.h"
#include "alder_cmacro.h"
#include "alder_ssa_khan09.h"

static const int64_t sMaxK = 3;
static const size_t BUCKETBEGINSIZE = 256;

/* This creates a sparse suffix array.
 * S - a string of characters
 * nS - the size of S
 * M - the size of memory available for S (N < M)
 * K - the sparseness of the suffix array
 * It returns a sparse suffix array if successful, NULL otherwise.
 */
alder_ssa_core_t * alder_ssa_core_init_withS(char *S,
                                             const int64_t nS,
                                             const int64_t M,
                                             const int64_t K)
{
    int s = 0;
    int64_t N = 0;
    if (K == 0 || K == 1 || K > sMaxK) return NULL;
    s = alder_ssa_khan09_appendDollarSign(S, nS, M, K, &N);
    if (s != 0) return NULL;
    
    alder_ssa_core_t *sparseSA = malloc(sizeof(alder_ssa_core_t));
    int64_t *intSA = malloc((N/K+1)*sizeof(int64_t));
    uint8_t *t_new = malloc((N/K+1)*sizeof(uint8_t));
    int64_t *BucketBegin = malloc(BUCKETBEGINSIZE * sizeof(int64_t));
    int64_t *SA = malloc((N/K)*sizeof(int64_t));
    int64_t *ISA = malloc((N/K)*sizeof(int64_t));
    int64_t *LCP = malloc((N/K)*sizeof(int64_t));
    if (sparseSA == NULL || intSA == NULL || t_new == NULL ||
        BucketBegin == NULL || SA == NULL || ISA == NULL || LCP == NULL) {
        XFREE(sparseSA);
        XFREE(intSA);
        XFREE(t_new);
        XFREE(BucketBegin);
        XFREE(SA);
        XFREE(ISA);
        XFREE(LCP);
        return NULL;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Build SA
    ///////////////////////////////////////////////////////////////////////////
    sparseSA->S = S;
    sparseSA->K = K;
    sparseSA->N = N;
    sparseSA->logN = (int64_t)ceil(log2((double)(N/K)));
    sparseSA->NKm1 = N/K-1;
    assert(N%K==0);
    uint16_t bucketNr = 1;
    for(int64_t i = 0; i < N/K+1; i++) intSA[i] = i;
    alder_ssa_khan09_radixStep(S, K, BUCKETBEGINSIZE, t_new, intSA, &bucketNr,
                               BucketBegin, 0, N/K-1, 0);
    assert(intSA[N/K] == N/K);
    t_new[N/K] = 0; // Terminate new integer string.
    s = (saint_t) divsufsort64(t_new, intSA, N/K+1);
    if (s != 0) {
        // Error!
    }
    ////////////////////////////////////////////////
    // Translate suffix array: set sparseSA->SA.
    for (int64_t i = 0; i < N/K; i++) SA[i] = intSA[i+1] * K;
    ///////////////////////////////////////////////////////////////////////////
    // Build ISA using sparse SA.
    for (int64_t i = 0; i < N/K; i++) ISA[SA[i]/K] = i;
    ///////////////////////////////////////////////////////////////////////////
    // Initialize LCP : SA + ISA -> LCP
    alder_ssa_khan09_computeLCP(S, SA, ISA, LCP, N, K);
    
    sparseSA->SA = SA;
    sparseSA->ISA = ISA;
    sparseSA->LCP = LCP;
    XFREE(intSA);
    XFREE(t_new);
    XFREE(BucketBegin);
    return sparseSA;
}

int alder_ssa_core_write(const char *fn, alder_ssa_core_t *asc)
{
    
}

alder_ssa_core_t * alder_ssa_core_init_withFile(const char *fn)
{
    
}

void alder_ssa_core_free(alder_ssa_core_t *asc)
{
    if (asc != NULL) {
        XFREE(asc->SA);
        XFREE(asc->ISA);
        XFREE(asc->LCP);
    }
    XFREE(asc);
}
