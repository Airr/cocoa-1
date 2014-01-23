//
//  main.c
//  alder-lz4
//
//  Created by Sang Chul Choi on 1/18/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "alder_lz4.h"

int main(int argc, const char * argv[])
{
    alder_lz4_test(argv[1]);
    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "lz4.h"

#define BLOCK_SIZE_MAX (1024 * 64)
static char in[BLOCK_SIZE_MAX];
static char out[LZ4_COMPRESSBOUND(BLOCK_SIZE_MAX)];
static char uncomp[BLOCK_SIZE_MAX];

int main01(int argc, const char * argv[])
{
    struct stat buf;
    FILE *inf;
    int rsize, fsize;
    int csize, dsize;
    void *lz4state;
    
    stat(argv[1], &buf);
    fsize = (int)buf.st_size;
    
    inf = fopen(argv[1], "r"); // dumpFile is a 512KB file
    if(NULL == inf)
    {
        fprintf(stderr, "open fail\n");
        return -1;
    }
    
    lz4state = malloc(LZ4_sizeofState());
    if(NULL == lz4state)
    {
        fclose(inf);
        return -1;
    }
    
    while((rsize = (int)fread(in, 1, BLOCK_SIZE_MAX, inf)) > 0)
    {
        csize = LZ4_compress_withState(lz4state, in, out, rsize);
        if(csize > 0)
        {
            //dsize = LZ4_decompress_fast();
            dsize = LZ4_decompress_safe(out, uncomp, csize, rsize);
            if(dsize < 0)
            {
                fprintf(stderr, "decompress fail, rsize=%d, csize=%d, dsize=%d\n",
                        rsize, csize, dsize);
                break;
            }
        }
        else
        {
            fprintf(stderr, "compress fail, rsize=%d, csize=%d\n",rsize, csize);
            break;
        }
    }
    
    fclose(inf);
    free(lz4state);
    
    return 0;
}