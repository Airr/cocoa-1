/**
 * This file, alder_fileseq_chunk.h, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_fileseq_alder_fileseq_chunk_h
#define alder_fileseq_alder_fileseq_chunk_h

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

#define BUFSIZE (1 << 16)

int
alder_fileseq_chunk(size_t *lenBuf, char *buf, size_t sizeBuf,
                    size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                    int kmer_size, FILE *fp, void *fx, int type);

__END_DECLS


#endif /* alder_fileseq_alder_fileseq_chunk_h */
