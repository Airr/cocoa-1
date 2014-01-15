/**
 * This file, bstrmore.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bstrmore.h"

#ifndef bstr__alloc
#define bstr__alloc(x) malloc (x)
#endif

#ifndef bstr__free
#define bstr__free(p) free (p)
#endif

#ifndef bstr__realloc
#define bstr__realloc(p,x) realloc ((p), (x))
#endif

#ifndef bstr__memcpy
#define bstr__memcpy(d,s,l) memcpy ((d), (s), (l))
#endif

#ifndef bstr__memmove
#define bstr__memmove(d,s,l) memmove ((d), (s), (l))
#endif

#ifndef bstr__memset
#define bstr__memset(d,c,l) memset ((d), (c), (l))
#endif

#ifndef bstr__memcmp
#define bstr__memcmp(d,c,l) memcmp ((d), (c), (l))
#endif

#ifndef bstr__memchr
#define bstr__memchr(s,c,l) memchr ((s), (c), (l))
#endif

/*
 * int bltrimc (bstring b, char c)
 *
 * Delete contiguous character c's from the left end of the string.
 */
int bltrimc (bstring b, char c) {
int i, len;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (len = b->slen, i = 0; i < len; i++) {
		if (b->data[i] != (unsigned char)c) {
			return bdelete (b, 0, i);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}

/*
 * int brtrimc (bstring b, char c)
 *
 * Delete contiguous character c's from the right end of the string.
 */
int brtrimc (bstring b, char c) {
int i;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] != (unsigned char)c) {
			if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
			b->slen = i + 1;
			return BSTR_OK;
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}

/*
 * int btrimc (bstring b, char c)
 *
 * Delete contiguous character c's from both ends of the string.
 */
int btrimc (bstring b, char c) {
int i, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] != (unsigned char)c) {
			if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
			b->slen = i + 1;
			for (j = 0; b->data[j] == (unsigned char)c; j++) {}
			return bdelete (b, 0, j);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}

/*
 * int bltrimc2 (bstring b, bstring b2, char c)
 *
 * Delete contiguous character c's from the left end of the string b.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int bltrimc2 (bstring b, bstring b2, char c) {
int i, len;
    

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (len = b->slen, i = 0; i < len; i++) {
		if (b->data[i] != (unsigned char)c) {
            bdelete(b2, 0, i);
			return bdelete (b, 0, i);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}

/*
 * int brtrimc2 (bstring b, bstring b2, char c)
 *
 * Delete contiguous character c's from the right end of the string.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int brtrimc2 (bstring b, bstring b2, char c) {
int i;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] != (unsigned char)c) {
			if (b->mlen > i) {
                b->data[i+1] = (unsigned char) '\0';
                b2->data[i+1] = (unsigned char) '\0';
            }
			b->slen = i + 1;
			b2->slen = i + 1;
			return BSTR_OK;
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}

/*
 * int btrimc2 (bstring b, bstring b2, char c)
 *
 * Delete contiguous character c's from both ends of the string.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int btrimc2 (bstring b, bstring b2, char c) {
int i, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] != (unsigned char)c) {
			if (b->mlen > i) {
                b->data[i+1] = (unsigned char) '\0';
                b2->data[i+1] = (unsigned char) '\0';
            }
			b->slen = i + 1;
			b2->slen = i + 1;
            for (j = 0; b->data[j] == (unsigned char)c; j++) {}
            bdelete(b2, 0, j);
			return bdelete (b, 0, j);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}


/*
 * int bltrimq (bstring b, int c)
 *
 * Delete contiguous characters that is less than c from the left end of the string.
 */
int bltrimq (bstring b, int c) {
int i, len;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (len = b->slen, i = 0; i < len; i++) {
		if (b->data[i] >= (unsigned char)c) {
			return bdelete (b, 0, i);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}











/*
 * int brtrimq (bstring b, int c)
 *
 * Delete contiguous characters that is less than c from the right end of the string.
 */
int brtrimq (bstring b, int c) {
int i;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] >= (unsigned char)c) {
			if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
			b->slen = i + 1;
			return BSTR_OK;
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}

/*
 * int btrimq (bstring b, int c)
 *
 * Delete contiguous characters that is less than c from the both end of the string.
 */
int btrimq (bstring b, int c) {
int i, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] >= (unsigned char)c) {
			if (b->mlen > i) b->data[i+1] = (unsigned char) '\0';
			b->slen = i + 1;
			for (j = 0; b->data[j] < (unsigned char)c; j++) {}
			return bdelete (b, 0, j);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	return BSTR_OK;
}

/*
 * int bltrimc2 (bstring b, bstring b2, int c)
 *
 * Delete contiguous characters that is less than c from the left end of the string.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int bltrimq2 (bstring b, bstring b2, int c) {
int i, len;
    

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (len = b->slen, i = 0; i < len; i++) {
		if (b->data[i] >= (unsigned char)c) {
            bdelete(b2, 0, i);
			return bdelete (b, 0, i);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}

/*
 * int brtrimq2 (bstring b, bstring b2, int c)
 *
 * Delete contiguous characters that is less than c from the right end of the string.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int brtrimq2 (bstring b, bstring b2, int c) {
int i;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] >= (unsigned char)c) {
			if (b->mlen > i) {
                b->data[i+1] = (unsigned char) '\0';
                b2->data[i+1] = (unsigned char) '\0';
            }
			b->slen = i + 1;
			b2->slen = i + 1;
			return BSTR_OK;
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}

/*
 * int btrimq2 (bstring b, bstring b2, int c)
 *
 * Delete contiguous characters that is less than c from the both end of the string.
 * String b2 is of as the same size as that of string b.
 * We also trim b2 as we do the b. If they are differnt in size, we return
 * BSTR_ERR.
 */
int btrimq2 (bstring b, bstring b2, int c) {
int i, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0 || b->slen != b2->slen) return BSTR_ERR;

	for (i = b->slen - 1; i >= 0; i--) {
		if (b->data[i] >= (unsigned char)c) {
			if (b->mlen > i) {
                b->data[i+1] = (unsigned char) '\0';
                b2->data[i+1] = (unsigned char) '\0';
            }
			b->slen = i + 1;
			b2->slen = i + 1;
            for (j = 0; b->data[j] < (unsigned char)c; j++) {}
            bdelete(b2, 0, j);
			return bdelete (b, 0, j);
		}
	}

	b->data[0] = (unsigned char) '\0';
	b->slen = 0;
	b2->data[0] = (unsigned char) '\0';
	b2->slen = 0;
	return BSTR_OK;
}


/*
 * int bcount (bstring b, char c)
 *
 * Count character c in string b.
 */
int bcount (bstring b, char c) {
int i, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;
    
    j = 0;
	for (i = 0; i < b->slen; i++) {
        if (b->data[i] == (unsigned char)c) j++;
    }
    
	return j;
}

/*
 * int bcountq (bstring b, int c)
 *
 * Count characters that are less than c.
 */
int bcountq (bstring b, int c) {
int i, len, j;

	if (b == NULL || b->data == NULL || b->mlen < b->slen ||
	    b->slen < 0 || b->mlen <= 0) return BSTR_ERR;

    j = 0;
	for (len = b->slen, i = 0; i < len; i++) {
		if (b->data[i] < (unsigned char)c) j++;
	}

	return j;
}

/*
 * Create a list of bstring with at least n.
 * This does not own the elements of bstrings.
 */
struct bstrList * bstrViewCreate()
{
    return bstrListCreate();
}

int bstrViewAdd(struct bstrList *sl, bstring b)
{
    if (sl->qty >= sl->mlen - 1) {
        int s = bstrListAlloc(sl, sl->mlen*2);
        if (s != BSTR_OK) {
            return s;
        }
    }
    sl->entry[sl->qty] = b;
    sl->qty++;
    return BSTR_OK;
}

/* 
 * Destory a list without freeing bstring elements.
 */
int bstrViewDestroy(struct bstrList *sl)
{
	if (sl == NULL || sl->qty < 0) return BSTR_ERR;
	sl->qty  = -1;
	sl->mlen = -1;
	bstr__free (sl->entry);
	sl->entry = NULL;
	bstr__free (sl);
	return BSTR_OK;
}


/*
 * Create a list of bstring with at least n.
 */
struct bstrList * bstrVectorCreate(int n)
{
    struct bstrList *sl = bstrListCreate();
    if (sl == NULL) {
        return NULL;
    }
    int s = bstrListAlloc(sl, n);
    if (s != BSTR_OK) {
        bstrListDestroy(sl);
        return NULL;
    }
    assert(sl->qty == 0);
    memset(sl->entry, 0, sl->mlen * sizeof(bstring));
    return sl;
}

void bstrVectorAddBstring(struct bstrList *sl, bstring b)
{
    if (sl->qty >= sl->mlen - 1) {
        int s = bstrListAlloc(sl, sl->mlen*2);
        if (s != BSTR_OK) {
            return;
        }
        assert(s == BSTR_OK);
    }
    sl->entry[sl->qty] = b;
    sl->qty++;
    return;
}

/* This function adds string s to the given string list sl.
 * returns
 * BSTR_OK or BSTR_ERR
 */
int bstrVectorAdd(struct bstrList *sl, const char *s)
{
    if (sl->qty >= sl->mlen - 1) {
        int s = bstrListAlloc(sl, sl->mlen*2);
        if (s != BSTR_OK) {
            return BSTR_ERR;
        }
    }
    bstring b = bfromcstr(s);
    if (b == NULL) {
        return BSTR_ERR;
    }
    sl->entry[sl->qty] = b;
    sl->qty++;
    return BSTR_OK;
}

int bstrVectorDelete(struct bstrList *sl)
{
    return bstrListDestroy(sl);
}

/* This function copies strings in the sl list.
 * returns 
 * a new string list or NULL
 */
struct bstrList * bstrVectorCopy(struct bstrList *sl)
{
    int r;
    struct bstrList *new_sl = bstrVectorCreate(sl->qty);
    if (new_sl == NULL) {
        return NULL;
    }
    for (int i = 0; i < sl->qty; i++) {
        if (bstrVectorAdd(new_sl, bdata(sl->entry[i])) != BSTR_OK) {
            r = bstrListDestroy(new_sl);
            assert(r == BSTR_OK);
            return NULL;
        }
    }
    return new_sl;
}


/*
 * 0: sl1 and sl2 are same.
 * 1: sl1 and sl2 are different.
 */
int bstrVectorCompare(struct bstrList *sl1, struct bstrList *sl2)
{
    if (sl1->qty != sl2->qty) {
        return 1;
    }
    for (size_t i = 0; i < sl1->qty; i++) {
        bstring b1 = sl1->entry[i];
        bstring b2 = sl2->entry[i];
        if (bstrcmp(b1, b2)) {
            return 1;
        }
    }
    return 0;
}

static int
bstrVectorSortCommpare(const void *a, const void *b)
{
    return bstrcmp(*(const_bstring*)a, *(const_bstring*)b);
}

void bstrVectorSort(struct bstrList *sl)
{
    qsort(sl->entry, sl->qty, sizeof(*sl->entry), bstrVectorSortCommpare);
}

void bstrVectorUnique(struct bstrList *sl)
{
    for (int i = 0; i < sl->qty; i++) {
        for (int j = 0; j < sl->qty; j++) {
            if (i == j) {
                continue;
            }
            if (!bstrcmp(sl->entry[i], sl->entry[j])) {
                bdestroy(sl->entry[j]);
                sl->entry[j] = NULL;
            }
        }
    }
    int j = 0;
    for (int i = 0; i < sl->qty; i++) {
        if (sl->entry[i] == NULL) {
            continue;
        }
        if (j < i) {
            sl->entry[j] = sl->entry[i];
            sl->entry[i] = NULL;
        }
        j++;
    }
    sl->qty = j;
}

/* This function tests whether a substring exists in the set of strings.
 * returns BSTR_ERR if no such string exists,
 * otherwise some other values.
 */
int bstrVectorExistSubstring(struct bstrList *sl, bstring b)
{
    int v = BSTR_ERR;
    for (int i = 0; i < sl->qty; i++) {
        v = binstr(sl->entry[i], 0, b);
        if (v != BSTR_ERR) {
            break;
        }
    }
    return v;
}

void bstrVectorPrint(FILE *fp, struct bstrList *sl)
{
    for (int i = 0; i < sl->qty; i++) {
        fprintf(fp, "[%02d] %s\n", i, bdata(sl->entry[i]));
    }
}

/* This function creates a string of c characters of size n.
 */
bstring brepeat(int n, char c)
{
    bstring b = bfromcstralloc(n, "");
    for (int i = 0; i < n; i++) {
        bconchar(b, c);
    }
    return b;
}