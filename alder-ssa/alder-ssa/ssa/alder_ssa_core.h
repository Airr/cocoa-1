/**
 * This file, alder_ssa_core.h, is part of alder-denovo.
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

#ifndef alder_denovo_alder_ssa_core_h
#define alder_denovo_alder_ssa_core_h

#include <stdint.h>
#include "alder_fasta.h"
#include "alder_seqid.h"

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

enum alderTypeS { ALDER_SSA_FASTA, ALDER_SSA_SEQID };

/**
 * This struct would replace alder_sparse_sa_t.
 * K - Sparse suffix array stores every K-th suffix.
 * N - Length of the text with dollar signs.
 * logN - floor[log_2 N] - convenient members.
 * NKm1 - N/K-1 - convenient members.
 * SA - Suffix array.           (2)
 * ISA - Inverse suffix array.  (3)
 * LCP - Longest common prefix. (4)
 * S - a long sequence.         (1) Just a pointer to a string in fS, sS.
 */
typedef struct
{
    int64_t           K;
    int64_t           N;
    int64_t           logN;
    int64_t           NKm1;
    int64_t           *SA;
    int64_t           *ISA;
    int64_t           *LCP;
    enum alderTypeS   typeS;
    union {
        alder_fasta_t *fS;
        alder_seqid_t *sS;
    };
    char              *S;
} alder_ssa_core_t;

alder_ssa_core_t * alder_ssa_core_init_withS(char *S,
                                             const int64_t N,
                                             const int64_t M,
                                             const int64_t K);
int alder_ssa_core_write(const char *fn, alder_ssa_core_t *asc);
alder_ssa_core_t * alder_ssa_core_init_withFile(const char *fn);
void alder_ssa_core_free(alder_ssa_core_t *asc);

__END_DECLS


#endif /* alder_denovo_alder_ssa_core_h */
