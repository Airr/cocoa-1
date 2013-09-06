/**
 * This file, alder_adapter_option.h, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_adapter_alder_adapter_option_h
#define alder_adapter_alder_adapter_option_h

#include "bstrlib.h"

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

typedef struct {
    int is_stdin;
    double error_rate;
    int cutadapt_flag;
    int cutadapt_degenerate;
    int keep;
    int phred;
    int trim_left;
    int trim_right;
    int trim_ambiguous_left;
    int trim_ambiguous_right;
    int trim_quality_left;
    int trim_quality_right;
    int filter_length;
    int filter_quality;
    int filter_ambiguous;
    int filter_duplicates;
    int *pair;
    struct bstrList *adapter;
    struct bstrList *infile;
    struct bstrList *outfile;
    bstring logfilename;
} alder_adapter_option_t;

void alder_adapter_option_free(alder_adapter_option_t *option);

__END_DECLS


#endif /* alder_adapter_alder_adapter_option_h */
