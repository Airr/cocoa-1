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
#include <assert.h>
#include "bstrmore.h"

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
 */
struct bstrList * bstrVectorCreate(int n)
{
    struct bstrList *sl = bstrListCreate();
    int s = bstrListAlloc(sl, n);
    assert(s == BSTR_OK);
    memset(sl->entry, 0, sl->mlen * sizeof(bstring));
    return sl;
}

void bstrVectorAdd(struct bstrList *sl, const char *s)
{
    bstring b = bfromcstr(s);
    sl->entry[sl->qty] = b;
    sl->qty++;
    return;
}

void bstrVectorDelete(struct bstrList *sl)
{
    bstrListDestroy(sl);
    return;
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
