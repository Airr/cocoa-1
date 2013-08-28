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

#ifndef UNIONINTERVALSIZE
#define UNIONINTERVALSIZE 10
#endif

#ifndef UNIONINTERVALINCSIZE
#define UNIONINTERVALINCSIZE 1.2
#endif

// I[0] or interval_t[0] is the size of the array.
// I[1] is the size of elements in the array.
// I[1] <= I[0].
typedef int64_t* interval_t;
size_t bsearch_int64_t(int64_t key, int64_t *v, size_t nel);
interval_t alder_union_interval_init();
int alder_union_interval_add(interval_t *refI, int64_t b, int64_t e);
void alder_union_interval_free(interval_t I);
int alder_union_interval_print(interval_t I);
int alder_union_interval_find_interval_point(interval_t I, int64_t b,
                                             int64_t *indexI,
                                             bool *isWithinInterval);
int alder_union_interval_find_endpoints_point(interval_t I, int64_t p,
                                              int64_t *b, int64_t *e,
                                              bool *isWithinInterval);

__END_DECLS

#endif
