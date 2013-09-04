//
//  bstrmore.h
//  bstring
//
//  Created by Sang Chul Choi on 9/1/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

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


struct bstrList * bstrVectorCreate(int n);
void bstrVectorAdd(struct bstrList *sl, const char *s);
void bstrVectorDelete(struct bstrList *sl);

int bstrVectorCompare(struct bstrList *sl1, struct bstrList *sl2);


__END_DECLS

#endif
