//
//  alder_file_isgzip.c
//  alder-adapter
//
//  Created by Sang Chul Choi on 8/31/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "alder_file_isgzip.h"

// Returns
// -1 : could not open the file fn.
//  1 : file fn is a gzip file.
//  0 : file fn is not a gzip file.
//  0 : file fn is nil.
int alder_file_isgzip(const char *fn)
{
    if (fn == NULL) {
        return 0;
    }
    FILE *fp = fopen(fn, "rb");
    if (fp == NULL) {
        return -1;
    }
    unsigned char c1, c2;
    size_t n = fread (&c1, sizeof (char), 1, fp);
    if (n != 1) return -1;
    n = fread (&c2, sizeof (char), 1, fp);
    if (n != 1) return -1;
    fclose(fp);
    
    if (c1 == 0x1f && c2 == 0x8b) {
        return 1;
    } else {
        return 0;
    }
}
