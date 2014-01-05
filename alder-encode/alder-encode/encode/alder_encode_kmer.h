/**
 * This file, alder_encode_kmer.h, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_encode_alder_encode_kmer_h
#define alder_encode_alder_encode_kmer_h

#include <stdint.h>
#include <stddef.h>

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

/* This C struct contains variables for converting a sequence to kmers.
 * in - input string
 * out - encoded kmer
 * k - size of kmer
 *
 */
typedef struct alder_encode_kmer_struct {
    uint8_t *in;
    uint8_t *out;
    int k;
    size_t len_in;
    size_t len_out;
    size_t end_in;
    size_t end_out;
} alder_encode_kmer_t;

alder_encode_kmer_t * alder_encode_kmer_create(int kmer_size, size_t len);
int alder_encode_kmer_in(alder_encode_kmer_t *o, char *in, size_t len);
int alder_encode_kmer_convert(alder_encode_kmer_t *o);
void alder_encode_kmer_destroy(alder_encode_kmer_t *o);
int alder_encode_kmer_decode(alder_encode_kmer_t *o);
int alder_encode_kmer_decode_as_number(alder_encode_kmer_t *o);



__END_DECLS


#endif /* alder_encode_alder_encode_kmer_h */
