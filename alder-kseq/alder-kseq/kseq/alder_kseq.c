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
 * This file, alder_kseq.c, is part of alder-kseq. See the license above 
 * by Heng Li.
 */

#include <stdio.h>
#include "alder_kseq.h"

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

#pragma mark file

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
// int - to - void*
// int i
// (void *) (intptr_t) i
//
// void* - to - int
// void *param
// int id = (intptr_t) param;

static inline kstream_t *
file_ks_init (int f)
{
    kstream_t *ks = (kstream_t *) calloc (1, sizeof (kstream_t));
    ks->f = (void *)(intptr_t)f;
    ks->buf = (char *) malloc (4096);
    return ks;
}

static inline void
ks_destroy (kstream_t * ks)
{
    if (ks)
    {
        free (ks->buf);
        free (ks);
    }
}

static inline int
file_ks_getc (kstream_t * ks)
{
    if (ks->is_eof && ks->begin >= ks->end)
        return -1;
    if (ks->begin >= ks->end)
    {
        ks->begin = 0;
        ks->end = (int)read ((int)((intptr_t)ks->f), ks->buf, 4096);
        if (ks->end == -1) {
            fprintf(stderr, "file_ks_getc: %s,", strerror(errno));
        }
        if (ks->end < 4096)
            ks->is_eof = 1;
        if (ks->end == 0)
            return -1;
    }
    return (int) ks->buf[ks->begin++];
}

static int
file_ks_getuntil (kstream_t * ks, int delimiter, kstring_t * str, int *dret)
{
    if (dret)
        *dret = 0;
    str->l = 0;
    if (ks->begin >= ks->end && ks->is_eof)
        return -1;
    for (;;)
    {
        int i;
        if (ks->begin >= ks->end)
        {
            if (!ks->is_eof)
            {
                ks->begin = 0;
                ks->end = (int)read ((int)((intptr_t)ks->f), ks->buf, 4096);
                if (ks->end < 4096)
                    ks->is_eof = 1;
                if (ks->end == 0)
                    break;
            }
            else
                break;
        }
        if (delimiter > 1)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (ks->buf[i] == delimiter)
                    break;
        }
        else if (delimiter == 0)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (isspace (ks->buf[i]))
                    break;
        }
        else if (delimiter == 1)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (isspace (ks->buf[i]) && ks->buf[i] != ' ')
                    break;
        }
        else
            i = 0;
        if (str->m - str->l < i - ks->begin + 1)
        {
            str->m = str->l + (i - ks->begin) + 1;
            kroundup32(str->m);
            str->s = (char *) realloc (str->s, str->m);
        }
        memcpy(str->s + str->l, ks->buf + ks->begin, i - ks->begin);
        str->l = str->l + (i - ks->begin);
        ks->begin = i + 1;
        if (i < ks->end)
        {
            if (dret)
                *dret = ks->buf[i];
            break;
        }
    }
    if (str->l == 0)
    {
        str->m = 1;
        str->s = (char *) calloc (1, 1);
    }
    str->s[str->l] = '\0';
    return (int)str->l;
}

static inline kseq_t *
file_kseq_init (int fd)
{
    kseq_t *s = (kseq_t *) calloc (1, sizeof (kseq_t));
    s->f = file_ks_init (fd);
    return s;
}

static inline void
kseq_rewind (kseq_t * ks)
{
    ks->last_char = 0;
    ks->f->is_eof = ks->f->begin = ks->f->end = 0;
}

static int
file_kseq_read (kseq_t * seq)
{
    int c;
    kstream_t *ks = seq->f;
    if (seq->last_char == 0)
    {
        while ((c = file_ks_getc (ks)) != -1 && c != '>' && c != '@');
        if (c == -1)
            return -1;
        seq->last_char = c;
    }
    seq->comment.l = seq->seq.l = seq->qual.l = 0;
    if (file_ks_getuntil (ks, 0, &seq->name, &c) < 0)
        return -1;
    if (c != '\n')
        file_ks_getuntil (ks, '\n', &seq->comment, 0);
    while ((c = file_ks_getc (ks)) != -1 && c != '>' && c != '+' && c != '@')
    {
        if (isgraph (c))
        {
            if (seq->seq.l + 1 >= seq->seq.m)
            {
                seq->seq.m = seq->seq.l + 2;
                kroundup32(seq->seq.m);
                seq->seq.s = (char *) realloc (seq->seq.s, seq->seq.m);
            }
            seq->seq.s[seq->seq.l++] = (char) c;
        }
    } if (c == '>' || c == '@')
        seq->last_char = c;
    seq->seq.s[seq->seq.l] = 0;
    if (c != '+')
        return (int)seq->seq.l;
    if (seq->qual.m < seq->seq.m)
    {
        seq->qual.m = seq->seq.m;
        seq->qual.s = (char *) realloc (seq->qual.s, seq->qual.m);
    }
    while ((c = file_ks_getc (ks)) != -1 && c != '\n');
    if (c == -1)
        return -2;
    while ((c = file_ks_getc (ks)) != -1 && seq->qual.l < seq->seq.l)
        if (c >= 33 && c <= 127)
            seq->qual.s[seq->qual.l++] = (unsigned char) c;
    seq->qual.s[seq->qual.l] = 0;
    seq->last_char = 0;
    if (seq->seq.l != seq->qual.l)
        return -2;
    return (int)seq->seq.l;
}

#pragma mark gzip

//
//  alder_kseq.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static inline kstream_t *
gzip_ks_init (gzFile f)
{
    kstream_t *ks = (kstream_t *) calloc (1, sizeof (kstream_t));
    ks->f = f;
    ks->buf = (char *) malloc (4096);
    ks->type = 1;
    return ks;
}

//static inline void
//ks_destroy (kstream_t * ks)
//{
//    if (ks)
//    {
//        free (ks->buf);
//        free (ks);
//    }
//}

static inline int
gzip_ks_getc (kstream_t * ks)
{
    if (ks->is_eof && ks->begin >= ks->end)
        return -1;
    if (ks->begin >= ks->end)
    {
        ks->begin = 0;
        ks->end = gzread ((gzFile)ks->f, ks->buf, 4096);
        if (ks->end < 4096)
            ks->is_eof = 1;
        if (ks->end == 0)
            return -1;
    }
    return (int) ks->buf[ks->begin++];
}

static int
gzip_ks_getuntil (kstream_t * ks, int delimiter, kstring_t * str, int *dret)
{
    if (dret)
        *dret = 0;
    str->l = 0;
    if (ks->begin >= ks->end && ks->is_eof)
        return -1;
    for (;;)
    {
        int i;
        if (ks->begin >= ks->end)
        {
            if (!ks->is_eof)
            {
                ks->begin = 0;
                ks->end = gzread ((gzFile)ks->f, ks->buf, 4096);
                if (ks->end < 4096)
                    ks->is_eof = 1;
                if (ks->end == 0)
                    break;
            }
            else
                break;
        }
        if (delimiter > 1)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (ks->buf[i] == delimiter)
                    break;
        }
        else if (delimiter == 0)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (isspace (ks->buf[i]))
                    break;
        }
        else if (delimiter == 1)
        {
            for (i = ks->begin; i < ks->end; ++i)
                if (isspace (ks->buf[i]) && ks->buf[i] != ' ')
                    break;
        }
        else
            i = 0;
        if (str->m - str->l < i - ks->begin + 1)
        {
            str->m = str->l + (i - ks->begin) + 1;
            kroundup32(str->m);
            str->s = (char *) realloc (str->s, str->m);
        }
        memcpy(str->s + str->l, ks->buf + ks->begin, i - ks->begin);
        str->l = str->l + (i - ks->begin);
        ks->begin = i + 1;
        if (i < ks->end)
        {
            if (dret)
                *dret = ks->buf[i];
            break;
        }
    }
    if (str->l == 0)
    {
        str->m = 1;
        str->s = (char *) calloc (1, 1);
    }
    str->s[str->l] = '\0';
    return (int)str->l;
}

static inline kseq_t *
gzip_kseq_init (gzFile fd)
{
    kseq_t *s = (kseq_t *) calloc (1, sizeof (kseq_t));
    s->f = gzip_ks_init (fd);
    return s;
}

//static inline void
//kseq_rewind (kseq_t * ks)
//{
//    ks->last_char = 0;
//    ks->f->is_eof = ks->f->begin = ks->f->end = 0;
//}

//static inline void
//kseq_destroy (kseq_t * ks)
//{
//    if (!ks)
//        return;
//    free (ks->name.s);
//    free (ks->comment.s);
//    free (ks->seq.s);
//    free (ks->qual.s);
//    ks_destroy (ks->f);
//    free (ks);
//}

static int
gzip_kseq_read (kseq_t * seq)
{
    int c;
    kstream_t *ks = seq->f;
    if (seq->last_char == 0)
    {
        while ((c = gzip_ks_getc (ks)) != -1 && c != '>' && c != '@');
        if (c == -1)
            return -1;
        seq->last_char = c;
    }
    seq->comment.l = seq->seq.l = seq->qual.l = 0;
    if (gzip_ks_getuntil (ks, 0, &seq->name, &c) < 0)
        return -1;
    if (c != '\n')
        gzip_ks_getuntil (ks, '\n', &seq->comment, 0);
    while ((c = gzip_ks_getc (ks)) != -1 && c != '>' && c != '+' && c != '@')
    {
        if (isgraph (c))
        {
            if (seq->seq.l + 1 >= seq->seq.m)
            {
                seq->seq.m = seq->seq.l + 2;
                kroundup32(seq->seq.m);
                seq->seq.s = (char *) realloc (seq->seq.s, seq->seq.m);
            }
            seq->seq.s[seq->seq.l++] = (char) c;
        }
    } if (c == '>' || c == '@')
        seq->last_char = c;
    seq->seq.s[seq->seq.l] = 0;
    if (c != '+')
        return (int)seq->seq.l;
    if (seq->qual.m < seq->seq.m)
    {
        seq->qual.m = seq->seq.m;
        seq->qual.s = (char *) realloc (seq->qual.s, seq->qual.m);
    }
    while ((c = gzip_ks_getc (ks)) != -1 && c != '\n');
    if (c == -1)
        return -2;
    while ((c = gzip_ks_getc (ks)) != -1 && seq->qual.l < seq->seq.l)
        if (c >= 33 && c <= 127)
            seq->qual.s[seq->qual.l++] = (unsigned char) c;
    seq->qual.s[seq->qual.l] = 0;
    seq->last_char = 0;
    if (seq->seq.l != seq->qual.l)
        return -2;
    return (int)seq->seq.l;
}


#pragma mark main

kseq_t * alder_kseq_init (void *fd, int type)
{
    if (type == 0) {
        return file_kseq_init((int)((intptr_t)fd));
    } else {
        return gzip_kseq_init((gzFile)fd);
    }
}

void
alder_kseq_destroy (kseq_t * ks)
{
    if (!ks)
        return;
    free (ks->name.s);
    free (ks->comment.s);
    free (ks->seq.s);
    free (ks->qual.s);
    ks_destroy (ks->f);
    free (ks);
}

// Return:
//   -1 = if seq is invalid.
int alder_kseq_read (kseq_t * seq)
{
    if (seq == NULL) return -1;
    if (seq->f->type == 0) {
        return file_kseq_read(seq);
    } else {
        return gzip_kseq_read(seq);
    }
}
