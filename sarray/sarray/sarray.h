//
//  sarray.h
//  sarray
//
//  Created by Sang Chul Choi on 7/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef sarray_sarray_h
#define sarray_sarray_h

typedef unsigned char uchar;

int ssarray(int *a);
int sarray(int *a, int n);
int bsarray(const uchar *b, int *a, int n);
int *lcp(const int *a, const char *s, int n);
int lcpa(const int *a, const char *s, int *b, int n);
int *scode(const char *s);
uchar *codetab(const uchar *s);
uchar *inverse(const uchar *t);

#endif
