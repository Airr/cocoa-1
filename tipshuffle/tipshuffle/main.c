//
//  main.c
//  tipshuffle
//
//  Created by Sang Chul Choi on 6/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// gengetopt generates this header file using tipshuffle.ggo.
#include "cmdline.h"
// Lehmer random number generator
#include "rngs.h"
#include "rvgs.h"
// Yu-wei Wu's Newick tree parser
#include "seqUtil.h"
#include "Newickform.h"

#define STR_OUT	"out"

int main(int argc, char *argv[])
{
    size_t i;
    
    // Parse the command line.
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0)
        exit(1) ;
    
    if (args_info.help_given)
    {
        my_cmdline_parser_print_help ();
    }
    
    if (args_info.version_given)
    {
        my_cmdline_parser_print_version ();
    }
    
    if (args_info.inputs_num < 1)
    {
        fprintf(stderr, "Error: no input file!\n");
        my_cmdline_parser_free (&args_info);
        exit(1);
    }
    
    // Seed random number generator.
    if (args_info.int_opt_given)
    {
        PlantSeeds(args_info.int_opt_arg);
    }
    else
    {
        PlantSeeds(-1);
    }
    // SelectStream(0); This causes the first value is always the same.
    SelectStream(1);
    
    // Newick tree parser.
    seqMemInit();
    
    // Read lines from a file and count characters including the end of line.
    FILE *fp;
    fp = fopen(args_info.inputs[0], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: no such file - %s\n", args_info.inputs[0]);
    }
    typedef enum {FILE_EOF,FILE_READING} fileStatus_t;
    fileStatus_t s = FILE_READING;
    int c = fgetc(fp);
    if (c == EOF)
    {
        fclose(fp);
        my_cmdline_parser_free (&args_info);
        fprintf(stderr, "Error: nothing in the file - %s\n", args_info.inputs[0]);
        exit(1);
    }
    ungetc(c, fp);
    while (s == FILE_READING) {
        size_t alloced = 3;
        char *p = malloc(alloced * sizeof(char));
        char *walk = p;
        size_t to_read = 0;
        size_t lengthOfBuffer = 2 + 1;
        
        while (1) {
            if (fgets(walk, (int)lengthOfBuffer, fp) == NULL) {
                s = FILE_EOF;
                break;
            }
            size_t len = strlen(walk);
            if (walk[len-1] == '\n') {
                walk[len-1] = '\0';
                break;
            }
            to_read += lengthOfBuffer - 1;
            alloced += lengthOfBuffer - 1;
            p = realloc(p, alloced * sizeof(char));
            walk = p + to_read;
        }

        // Shuffle the Newick tree string.
        if (s == FILE_READING)
        {
            // Parse tree string
            newick_node *root;
//            fprintf(stdout, "R: %s %d\n", p, strlen(p));
            char *pcTreeStr = NULL;
            pcTreeStr = (char*)seqMalloc((int)(strlen(p) + 1)*sizeof(char));
            memcpy(pcTreeStr, p, strlen(p));
            
//            root = parseTree(p);
            root = parseTree(pcTreeStr);
            size_t numberOfTip = leafCount(root);
            
            // Permute an array of size being the number of leaves in the tree.
            size_t *a = (size_t *)malloc(numberOfTip * sizeof(size_t));
            size_t *b = (size_t *)malloc(numberOfTip * sizeof(size_t));
//            size_t b[10000];
            for (i = 0; i < numberOfTip; i++) {
                a[i] = i;
                b[i] = numberOfTip;
            }
            for (i = 0; i < numberOfTip; i++) {
                long x = Equilikely(0L,numberOfTip - 1 - i);
//                if (i == 0) {
//                    printf("%d\n", (int)x);
//                }
                b[numberOfTip - 1 - i] = a[x];
                a[x] = a[numberOfTip - 1 - i];
            }
//            for (i = 0; i < numberOfTip; i++) {
//                fprintf(stdout, "%zu ", b[i]);
//            }
//            fprintf(stdout, "\n");
            // Now, b contains a permuted indices.
            shuffleTipOfTree(root, b);
            free(a);
            free(b);
            
            printTree(root);
            printf(";\n");
            seqFree(root);
            root = NULL;
            seqFree(pcTreeStr);
            pcTreeStr = NULL;
        }
        free(p);
        p = NULL;
    }
    
    fclose(fp);
    fp = NULL;
    
    my_cmdline_parser_free (&args_info);

    // End memory management procedure and free all allocated space
	seqFreeAll();

    return 0;
}


