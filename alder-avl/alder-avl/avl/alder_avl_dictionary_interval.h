/**
 * This file, alder_avl_dictionary_interval.h, is part of alder-avl.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-avl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-avl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-avl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_avl_alder_avl_dictionary_interval_h
#define alder_avl_alder_avl_dictionary_interval_h

#include <stdint.h>
#include "rb.h"

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
    int64_t a;
    int64_t b;
} alder_avl_dictionary_interval_t;

#ifndef LIBAVL_ALLOCATOR
#define LIBAVL_ALLOCATOR
struct libavl_allocator
{
    void *(*libavl_malloc) (struct libavl_allocator *, size_t libavl_size);
    void (*libavl_free) (struct libavl_allocator *, void *libavl_block);
};
#endif

int compare_interval (const void *pa, const void *pb, void *param);
struct rb_table* alder_avl_dictionary_interval_create(int64_t a, int64_t b);
int alder_avl_dictionary_interval_insert(int64_t a, int64_t b);
int alder_avl_dictionary_interval_destroy(struct rb_table*);
int alder_avl_dictionary_interval();

__END_DECLS


#endif /* alder_avl_alder_avl_dictionary_interval_h */
