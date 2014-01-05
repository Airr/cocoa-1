/**
 * This file, alder_hashtable_dsk.h, is part of alder-hashtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-hashtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hashtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hashtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_hashtable_alder_hashtable_dsk_h
#define alder_hashtable_alder_hashtable_dsk_h

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

typedef struct alder_hashtable_dsk_struct
alder_hashtable_dsk_t;
    
enum {
    ALDER_HASHTABLE_SUCCESS,
    ALDER_HASHTABLE_ERROR
};

alder_hashtable_dsk_t * alder_hashtable_dsk_create(int k, size_t size, uint64_t *empty_key);
void alder_hashtable_dsk_destroy(alder_hashtable_dsk_t *o);
void alder_hashtable_dsk_empty_key(alder_hashtable_dsk_t *o,
                                   uint64_t *empty_key);
int alder_hashtable_dsk_increment(alder_hashtable_dsk_t *o, uint64_t *key);

int alder_hashtable_dsk_print_header_with_filename(alder_hashtable_dsk_t *o,
                                                   const char *fn,
                                                   int kmer_size,
                                                   int hashtable_size,
                                                   uint64_t n_ni,
                                                   uint64_t n_np);
int alder_hashtable_dsk_print_header_with_descriptor(alder_hashtable_dsk_t *o,
                                                     int fildes,
                                                     int kmer_size,
                                                     int hashtable_size,
                                                     uint64_t n_ni,
                                                     uint64_t n_np);
int alder_hashtable_dsk_print_header_with_FILE(alder_hashtable_dsk_t *o,
                                               FILE *fp,
                                               int kmer_size,
                                               int hashtable_size,
                                               uint64_t n_ni,
                                               uint64_t n_np);
int alder_hashtable_dsk_print_with_filename(alder_hashtable_dsk_t *o, const char *fn);
int alder_hashtable_dsk_print_with_descriptor(alder_hashtable_dsk_t *o, int fildes);
int alder_hashtable_dsk_print_with_FILE(alder_hashtable_dsk_t *o, FILE *fp);
alder_hashtable_dsk_t * alder_hashtable_dsk_load(const char *fn);
int alder_hashtable_dsk_print(alder_hashtable_dsk_t *o, FILE *fp);
int alder_hashtable_dsk_reset(alder_hashtable_dsk_t *o);

__END_DECLS


#endif /* alder_hashtable_alder_hashtable_dsk_h */
