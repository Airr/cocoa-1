/**
 * This file, alder_seq_streamer.h, is part of alder-seq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-seq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-seq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-seq.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_seq_alder_seq_streamer_h
#define alder_seq_alder_seq_streamer_h

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

int alder_seq_streamer(int *len_content, char *buf, char *buf2,
                       size_t size_buf, FILE *fp,
                       int kmer_size, int max_name);

__END_DECLS


#endif /* alder_seq_alder_seq_streamer_h */
