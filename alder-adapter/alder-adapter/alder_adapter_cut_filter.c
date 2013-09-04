//
//  alder_adapter_cut_filter.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "bstrlib.h"
#include "bstrmore.h"
#include "alder_logger.h"
#include "alder_adapter_cut.h"
#include "alder_adapter_cut_core.h"

//
// bSequence : seq.s
// bQuality  : qual.s
int alder_adapter_cut_filter(const char *adapter_seq, const int m,
                             bstring bSequence, bstring bQuality,
                             int *isFiltered_,
                             int *start2_,
                             const alder_adapter_option_t *option)
{
    // Trims read seqeunces.
    btoupper(bSequence);
    bltrim(bSequence, option->trim_left);
    brtrim(bSequence, option->trim_right);
    bltrim(bQuality, option->trim_left);
    brtrim(bQuality, option->trim_right);
    if (option->trim_ambiguous_left == 0) bltrimc2(bSequence, bQuality, 'N');
    if (option->trim_ambiguous_right == 0) brtrimc2(bSequence, bQuality, 'N');
    bltrimq2(bQuality, bSequence, option->phred+option->trim_quality_left);
    brtrimq2(bQuality, bSequence, option->phred+option->trim_quality_right);
    
    // Cuts adapter parts.
    int start1, best_i, start2, best_j, best_matches, best_cost;
    char *seqS = bstr2cstr(bSequence, '\0');
    alder_adapter_cut(adapter_seq, m, seqS, blength(bSequence),
                      option->error_rate, option->cutadapt_flag,
                      option->cutadapt_degenerate,
                      &start1, &best_i, &start2, &best_j,
                      &best_matches, &best_cost);
    bcstrfree(seqS);
    
    // Filter out reads.
    if (start2 < 0 || blength(bSequence) < start2) {
        return 1;
    }
    btrunc(bSequence, start2);
    btrunc(bQuality, start2);
    int isFiltered = 0;
    if (blength(bSequence) < option->filter_length) isFiltered = 1;
    if (option->filter_ambiguous < bcount(bSequence, 'N')) isFiltered = 1;
    int status = bcountq(bQuality, option->phred+option->filter_quality);
    if (status != BSTR_ERR && status > 0) isFiltered = 1;
    *isFiltered_ = isFiltered;
    *start2_ = start2;

    return 0;
}
