/**
 * This file, suftest.c, is part of alder-divsufsort.
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
void
print_help(const char *progname, int status) {
    fprintf(stderr,
            "suftest, a suffixsort tester, version %s.\n",
            divsufsort64_version());
    fprintf(stderr, "usage: %s FILE\n\n", progname);
    exit(status);
}

int
main(int argc, const char *argv[]) {
    FILE *fp;
    const char *fname;
    sauchar_t *T;
    saidx64_t *SA;
    LFS_OFF_T n;
    clock_t start, finish;
    saint_t needclose = 1;
    
    /* Check arguments. */
    if((argc == 1) ||
       (strcmp(argv[1], "-h") == 0) ||
       (strcmp(argv[1], "--help") == 0)) { print_help(argv[0], EXIT_SUCCESS); }
    if(argc != 2) { print_help(argv[0], EXIT_FAILURE); }
    
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
        needclose = 0;
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
    
    /* Allocate 5n bytes of memory. */
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
                argv[1]);
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
    
    /* Check the suffix array. */
    if(sufcheck64(T, SA, (saidx64_t)n, 1) != 0) { exit(EXIT_FAILURE); }
    
    /* Deallocate memory. */
    free(SA);
    free(T);
    
    return 0;
}
