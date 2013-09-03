//
//  alder_adatper_cmd_check.h
//  alder-adapter
//
//  Created by Sang Chul Choi on 9/1/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_adapter_alder_adapter_cmd_check_h
#define alder_adapter_alder_adapter_cmd_check_h

#include "alder_adapter_cut_core.h"
#include "cmdline.h"

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

int alder_adatper_cmd_check(alder_adapter_option_t *option, struct gengetopt_args_info *args_info);

__END_DECLS


#endif
