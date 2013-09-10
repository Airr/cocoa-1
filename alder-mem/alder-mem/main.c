/**
 * This file, main.c, is part of alder-mem.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-mem is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-mem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-mem.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmdline.h"
#include "alder_ssa.h"
#include "alder_mem.h"
#include "alder_interval.h"
#include "alder_vector_palign.h"
#include "gsl_vector_match.h"
#include "alder_mem_option.h"

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_mem_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_mem_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_mem_option_free(&option);
        exit(1);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Build SA.
    ///////////////////////////////////////////////////////////////////////////
    alder_sparse_sa_t *sparseSA = alder_sparse_sa_alloc_file(option.refile,
                                                             (int64_t)option.sparse_suffix_array_k);
    if (sparseSA == NULL) {
//        if (option.logfilename != NULL) alder_logger_finalize();
        alder_mem_option_free(&option);
//        alder_logger_error_finalize();
        return 1;
    }
    
    uint64_t min_len = args_info.min_len_mem_arg;
    
//    alder_mem_palign(sparseSA, bdata(option.query), min_len);
    
    ///////////////////////////////////////////////////////////////////////////
    // Find all of maximal exact matches.
    ///////////////////////////////////////////////////////////////////////////
    // Find all MEMs and their positions in the query.
    gsl_vector_match *mem = gsl_vector_match_init();
    alder_mem_main(sparseSA, bdata(option.query), mem, min_len);
    
    gsl_vector_match_fprintf(stdout, mem, NULL);
    
    ///////////////////////////////////////////////////////////////////////////
    // Take the union of L-windows.
    ///////////////////////////////////////////////////////////////////////////
    int64_t query_length = (int64_t) blength(option.query);
    interval_t l_window = alder_interval_union_init();
    for (size_t i = 0; i < mem->size; i++) {
        alder_match_t *m = gsl_vector_match_ptr(mem, i);
        // WARN: interval is 1-based (I need to change this to 0-based).
        alder_interval_union_add(&l_window, m->ref - m->query + 1, m->ref - m->query + query_length);
    }
    alder_interval_union_print(l_window);
    
    ///////////////////////////////////////////////////////////////////////////
    // Add L-windows to alder_vector_palign.
    ///////////////////////////////////////////////////////////////////////////
    alder_vector_palign_t *v_palign = alder_vector_palign_init (1);
    size_t sizeI = alder_interval_union_size(l_window);
    for (size_t i = 0; i < sizeI; i++) {
        int64_t start = alder_interval_union_start(l_window, i);
        int64_t end = alder_interval_union_end(l_window, i);
        alder_vector_palign_data_t m = {
            .s = start - 1,
            .e = end,
            .anchor = alder_vector_interval_init(1),
            .query = NULL,
            .subject = NULL
        };
        alder_vector_palign_add(v_palign, m);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Add intervals to each palign in the following for-loop
    // to assign anchors to L-windows.
    ///////////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < mem->size; i++) {
        alder_match_t *m = gsl_vector_match_ptr(mem, i);
        // m->ref + 1
        int64_t indexI = -1;
        bool isWithinInterval = false;
        alder_interval_union_find_interval_point(l_window, m->ref+1, &indexI,
                                                 &isWithinInterval);
        printf("%zd: %lld %d\n", i, indexI, isWithinInterval);
        // interval - a list of matches.
        //
        alder_vector_palign_data_t *pm = alder_vector_palign_ptr(v_palign, indexI);
        // WARN: this interval is 0-based.
        alder_vector_interval_data_t mI = {
            .s = m->ref,
            .e = m->ref + m->len
        };
        alder_vector_interval_add(pm->anchor, mI);
        alder_vector_interval_data_t mqI = {
            .s = m->query,
            .e = m->query + m->len
        };
        alder_vector_interval_add(pm->qanchor, mqI);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Add query and subject sequences.
    ///////////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < v_palign->size; i++) {
        alder_vector_palign_data_t *pm = alder_vector_palign_ptr(v_palign, i);
        v_palign->data[i].query = bstrcpy(option.query);
        v_palign->data[i].subject = blk2bstr(sparseSA->S + pm->s, (int)(pm->e - pm->s));
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Print the palign vector.
    ///////////////////////////////////////////////////////////////////////////
    alder_vector_palign_fprintf(stdout, v_palign);
    
    
    alder_interval_union_free(l_window);
    
    alder_vector_palign_free(v_palign);
    
    gsl_vector_match_reset(mem);
    gsl_vector_match_free(mem);
    
    my_cmdline_parser_free(&args_info);
    alder_mem_option_free(&option);
    return 0;
}

