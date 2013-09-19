/**
 * This file, alder_denovo_match.c, is part of alder-denovo.
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
#include "alder_seqid.h"
#include "alder_ssa.h"
#include "alder_denovo_match.h"
//#include "alder_mem.h"
#include "alder_ssa_khan09.h"


void alder_denovo_match(struct bstrList *fn)
{
    for (int i = 0; i < fn->qty; i++) {
        bstring bfilename = fn->entry[i];
//        fprintf(stdout, "file: %s\n", bdata(bfilename));
    }
    
    // Concatenate sequences.
    alder_seqid_t *ast = NULL;
    ast = alder_seqid_init(fn);
    if (ast != NULL) {
        alder_seqid_fprintf(stdout, ast);
//        alder_seqid_free(ast);
    }
    
    // Make SSA.
    alder_ssa_core_t *asc = alder_ssa_core_init_withS(ast->data,
                                                      ast->numberOfBase,
                                                      ast->sizeCapacity,
                                                      2);
    if (asc == NULL) {
        return;
    }
    
    // Find all possible pairs.
    // Take one read and find MEMs of it.
    // Use alder_seq_t to take one.
    alder_seqid_open(ast);
    for (int64_t i = 0; i < ast->numberOfSequence; i++) {
        bstring a = alder_seqid_read(ast);
        fprintf(stdout, "%s\n", bdata(a));
        
        gsl_vector_match *mem = gsl_vector_match_init();
        alder_ssa_khan09_MEM(asc->S, asc->SA, asc->ISA, asc->LCP, asc->N,
                             asc->K, asc->logN, asc->NKm1, bdata(a),
                             mem, 3, true, 1);
        gsl_vector_match_fprintf(stdout, mem, NULL);
        gsl_vector_match_free(mem);
        
        bdestroy(a);
    }
    
    alder_seqid_free(ast);
    alder_ssa_core_free(asc);

}
