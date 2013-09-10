/**
 * This file, alder_vector_interval.h, is part of alder-vector.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-vector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-vector is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-vector.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_vector_alder_vector_interval_h
#define alder_vector_alder_vector_interval_h

#include <stdint.h>
#include <gsl/gsl_permutation.h>

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
    // Add your data members (e.g., int i;)
    int64_t s;
    int64_t e;
} alder_vector_interval_data_t;

typedef struct {
    size_t size;
    alder_vector_interval_data_t * data;
} alder_vector_interval_block_t;

typedef struct {
    size_t size;
    alder_vector_interval_data_t * data;
    alder_vector_interval_block_t * block;
} alder_vector_interval_t;

#pragma mark test

void cmd_alder_vector_interval(int * a, size_t n);

#pragma mark block

alder_vector_interval_block_t *
alder_vector_interval_block_t_alloc (const size_t n);

alder_vector_interval_block_t *
alder_vector_interval_block_t_calloc (const size_t n);

void
alder_vector_interval_block_t_free (alder_vector_interval_block_t * b);

size_t
alder_vector_interval_block_t_size (const alder_vector_interval_block_t * b);

alder_vector_interval_data_t *
alder_vector_interval_block_t_data (const alder_vector_interval_block_t * b);

alder_vector_interval_block_t *
alder_vector_interval_block_t_realloc (alder_vector_interval_block_t *b,
                                       const size_t n);

int
alder_vector_interval_raw_fprintf (FILE * stream,
                                   const alder_vector_interval_data_t * data,
                                   const size_t n);

#pragma mark vector

alder_vector_interval_t *
alder_vector_interval_alloc (const size_t n);

void
alder_vector_interval_free (alder_vector_interval_t * v);

alder_vector_interval_t *
alder_vector_interval_init (const size_t n);

int
alder_vector_interval_add (alder_vector_interval_t * v,
                           const alder_vector_interval_data_t m);

alder_vector_interval_data_t
alder_vector_interval_get (const alder_vector_interval_t * v, const size_t i);

void
alder_vector_interval_set (alder_vector_interval_t * v,
                           const size_t i, alder_vector_interval_data_t x);

alder_vector_interval_data_t *
alder_vector_interval_ptr (alder_vector_interval_t * v, const size_t i);

const alder_vector_interval_data_t *
alder_vector_interval_const_ptr (const alder_vector_interval_t * v,
                                 const size_t i);

int
alder_vector_interval_fprintf (FILE * stream,
                               const alder_vector_interval_t * v);

#pragma mark sort

void
alder_vector_interval_sort_interval (alder_vector_interval_data_t * data, const size_t stride, const size_t n);

void
alder_vector_interval_sort_vector_interval (alder_vector_interval_t * v);

void
alder_vector_interval_sort_interval_index (size_t * p, const alder_vector_interval_data_t * data, const size_t stride, const size_t n);

int
alder_vector_interval_sort_vector_interval_index (gsl_permutation * permutation, const alder_vector_interval_t * v);


__END_DECLS


#endif /* alder_vector_alder_vector_interval_h */
