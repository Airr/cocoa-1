/**
 * This file, alder_fastq_kseq.h, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fastq_alder_fastq_kseq_h
#define alder_fastq_alder_fastq_kseq_h

#include <stdlib.h>

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

typedef struct __kstring_t
{
    size_t l, m;
    char *s;
} kstring_t;

typedef struct __kstream_t
{
    char *buf;
    int begin, end, is_eof;
    void *f;
    int type; // 0: regular file, 1: gzip file
} kstream_t;

typedef struct
{
    kstring_t name, comment, seq, qual;
    int last_char;
    kstream_t *f;
} kseq_t;

kseq_t * alder_kseq_init (void *fd, int type);
void alder_kseq_destroy (kseq_t * ks);
int alder_kseq_read (kseq_t * seq);

__END_DECLS


#endif /* alder_fastq_alder_fastq_kseq_h */
