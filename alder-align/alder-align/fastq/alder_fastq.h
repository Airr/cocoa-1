//
//  alder_fastq.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_fastq_h
#define alder_align_alder_fastq_h

#include <zlib.h>
#include "kseq.h"

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

//KSEQ_INIT(gzFile, gzread)



typedef struct __kstream_t
{
    unsigned char *buf;
    int begin, end, is_eof;
    gzFile f;
} kstream_t;

static inline kstream_t *
ks_init (gzFile f)
{
    kstream_t *ks = (kstream_t *) calloc (1, sizeof (kstream_t));
    ks->f = f;
    ks->buf = (unsigned char *) malloc (16384);
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
ks_getc (kstream_t * ks)
{
    if (ks->is_eof && ks->begin >= ks->end)
        return -1;
    if (ks->begin >= ks->end)
    {
        ks->begin = 0;
        ks->end = gzread (ks->f, ks->buf, 16384);
        if (ks->end < 16384)
            ks->is_eof = 1;
        if (ks->end == 0)
            return -1;
    }
    return (int) ks->buf[ks->begin++];
}

static int
ks_getuntil2 (kstream_t * ks, int delimiter, kstring_t * str, int *dret,
              int append)
{
    if (dret)
        *dret = 0;
    str->l = append ? str->l : 0;
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
                ks->end = gzread (ks->f, ks->buf, 16384);
                if (ks->end < 16384)
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
        if (str->m - str->l < (size_t) i - ks->begin + 1)
        {
            str->m = str->l + (i - ks->begin) + 1;
            (--(str->m), (str->m) |= (str->m) >> 1, (str->m) |=
             (str->m) >> 2, (str->m) |= (str->m) >> 4, (str->m) |=
             (str->m) >> 8, (str->m) |= (str->m) >> 16, ++(str->m));
            str->s = (char *) realloc (str->s, str->m);
        }
        ((__builtin_object_size (str->s + str->l, 0) !=
          (size_t) - 1) ? __builtin___memcpy_chk (str->s + str->l,
                                                  ks->buf + ks->begin,
                                                  i - ks->begin,
                                                  __builtin_object_size (str->s
                                                                         +
                                                                         str->l,
                                                                         0)) :
         __inline_memcpy_chk (str->s + str->l, ks->buf + ks->begin,
                              i - ks->begin));
        str->l = str->l + (i - ks->begin);
        ks->begin = i + 1;
        if (i < ks->end)
        {
            if (dret)
                *dret = ks->buf[i];
            break;
        }
    }
    if (str->s == 0)
    {
        str->m = 1;
        str->s = (char *) calloc (1, 1);
    }
    str->s[str->l] = '\0';
    return str->l;
}

static inline int
ks_getuntil (kstream_t * ks, int delimiter, kstring_t * str, int *dret)
{
    return ks_getuntil2 (ks, delimiter, str, dret, 0);
}

typedef struct
{
    kstring_t name, comment, seq, qual;
    int last_char;
    kstream_t *f;
} kseq_t;

static inline kseq_t *
kseq_init (gzFile fd)
{
    kseq_t *s = (kseq_t *) calloc (1, sizeof (kseq_t));
    s->f = ks_init (fd);
    return s;
}

static inline void
kseq_rewind (kseq_t * ks)
{
    ks->last_char = 0;
    ks->f->is_eof = ks->f->begin = ks->f->end = 0;
} static inline void

kseq_destroy (kseq_t * ks)
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

static int
kseq_read (kseq_t * seq)
{
    int c;
    kstream_t *ks = seq->f;
    if (seq->last_char == 0)
    {
        while ((c = ks_getc (ks)) != -1 && c != '>' && c != '@');
        if (c == -1)
            return -1;
        seq->last_char = c;
    }
    seq->comment.l = seq->seq.l = seq->qual.l = 0;
    if (ks_getuntil (ks, 0, &seq->name, &c) < 0)
        return -1;
    if (c != '\n')
        ks_getuntil (ks, '\n', &seq->comment, 0);
    if (seq->seq.s == 0)
    {
        seq->seq.m = 256;
        seq->seq.s = (char *) malloc (seq->seq.m);
    }
    while ((c = ks_getc (ks)) != -1 && c != '>' && c != '+' && c != '@')
    {
        seq->seq.s[seq->seq.l++] = c;
        ks_getuntil2 (ks, '\n', &seq->seq, 0, 1);
    }
    if (c == '>' || c == '@')
        seq->last_char = c;
    if (seq->seq.l + 1 >= seq->seq.m)
    {
        seq->seq.m = seq->seq.l + 2;
        (--(seq->seq.m), (seq->seq.m) |= (seq->seq.m) >> 1, (seq->seq.m) |=
         (seq->seq.m) >> 2, (seq->seq.m) |= (seq->seq.m) >> 4, (seq->seq.m) |=
         (seq->seq.m) >> 8, (seq->seq.m) |= (seq->seq.m) >> 16, ++(seq->seq.m));
        seq->seq.s = (char *) realloc (seq->seq.s, seq->seq.m);
    }
    seq->seq.s[seq->seq.l] = 0;
    if (c != '+')
        return seq->seq.l;
    if (seq->qual.m < seq->seq.m)
    {
        seq->qual.m = seq->seq.m;
        seq->qual.s = (char *) realloc (seq->qual.s, seq->qual.m);
    }
    while ((c = ks_getc (ks)) != -1 && c != '\n');
    if (c == -1)
        return -2;
    while (ks_getuntil2 (ks, '\n', &seq->qual, 0, 1) >= 0
           && seq->qual.l < seq->seq.l);
    seq->last_char = 0;
    if (seq->seq.l != seq->qual.l)
        return -2;
    return seq->seq.l;
}


typedef struct fastq_t {
	gzFile fp;
	kseq_t *seq;
} fastq_t;

fastq_t *alder_fastq_alloc(const char *fn);
int alder_fastq_next(fastq_t *f);
void alder_fastq_free(fastq_t *f);
int peach_readfq(const char* path);
int peachFastqPreprocess(const char* path);
//int randomFastqUsingGFF(const char* fastaFilename, const char* gffFilename);
//int randomFastqUsingFasta(const char* fastaFilename, unsigned int numberOfRead);
__END_DECLS

#endif