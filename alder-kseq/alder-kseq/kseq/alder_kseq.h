/*  This is just a Heng Li's kseq.
 */
/* The MIT License
 
 Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 
 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

/**
 * This file, alder_kseq.h, is part of alder-kseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-kseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kseq.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_kseq_alder_kseq_h
#define alder_kseq_alder_kseq_h

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

#endif /* alder_kseq_alder_kseq_h */
