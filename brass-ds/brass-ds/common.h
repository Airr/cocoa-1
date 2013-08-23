//
//  common.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/16/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_common_h
#define brass_ds_common_h

#include <stdint.h>

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

typedef int64_t item_t;
typedef int64_t key_t;
typedef uint64_t endpoint_t;

__END_DECLS

#endif
