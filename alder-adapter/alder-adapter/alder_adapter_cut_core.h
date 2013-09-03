//
//  alder_adapter_cut_core.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/1/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_adapter_cut_core_h
#define alder_adapter_alder_adapter_cut_core_h

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
    int filter_uniq;
} alder_adapter_option_t;

__END_DECLS


#endif
