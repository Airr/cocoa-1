/**
 * This file, alder_fasta_chunk.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <zlib.h>
#include <fcntl.h>
#include <stdint.h>
#include "bzlib.h"
#include "bstrlib.h"
#include "alder_logger.h"
#include "alder_cmacro.h"
#include "alder_fasta_chunk.h"

void
alder_fasta_chunk_test(char **inputs, unsigned int inputs_num)
{
    int s = 0;
    size_t sizeBuf = 10000; //15;
    size_t sizeBuf2 = 1000; //14;
    size_t kmer_size = 32;  //5;
    size_t lenBuf = sizeBuf;
    size_t lenBuf2 = sizeBuf2;
    char *buf = malloc(sizeof(*buf) * sizeBuf);
    memset(buf,0,sizeof(*buf) * sizeBuf);
    char *buf2 = malloc(sizeof(*buf2) * sizeBuf2);
    memset(buf2,0,sizeof(*buf2) * sizeBuf2);
    for (unsigned int i = 0; i < inputs_num; i++) {
        FILE *fp = fopen(inputs[i], "r");
        lenBuf2 = 0;
        s = alder_fasta_chunk2(&lenBuf, buf, sizeBuf,
                              &lenBuf2, buf2, sizeBuf2,
                              kmer_size - 1, fp);
#if !defined(NDEBUG)
        if (lenBuf > 0) {
            bstring bbuf = blk2bstr(buf, (int)lenBuf);
            bstring bbuf2 = blk2bstr(buf2, (int)lenBuf2);
            alder_log4("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
            alder_log5("buf2 (%02d): %s", blength(bbuf2), bdata(bbuf2));
            bdestroy(bbuf);
            bdestroy(bbuf2);
        }
#endif
        
        while (s == ALDER_STATUS_SUCCESS && lenBuf > 0) {
            s = alder_fasta_chunk2(&lenBuf, buf, sizeBuf,
                                  &lenBuf2, buf2, sizeBuf2,
                                  kmer_size - 1, fp);
#if !defined(NDEBUG)
            if (s == ALDER_STATUS_SUCCESS && lenBuf > 0) {
                bstring bbuf = blk2bstr(buf, (int)lenBuf);
                bstring bbuf2 = blk2bstr(buf2, (int)lenBuf2);
                alder_log4("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
                alder_log5("buf2 (%02d): %s", blength(bbuf2), bdata(bbuf2));
                bdestroy(bbuf);
                bdestroy(bbuf2);
            }
#endif
        }
        
        XFCLOSE(fp);
    }
    XFREE(buf);
    XFREE(buf2);
}


/* This function returns a chunk of data from a file (fp).
 * returns
 * SUCCESS if there are more data to read from the file.
 * ERROR otherwise.
 *
 * RECOMMENDED: sizeBuf > 10000
 *              sizebuf2 >= 1000
 * NOTE: sizeTailbuf <= sizeBuf2
 *       sizeBuf2 < sizeBuf
 *
 * given
 * lenBuf: for returning the length of relevant content in `buf'
 * buf: the main buffer for the data
 * sizeBuf: capacity of buf
 * lenBuf2: length of content in buf2. The content will be copied to buf.
 * buf2: the secondary buffer for the data.
 * sizeBuf2: capacity of buf2. sizeBuf2 is smaller than sizeBuf
 * sizeTailbuf: if the tail of buf does not contain '<', these sizeTailbuf
 *              characters in the tail of buf would be copied to buf2.
 * fp: file pointer
 *
 * 1. copy buf2 to buf, and additionally read a chunk to buf. The
 *    total buffer size is the sum of the size of the secondary buffer and
 *    the size of the main. If the secondary buffer buf2 has any thing,
 *    I'd copy buf2 to buf, and read the remaining part from the input file.
 *    This way, the total size of the main buffer is less than or equal to
 *    its capacity, sizeBuf.
 *    I'd return the function with empty buffer when there is no content to 
 *    read from the file.
 * 2. search the main buffer for '>'. The search range is either the size of
 *    the secondary buffer or the main buffer size, whichever is smaller.
 * 3. return the function with just before '>' if '>' is found.
 *    When returning, copy buf2 in the front of the buf.
 * 4. Otherwise, save K-1 char in buf2, and return the function.
 */
int
alder_fasta_chunk(size_t *lenBuf, char *buf, size_t sizeBuf,
                  size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                  size_t sizeTailbuf, void *fx, int gz_flag)
{
    assert(sizeBuf2 < sizeBuf);
    assert(sizeTailbuf <= sizeBuf2);
    assert(*lenBuf2 <= sizeBuf2);
    char *startBuf = buf + (*lenBuf2);
    // 1
    if (*lenBuf2 > 0) {
        memcpy(buf, buf2, sizeof(*buf) * (*lenBuf2));
        alder_loga4("buf0", (uint8_t*)buf, (*lenBuf2));
    }
    
    int lenRead;
    if (gz_flag == 1) {
        lenRead = gzread((gzFile)fx, startBuf,
                         (unsigned)(sizeof(*buf)*(sizeBuf - *lenBuf2)));
    } else if (gz_flag == 2) {
        lenRead = BZ2_bzread((BZFILE*)fx, startBuf,
                             (unsigned)(sizeof(*buf)*(sizeBuf - *lenBuf2)));
    } else {
        lenRead = (int) read((int)((intptr_t)fx), startBuf,
                             sizeof(*buf)*(sizeBuf - *lenBuf2));
    }
    
    if (lenRead == 0) {
        // check if this is error or end of file
        if (*lenBuf2 > 0) {
            *lenBuf = *lenBuf2;
            *lenBuf2 = 0;
            return ALDER_STATUS_SUCCESS;
        } else {
//            if (feof(fp)) {
                alder_log3("End of File!");
//            } else if (ferror(fp)) {
//                alder_loge(1,"Error - fread fasta!");
//            }
            *lenBuf = 0;
            *lenBuf2 = 0;
            return ALDER_STATUS_ERROR;
        }
    }
    alder_loga4("buf1", (uint8_t*)startBuf, lenRead);
    // 2
    bool foundAngle = false;
    size_t atAngle = 0;
    for (atAngle = 0;
         atAngle < sizeBuf2 && atAngle < sizeBuf;
         atAngle++) {
        if (buf[*lenBuf2 + lenRead - 1 - atAngle] == '>') {
            foundAngle = true;
            break;
        }
    }
    if (atAngle == *lenBuf2 + lenRead - 1) {
        alder_log5("First is '<': angle turned out to be not found!");
        foundAngle = false;
    }
    // 3
    if (foundAngle == true) {
        // Copy a tail of buf to buf2.
        *lenBuf = *lenBuf2 + lenRead - (atAngle + 1);
        *lenBuf2 = atAngle + 1;
        memcpy(buf2, buf + *lenBuf, *lenBuf2);
    } else {
        // Copy characters to buf2.
        size_t i = 0;
        size_t lenTail = 0;
        while (lenTail < sizeTailbuf) {
            int c = startBuf[lenRead - 1 - i];
            if (isalnum(c)) {
                buf2[sizeTailbuf - 1 - lenTail] = c;
                lenTail++;
            }
            i++;
        }
        assert(lenTail == sizeTailbuf);
        *lenBuf = *lenBuf2 + lenRead;
        *lenBuf2 = sizeTailbuf;
    }
    
//#if !defined(NDEBUG)
//    bstring logTitle = bformat("buf1 (%zu)", *lenBuf);
//    alder_loga5(bdata(logTitle), (uint8_t*)buf, *lenBuf);
//    btrunc(logTitle, 0);
//    bformata(logTitle, "buf2 (%zu)", *lenBuf2);
//    alder_loga5(bdata(logTitle), (uint8_t*)buf2, *lenBuf2);
//    bdestroy(logTitle);
//#endif
    
    assert(*lenBuf2 <= sizeBuf2);
    return ALDER_STATUS_SUCCESS;
}

int
alder_fasta_chunk2(size_t *lenBuf, char *buf, size_t sizeBuf,
                  size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                  size_t sizeTailbuf, FILE *fp)
{
    assert(sizeBuf2 < sizeBuf);
    assert(sizeTailbuf <= sizeBuf2);
    assert(*lenBuf2 <= sizeBuf2);
    char *startBuf = buf + (*lenBuf2);
    // 1
    if (*lenBuf2 > 0) {
        memcpy(buf, buf2, sizeof(*buf) * (*lenBuf2));
        alder_loga4("buf0", (uint8_t*)buf, (*lenBuf2));
    }
    size_t lenRead = fread(startBuf, sizeof(*buf), sizeBuf - *lenBuf2, fp);
    if (lenRead == 0) {
        // check if this is error or end of file
        if (*lenBuf2 > 0) {
            *lenBuf = *lenBuf2;
            *lenBuf2 = 0;
            return ALDER_STATUS_SUCCESS;
        } else {
            if (feof(fp)) {
                alder_log3("End of File!");
            } else if (ferror(fp)) {
                alder_loge(1,"Error - fread fasta!");
            }
            *lenBuf = 0;
            *lenBuf2 = 0;
            return ALDER_STATUS_ERROR;
        }
    }
    alder_loga4("buf1", (uint8_t*)startBuf, lenRead);
    // 2
    bool foundAngle = false;
    size_t atAngle = 0;
    for (atAngle = 0;
         atAngle < sizeBuf2 && atAngle < sizeBuf;
         atAngle++) {
        if (buf[*lenBuf2 + lenRead - 1 - atAngle] == '>') {
            foundAngle = true;
            break;
        }
    }
    if (atAngle == *lenBuf2 + lenRead - 1) {
        alder_log5("First is '<': angle turned out to be not found!");
        foundAngle = false;
    }
    // 3
    if (foundAngle == true) {
        // Copy a tail of buf to buf2.
        *lenBuf = *lenBuf2 + lenRead - (atAngle + 1);
        *lenBuf2 = atAngle + 1;
        memcpy(buf2, buf + *lenBuf, *lenBuf2);
    } else {
        // Copy characters to buf2.
        size_t i = 0;
        size_t lenTail = 0;
        while (lenTail < sizeTailbuf) {
            int c = startBuf[lenRead - 1 - i];
            if (isalnum(c)) {
                buf2[sizeTailbuf - 1 - lenTail] = c;
                lenTail++;
            }
            i++;
        }
        assert(lenTail == sizeTailbuf);
        *lenBuf = *lenBuf2 + lenRead;
        *lenBuf2 = sizeTailbuf;
    }
    
//#if !defined(NDEBUG)
//    bstring logTitle = bformat("buf1 (%zu)", *lenBuf);
//    alder_loga5(bdata(logTitle), (uint8_t*)buf, *lenBuf);
//    btrunc(logTitle, 0);
//    bformata(logTitle, "buf2 (%zu)", *lenBuf2);
//    alder_loga5(bdata(logTitle), (uint8_t*)buf2, *lenBuf2);
//    bdestroy(logTitle);
//#endif
    
    assert(*lenBuf2 <= sizeBuf2);
    return ALDER_STATUS_SUCCESS;
}