/**
 * This file, unbwt.c, is part of alder-divsufsort.
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
read_int(FILE *fp, saidx64_t *n) {
    unsigned char c[4];
    size_t m = fread(c, sizeof(unsigned char), 4, fp);
    if(m == 4) {
        *n = (c[0] <<  0) | (c[1] <<  8) |
        (c[2] << 16) | (c[3] << 24);
    }
    return m;
}

static
void
print_help(const char *progname, int status) {
    fprintf(stderr,
            "unbwt, an inverse burrows-wheeler transform program, version %s.\n",
            divsufsort64_version());
    fprintf(stderr, "usage: %s INFILE OUTFILE\n\n", progname);
    exit(status);
}

int
main(int argc, const char *argv[]) {
    FILE *fp, *ofp;
    const char *fname, *ofname;
    sauchar_t *T;
    saidx64_t *A;
    LFS_OFF_T n;
    size_t m;
    saidx64_t pidx;
    clock_t start, finish;
//    saint_t err, blocksize, needclose = 3;
    saidx64_t err, blocksize, needclose = 3;
    
    /* Check arguments. */
    if((argc == 1) ||
       (strcmp(argv[1], "-h") == 0) ||
       (strcmp(argv[1], "--help") == 0)) { print_help(argv[0], EXIT_SUCCESS); }
    if(argc != 3) { print_help(argv[0], EXIT_FAILURE); }
    
    /* Open a file for reading. */
    if(strcmp(argv[1], "-") != 0) {
        if((fp = LFS_FOPEN(fname = argv[1], "rb")) == NULL) {
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
    
    /* Open a file for writing. */
    if(strcmp(argv[2], "-") != 0) {
        if((ofp = LFS_FOPEN(ofname = argv[2], "wb")) == NULL) {
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
    
    /* Read the blocksize. */
    if(read_int(fp, &blocksize) != 4) {
        fprintf(stderr, "%s: Cannot read from `%s': ", argv[0], fname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    /* Allocate 5blocksize bytes of memory. */
    T = (sauchar_t *)malloc(blocksize * sizeof(sauchar_t));
    A = (saidx64_t *)malloc(blocksize * sizeof(saidx64_t));
    if((T == NULL) || (A == NULL)) {
        fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    fprintf(stderr, "UnBWT (blocksize %lld" PRIdSAINT_T ") ... ", blocksize);
    start = clock();
    for(n = 0; (m = read_int(fp, &pidx)) != 0; n += m) {
        /* Read blocksize bytes of data. */
        if((m != 4) || ((m = fread(T, sizeof(sauchar_t), blocksize, fp)) == 0)) {
            fprintf(stderr, "%s: %s `%s': ",
                    argv[0],
                    (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
                    fname);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
        
        /* Inverse Burrows-Wheeler Transform. */
        if((err = inverse_bw_transform64(T, T, A, m, pidx)) != 0) {
            fprintf(stderr, "%s (reverseBWT): %s.\n",
                    argv[0],
                    (err == -1) ? "Invalid data" : "Cannot allocate memory");
            exit(EXIT_FAILURE);
        }
        
        /* Write m bytes of data. */
        if(fwrite(T, sizeof(sauchar_t), m, ofp) != m) {
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
    free(A);
    free(T);
    
    return 0;
}
