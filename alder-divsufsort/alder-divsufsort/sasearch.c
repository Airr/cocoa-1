/**
 * This file, sasearch.c, is part of alder-divsufsort.
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
#include "divsufsort64.h"
#include "lfs.h"


static
void
print_help(const char *progname, int status) {
    fprintf(stderr,
            "sasearch, a simple SA-based full-text search tool, version %s\n",
            divsufsort64_version());
    fprintf(stderr, "usage: %s PATTERN FILE SAFILE\n\n", progname);
    exit(status);
}

int
main(int argc, const char *argv[]) {
    FILE *fp;
    const char *P;
    sauchar_t *T;
    saidx64_t *SA;
    LFS_OFF_T n;
    size_t Psize;
    saidx64_t i, size, left;
    
    if((argc == 1) ||
       (strcmp(argv[1], "-h") == 0) ||
       (strcmp(argv[1], "--help") == 0)) { print_help(argv[0], EXIT_SUCCESS); }
    if(argc != 4) { print_help(argv[0], EXIT_FAILURE); }
    
    P = argv[1];
    Psize = strlen(P);
    
    /* Open a file for reading. */
    if((fp = LFS_FOPEN(argv[2], "rb")) == NULL) {
        fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[2]);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    /* Get the file size. */
    if(LFS_FSEEK(fp, 0, SEEK_END) == 0) {
        n = LFS_FTELL(fp);
        rewind(fp);
        if(n < 0) {
            fprintf(stderr, "%s: Cannot ftell `%s': ", argv[0], argv[2]);
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "%s: Cannot fseek `%s': ", argv[0], argv[2]);
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
                argv[2]);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    
    /* Open the SA file for reading. */
    if((fp = LFS_FOPEN(argv[3], "rb")) == NULL) {
        fprintf(stderr, "%s: Cannot open file `%s': ", argv[0], argv[3]);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    /* Read n * sizeof(saidx64_t) bytes of data. */
    if(fread(SA, sizeof(saidx64_t), (size_t)n, fp) != (size_t)n) {
        fprintf(stderr, "%s: %s `%s': ",
                argv[0],
                (ferror(fp) || !feof(fp)) ? "Cannot read from" : "Unexpected EOF in",
                argv[3]);
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    
    /* Search and print */
    size = sa_search64(T, (saidx64_t)n,
                     (const sauchar_t *)P, (saidx64_t)Psize,
                     SA, (saidx64_t)n, &left);
    for(i = 0; i < size; ++i) {
        fprintf(stdout, "%lld" PRIdSAIDX64_T "\n", SA[left + i]);
    }
    
    /* Deallocate memory. */
    free(SA);
    free(T);
    
    return 0;
}
