/**
 * This file, mksary.c, is part of alder-divsufsort.
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

#include <stdio.h>

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
void
print_help(const char *progname, int status) {
    fprintf(stderr,
            "mksary, a simple suffix array builder, version %s.\n",
            divsufsort64_version());
    fprintf(stderr, "usage: %s INFILE OUTFILE\n\n", progname);
    exit(status);
}


int
main(int argc, const char *argv[]) {
    FILE *fp, *ofp;
    const char *fname, *ofname;
    sauchar_t *T;
    saidx64_t *SA;
    LFS_OFF_T n;
    clock_t start, finish;
    saint_t needclose = 3;
    
    /* Check arguments. */
    if((argc == 1) ||
       (strcmp(argv[1], "-h") == 0) ||
       (strcmp(argv[1], "--help") == 0)) { print_help(argv[0], EXIT_SUCCESS); }
    if(argc != 3) { print_help(argv[0], EXIT_FAILURE); }
    
    /* Open a file for reading. */
    if(strcmp(argv[1], "-") != 0) {
        if((fp = LFS_FOPEN(fname = argv[1], "rb")) == NULL) {
//        if(fopen_s(&fp, fname = argv[1], "rb") != 0) {
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
//        if(fopen_s(&ofp, ofname = argv[2], "wb") != 0) {
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
        if(0x7fffffff <= n) {
            fprintf(stderr, "%s: Input file `%s' is too big.\n", argv[0], fname);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "%s: Cannot fseek `%s': ", argv[0], fname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    /* Allocate 5blocksize bytes of memory. */
    T = (sauchar_t *)malloc((size_t)n * sizeof(sauchar_t));
    SA = (saidx64_t *)malloc((size_t)n * sizeof(saidx64_t));
    if((T == NULL) || (SA == NULL)) {
        fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /* Read n bytes of data. */
    if(fread(T, sizeof(sauchar_t), (size_t)n, fp) != (size_t)n) {
        fprintf(stderr, "%s: %s `%s': ",
                argv[0],
                (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
                fname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    if(needclose & 1) { fclose(fp); }
    
    /* Construct the suffix array. */
    fprintf(stderr, "%s: %lld" PRIdOFF_T " bytes ... ", fname, n);
    start = clock();
    if(divsufsort64(T, SA, (saidx64_t)n) != 0) {
        fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    finish = clock();
    fprintf(stderr, "%.4f sec\n", (double)(finish - start) / (double)CLOCKS_PER_SEC);
    
    /* Write the suffix array. */
    if(fwrite(SA, sizeof(saidx64_t), (size_t)n, ofp) != (size_t)n) {
        fprintf(stderr, "%s: Cannot write to `%s': ", argv[0], ofname);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    if(needclose & 2) { fclose(ofp); }
    
    /* Deallocate memory. */
    free(SA);
    free(T);
    
    return 0;
}

