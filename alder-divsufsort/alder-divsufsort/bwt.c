/**
 * This file, bwt.c, is part of alder-divsufsort.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-divsufsort is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-divsufsort is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-divsufsort.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "alder_divsufsort.h"
#if HAVE_DIVSUFSORT_CONFIG_H
# include "divsufsort_config.h"
#endif
#include <stdio.h>
#if HAVE_STRING_H
# include <string.h>
#endif
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#if HAVE_MEMORY_H
# include <memory.h>
#endif
#if HAVE_STDDEF_H
# include <stddef.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_IO_H && HAVE_FCNTL_H
# include <io.h>
# include <fcntl.h>
#endif
#include <time.h>
#include "divsufsort64.h"
#include "lfs.h"


static
size_t
write_int(FILE *fp, saidx64_t n) {
    unsigned char c[4];
    c[0] = (unsigned char)((n >>  0) & 0xff), c[1] = (unsigned char)((n >>  8) & 0xff),
    c[2] = (unsigned char)((n >> 16) & 0xff), c[3] = (unsigned char)((n >> 24) & 0xff);
    return fwrite(c, sizeof(unsigned char), 4, fp);
}

static
void
print_help(const char *progname, int status) {
    fprintf(stderr,
            "bwt, a burrows-wheeler transform program, version %s.\n",
            divsufsort64_version());
    fprintf(stderr, "usage: %s [-b num] INFILE OUTFILE\n", progname);
    fprintf(stderr, "  -b num    set block size to num MiB [1..512] (default: 32)\n\n");
    exit(status);
}

int
main(int argc, const char *argv[]) {
    FILE *fp, *ofp;
    const char *fname, *ofname;
    sauchar_t *T;
    saidx64_t *SA;
    LFS_OFF_T n;
    size_t m;
    saidx64_t pidx;
    clock_t start,finish;
    saint_t i, blocksize = 32, needclose = 3;
    
    /* Check arguments. */
    if((argc == 1) ||
       (strcmp(argv[1], "-h") == 0) ||
       (strcmp(argv[1], "--help") == 0)) { print_help(argv[0], EXIT_SUCCESS); }
    if((argc != 3) && (argc != 5)) { print_help(argv[0], EXIT_FAILURE); }
    i = 1;
    if(argc == 5) {
        if(strcmp(argv[i], "-b") != 0) { print_help(argv[0], EXIT_FAILURE); }
        blocksize = atoi(argv[i + 1]);
        if(blocksize < 0) { blocksize = 1; }
        else if(512 < blocksize) { blocksize = 512; }
        i += 2;
    }
    blocksize <<= 20;
    
    /* Open a file for reading. */
    if(strcmp(argv[i], "-") != 0) {
        if((fp = LFS_FOPEN(fname = argv[i], "rb")) == NULL) {
            fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], fname);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    } else {
#if HAVE__SETMODE && HAVE__FILENO
        if(_setmode(_fileno(stdin), _O_BINARY) == -1) {
            fprintf(stderr, "%s: Cannot set mode: ", argv[0]);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
#endif
        fp = stdin;
        fname = "stdin";
        needclose ^= 1;
    }
    i += 1;
    
    /* Open a file for writing. */
    if(strcmp(argv[i], "-") != 0) {
        if((ofp = LFS_FOPEN(ofname = argv[i], "wb")) == NULL) {
            fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], ofname);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    } else {
#if HAVE__SETMODE && HAVE__FILENO
        if(_setmode(_fileno(stdout), _O_BINARY) == -1) {
            fprintf(stderr, "%s: Cannot set mode: ", argv[0]);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
#endif
        ofp = stdout;
        ofname = "stdout";
        needclose ^= 2;
    }
    
    /* Get the file size. */
    if(LFS_FSEEK(fp, 0, SEEK_END) == 0) {
        n = LFS_FTELL(fp);
        rewind(fp);
        if(n < 0) {
            fprintf(stderr, "%s: Cannot ftell `%s': ", argv[0], fname);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
        if(0x20000000L < n) { n = 0x20000000L; }
        if((blocksize == 0) || (n < blocksize)) { blocksize = (saint_t)n; }
    } else if(blocksize == 0) { blocksize = 32 << 20; }
    
    /* Allocate 5blocksize bytes of memory. */
    T = (sauchar_t *)malloc(blocksize * sizeof(sauchar_t));
    SA = (saidx64_t *)malloc(blocksize * sizeof(saidx64_t));
    if((T == NULL) || (SA == NULL)) {
        fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /* Write the blocksize. */
    if(write_int(ofp, blocksize) != 4) {
        fprintf(stderr, "%s: Cannot write to `%s': ", argv[0], ofname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, "  BWT (blocksize %" PRIdSAINT_T ") ... ", blocksize);
    start = clock();
    for(n = 0; 0 < (m = fread(T, sizeof(sauchar_t), blocksize, fp)); n += m) {
        /* Burrows-Wheeler Transform. */
        pidx = divbwt64(T, T, SA, m);
        if(pidx < 0) {
            fprintf(stderr, "%s (bw_transform): %s.\n",
                    argv[0],
                    (pidx == -1) ? "Invalid arguments" : "Cannot allocate memory");
            exit(EXIT_FAILURE);
        }
        
        /* Write the bwted data. */
        if((write_int(ofp, pidx) != 4) ||
           (fwrite(T, sizeof(sauchar_t), m, ofp) != m)) {
            fprintf(stderr, "%s: Cannot write to `%s': ", argv[0], ofname);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    }
    if(ferror(fp)) {
        fprintf(stderr, "%s: Cannot read from `%s': ", argv[0], fname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    finish = clock();
    fprintf(stderr, "%lld" PRIdOFF_T " bytes: %.4f sec\n",
            n, (double)(finish - start) / (double)CLOCKS_PER_SEC);
    
    /* Close files */
    if(needclose & 1) { fclose(fp); }
    if(needclose & 2) { fclose(ofp); }
    
    /* Deallocate memory. */
    free(SA);
    free(T);
    
    return 0;
}
