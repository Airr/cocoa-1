//
//  main.c
//  peach-fasta-shortest-fragment
//
//  Created by Sang Chul Choi on 8/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <search.h>
#include "fasta.h"


uint64_t encodeDNA(const char* s, size_t size);
char* decodeDNA(char *s, uint64_t k, size_t size);
int compareCode(const void *a, const void *b)
{
    uint64_t *x = (uint64_t *) a;
    uint64_t *y = (uint64_t *) b;
    if (*x < *y)
        return -1;
    else if (*x > *y)
        return 1;
    return 0;
}


int main(int argc, const char * argv[])
{
    char stringFragment[100];
    const char* programName = argv[0];
    size_t sizeFragment = 0;
    uint64_t *array = NULL;
    size_t sizeArray = 0;
    unsigned int sizeSequence = 0;
    
    if (argc != 2 && argc != 3 && argc != 4)
    {
        fprintf(stderr, "%s [errro] - wrong argument\n", programName);
        fprintf(stderr, "Usage: %s fasta_file_name [length] [cat]\n", programName);
        fprintf(stderr, "  length (optional) - default 0\n");
        fprintf(stderr, "  cat (optional) - print all sequences in a line without space.\n");
        fprintf(stderr, "  length is required if cat is used.\n");
        exit(-1);
    }
    const char* fastaFilename = argv[1];
    if (argc == 3 || argc == 4)
    {
        sizeSequence = (unsigned int) atoi(argv[2]);
    }
    
    unsigned int ui1 = 0;
    unsigned int ui2 = 0;
    fasta_t genomeFasta;
    lengthSequenceFasta(fastaFilename, &ui1, &ui2);
    genomeFasta.numberOfSequence = ui1;
    genomeFasta.numberOfBase = ui2;
    genomeFasta.index = (unsigned int*)malloc(ui1*sizeof(unsigned int));
    genomeFasta.data = (char*)malloc((ui2 + 1)*sizeof(char));
    readFasta(fastaFilename, &genomeFasta);
    genomeFasta.data[genomeFasta.numberOfBase] = '\0';
    if (argc == 4)
    {
        for (unsigned int i = 0; i < genomeFasta.numberOfSequence; i++) {
            unsigned int begin = genomeFasta.index[i];
            unsigned int end;
            if (i == genomeFasta.numberOfSequence - 1)
            {
                end = genomeFasta.numberOfBase;
            }
            else
            {
                end = genomeFasta.index[i + 1];
            }
            size_t lengthOfSequence = end - begin;
            if (sizeSequence > 0 && lengthOfSequence > sizeSequence)
            {
                begin = end - sizeSequence;
            }
            for (size_t j = begin; j < end; j++)
            {
                fprintf(stdout, "%c", genomeFasta.data[j]);
            }
            fprintf(stdout, "\n");
        }
        fprintf(stderr, "We created a concatenated sequence file.\n");
            
        free(genomeFasta.index);
        free(genomeFasta.data);
        return 0;
    }
    if (argc == 3)
    {
        fprintf(stdout, "We are using the 3' %d bases.\n", sizeSequence);
        fprintf(stderr, "We are using the 3' %d bases.\n", sizeSequence);
    }
    else
    {
        fprintf(stdout, "We are using all of the bases.\n");
        fprintf(stderr, "We are using all of the bases.\n");
    }
    
//    unsigned int stepSize = genomeFasta.numberOfSequence / 10;
    
    uint64_t missingCode = 0;
    while (missingCode == 0) {
        int isFull = 0;
        sizeFragment++;
        sizeArray = 0;
        
        uint64_t maxCode = 1;
        for (size_t i = 0; i < sizeFragment; i++) {
            maxCode <<= 2;
        }
        
        uint64_t numberOfInvalidFragment = 0;
        for (unsigned int i = 0; i < genomeFasta.numberOfSequence; i++)
        {
//            fprintf(stderr, "%d\n", i);
//            if (i % stepSize == 0)
            double progress = 100.0 * (double)i / (double)genomeFasta.numberOfSequence;
            {
                fprintf(stderr, "\r[%zd] [", sizeFragment);
                for (double progressI = 0.0; progressI < progress; progressI += 1.0)
                {
                    fprintf(stderr, "-");
                }
                fprintf(stderr, "] %d %% ", (int)progress);
            }
            unsigned int begin = genomeFasta.index[i];
            unsigned int end;
            if (i == genomeFasta.numberOfSequence - 1)
            {
                end = genomeFasta.numberOfBase;
            }
            else
            {
                end = genomeFasta.index[i + 1];
            }
            
            size_t lengthOfSequence = end - begin;
            if (sizeSequence > 0 && lengthOfSequence > sizeSequence)
            {
                begin = end - sizeSequence;
            }
            size_t progressStatus = 0;
            for (size_t j = begin; j <= end - sizeFragment; j++)
            {
                strncpy(stringFragment, genomeFasta.data + j, sizeFragment);
                stringFragment[sizeFragment] = '\0';
                uint64_t encodedFragment = encodeDNA(stringFragment, sizeFragment);
                if (encodedFragment == UINT64_MAX)
                {
                    numberOfInvalidFragment++;
                    continue;
                }
                uint64_t *found = bsearch(&encodedFragment, array, sizeArray, sizeof(uint64_t), compareCode);
                if (found == NULL)
                {
                    sizeArray++;
                    uint64_t *tmp = realloc(array, sizeof(uint64_t) * sizeArray);
                    if (tmp == NULL) {
                        fprintf(stderr, "Error: memory\n");
                        abort();
                    }
                    array = tmp;
                    array[sizeArray - 1] = encodedFragment;
                    qsort(array, sizeArray, sizeof(uint64_t), compareCode);
                }
                if (sizeArray == maxCode)
                {
                    isFull = 1;
                    break;
                }
                
            if (j % 100 == 0)
            {
                progressStatus++;
                fprintf(stderr, "\r[%zd] [", sizeFragment);
                for (double progressI = 0.0; progressI < progress - 1.0; progressI += 1.0)
                {
                    fprintf(stderr, "-");
                }
                if (progressStatus % 4 == 0)
                {
                    fprintf(stderr, "-");
                }
                else if (progressStatus % 4 == 1)
                {
                    fprintf(stderr, "\\");
                }
                else if (progressStatus % 4 == 2)
                {
                    fprintf(stderr, "|");
                }
                else if (progressStatus % 4 == 3)
                {
                    fprintf(stderr, "/");
                }
                double progressCode = 100.0 * (double)sizeArray / (double)maxCode;
                fprintf(stderr, "] %d %% (Code: %d %%) ", (int)progress, (int)progressCode);
            }
                
                
            }
            if (isFull == 1)
            {
                fprintf(stderr, "\r[%zd] [", sizeFragment);
                for (double progressI = 0.0; progressI < progress; progressI += 1.0)
                {
                    fprintf(stderr, "-");
                }
                double progressCode = 100.0 * (double)sizeArray / (double)maxCode;
                fprintf(stderr, "] %d %% (Code: 100 %%) ", (int)progress);
                break;
            }
        }
        
        // Print items not in the array.
        if (isFull == 0) {
            for (uint64_t i = 0; i < maxCode; i++)
            {
                uint64_t *found = bsearch(&i, array, sizeArray, sizeof(uint64_t), compareCode);
                if (found == NULL)
                {
                    missingCode++;
                    decodeDNA(stringFragment, i, sizeFragment);
                    fprintf(stdout, "[%llu] %s\n", i, stringFragment);
                }
            }
            fprintf(stdout, "Number of missing fragments: %llu\n", missingCode);
        }
        else
        {
            fprintf(stderr, "All fragments were found!\n");
        }
        
        free(array);
        array = NULL;
    }
    
    free(genomeFasta.index);
    free(genomeFasta.data);
    return 0;
}

uint64_t encodeDNA(const char* s, size_t size)
{
    uint64_t k = 0;
    uint64_t b = 1;
    for (size_t i = 0; i < size; i++) {
        switch (s[i]) {
            case 'a':
            case 'A':
                k += b * 0;
                break;
            case 'c':
            case 'C':
                k += b * 1;
                break;
            case 'g':
            case 'G':
                k += b * 2;
                break;
            case 't':
            case 'T':
                k += b * 3;
                break;
            default:
                k = UINT64_MAX;
                break;
        }
        if (k == UINT64_MAX)
            break;
        b = b << 2;
    }
    return k;
}

char* decodeDNA(char *s, uint64_t k, size_t size)
{
    uint64_t b = 1;
    for (size_t i = 0; i < size - 1; i++) {
        b = b << 2;
    }
    
    uint64_t q = k;
    for (size_t i = 0; i < size; i++) {
        uint64_t r = q % b;
        q /= b;
        switch (q) {
            case 0:
                s[size - i - 1] = 'A';
                break;
            case 1:
                s[size - i - 1] = 'C';
                break;
            case 2:
                s[size - i - 1] = 'G';
                break;
            case 3:
                s[size - i - 1] = 'T';
                break;
            default:
                assert(0);
                break;
        }
        q = r;
        b >>= 2;
    }
    s[size] = '\0';
    return s;
}

