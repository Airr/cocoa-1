//
//  mktemp.c
//  deseq
//
//  Created by Sang Chul Choi on 7/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// MacOSX
#include <unistd.h>

int peach_mktemp()
{
    char *tmpdir;
    char *template = malloc (100 * sizeof(char));
    strcpy(template, "/tmp/deseq.XXXXXX");
    tmpdir = mkdtemp(template);
    printf ("Temporary directory created : %s\n", tmpdir);
    free(template);
    return 0;
}

int peach_remove()
{
    if (remove("/tmp/deseq.PEh6hk") != 0)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    else
    {
        printf("Temporary directory deleted : deseq.PEh6hk\n");
    }
    return 0;
}