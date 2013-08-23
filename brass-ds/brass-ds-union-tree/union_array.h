//
//  union_array.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/21/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_union_array_h
#define brass_ds_union_array_h

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

#ifndef CAPACITYI
#define CAPACITYI 100
#endif

typedef int64_t key_t;

typedef struct endpoint_struct {
    key_t key;
    enum {BEGIN, END} dir;
} endpoint_t;

typedef struct interval_struct {
    key_t b;
    key_t e;
} interval_t;

endpoint_t *create_union_array(interval_t *I, size_t sizeI);

__END_DECLS

#endif
