/**
 * This file, bstrmore.h, is part of alder-adapter.
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

#ifndef bstring_bstrmore_h
#define bstring_bstrmore_h

#include "bstrlib.h"

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

/* Delete l characters from the right end of the string. */
#define brtrim(b,n)                  ((btrunc)((b), blength((b))-(n)))
/* Delete l characters from the left end of the string. */
#define bltrim(b,n)                  ((bdelete)((b), 0, (n)))

int bltrimc (bstring b, char c);
int brtrimc (bstring b, char c);
int btrimc (bstring b, char c);
int bltrimc2 (bstring b, bstring b2, char c);
int brtrimc2 (bstring b, bstring b2, char c);
int btrimc2 (bstring b, bstring b2, char c);

int bltrimq (bstring b, int c);
int brtrimq (bstring b, int c);
int btrimq (bstring b, int c);
int bltrimq2 (bstring b, bstring b2, int c);
int brtrimq2 (bstring b, bstring b2, int c);
int btrimq2 (bstring b, bstring b2, int c);

int bcount (bstring b, char c);
int bcountq (bstring b, int c);


bstring brepeat(int n, char c);
struct bstrList * bstrVectorCreate(int n);
int bstrVectorAdd(struct bstrList *sl, const char *s);
void bstrVectorAddBstring(struct bstrList *sl, bstring b);
int bstrVectorDelete(struct bstrList *sl);
struct bstrList * bstrVectorCopy(struct bstrList *sl);
int bstrVectorCompare(struct bstrList *sl1, struct bstrList *sl2);
void bstrVectorSort(struct bstrList *sl);
void bstrVectorUnique(struct bstrList *sl);
int bstrVectorExistSubstring(struct bstrList *sl, bstring b);
void bstrVectorPrint(FILE *fp, struct bstrList *sl);

#pragma mark view

struct bstrList * bstrViewCreate();
int bstrViewAdd(struct bstrList *sl, bstring b);
int bstrViewDestroy(struct bstrList *sl);

__END_DECLS

#endif
