/**
 * This file, alder_file_io.c, is part of alder-file.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_file_io.h"

/* This function tests functions for writing and reading KFM binary files.
 *
 * Pos(hex) Size Description
 * Note that all values are little-endian.
 * +00 8B Magic number \211 K F M \r \n \032 \n
 * +08 1B File version number (currently 1)
 * +09 7B (pad byte with zeros)
 * +10 25B http://www.alder.org/KFM/
 * +29 7B (pad byte with zeros)
 * +30 DATA
 *
 * (4B)iKFM
 * (1B)type
 * (3B)zeros
 * (8B)size or number of 64bit numbers (in little endian)
 * 64bit numbers = size x 8B
 *
 * (4B)iRHO
 * (1B)type
 * (3B)zeros
 * (8B)size
 * 64bit numbers = size x 8B
 * Compute 1 + (size - 1)/64 = sizeK
 * 64bit numbers for increments = sizeK x 8B
 * 64bit numbers for stored     = sizeK x 8B
 * Total (size + sizeK * 2) x 8B are data
 */
void
alder_file_io_test()
{
    FILE *fp = fopen("byte.adr", "wb");
    
    // Magic number (8B): \211 K F M \r \n \032 \n
    fputc('\211', fp);
    fputc('K', fp);
    fputc('F', fp);
    fputc('M', fp);
    fputc('\r', fp);
    fputc('\n', fp);
    fputc('\032', fp);
    fputc('\n', fp);
    // Version number (1B): 1
    fputc(0x01, fp);
    // Pad bytes with zeros.
    for (size_t i = 0; i < 7; i++) {
        fputc(0, fp);
    }
    // Location of spec http://www.alder.org/KFM/ (42 - 17 = 25)
    fprintf(fp, "http://www.alder.org/KFM/");
    // Pad bytes with zeros.
    for (size_t i = 0; i < 7; i++) {
        fputc(0, fp);
    }
    // Data chunk: KFMindex type iKFM
    fprintf(fp, "iKFM");
    // iKFM type (1B): 1
    fputc(0x01, fp);
    // Pad bytes with zeros.
    for (size_t i = 0; i < 3; i++) fputc(0, fp);
    size_t s = 5;
    fwrite(&s, sizeof(s), 1, fp);
    for (size_t i = 0; i < s; i++) {
        uint64_t x = 100 * i + 1;
        fwrite(&x, sizeof(x), 1, fp);
    }
    
    // Data chunk: rho type iRHO
    fprintf(fp, "iRHO");
    fputc(0x01, fp);
    // Pad bytes with zeros.
    for (size_t i = 0; i < 3; i++) fputc(0, fp);
    s = 5;
    fwrite(&s, sizeof(s), 1, fp);
    for (size_t i = 0; i < s; i++) {
        uint64_t x = 100 * i + 1;
        fwrite(&x, sizeof(x), 1, fp);
    }
    
    
    fclose(fp);
    
    fp = fopen("byte.adr", "rb");
    for (size_t i = 0; i < 8; i++) {
        int c = fgetc(fp); printf("%02X ", c);
    }
    printf("\n");
    int c = fgetc(fp);
    printf("File version: %d\n", c);
    // Eat off 7 zeros.
    for (size_t i = 0; i < 7; i++) fgetc(fp);
    bstring bhttp = bfromcstralloc(25+1, "");
    for (size_t i = 0; i < 25; i++) {
        c = fgetc(fp);
        bconchar(bhttp, c);
    }
    printf("WWW: %s\n", bdata(bhttp));
    // Eat off 7 zeros.
    for (size_t i = 0; i < 7; i++) fgetc(fp);
    
    
    fclose(fp);
    
}


void alder_file_io_test01()
{
    
    
}