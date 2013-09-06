/**
 * This file, alder_fastq_ragel.h, is part of alder-fastq.
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

#ifndef alder_fastq_alder_fastq_ragel_h
#define alder_fastq_alder_fastq_ragel_h

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

#define BUFSIZE 10000

typedef struct __rstring_t
{
    size_t l, m;
    char s[BUFSIZE];
} rstring_t;

typedef struct __rstream_t
{
    char buf[BUFSIZE];
    int have;
    int cs;
    int stop;
    int begin, end, is_eof;
    void *f;
    int type; // 0: regular file, 1: gzip file
} rstream_t;

typedef struct
{
    rstring_t name, comment, seq, qual; 
    int last_char;
    rstream_t *f;
} rseq_t;

rseq_t * alder_rseq_init (const char *fn, int type);
void alder_rseq_destroy (rseq_t * ks);
int alder_rseq_read (rseq_t * seq);

__END_DECLS


#endif /* alder_fastq_alder_fastq_ragel_h */
