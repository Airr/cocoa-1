//
//  alder_calignmodule.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_calignmodule_h
#define alder_adapter_alder_calignmodule_h

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

int alder_adapter_cut(const char *s1,
                      const int m,
                      const char *s2,
                      const int n,
                      const double error_rate,
                      const int flags,
                      const int degenerate,
                      int *start1_,
                      int *best_i_,
                      int *start2_,
                      int *best_j_,
                      int *best_matches_,
                      int *best_cost_);

__END_DECLS

#endif
