/**
 * This file, alder_fastq_chunk.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
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
#include "bstrlib.h"
#include "alder_logger.h"
#include "alder_cmacro.h"
#include "alder_fastq_chunk.h"

void
alder_fastq_chunk_test(char **inputs, unsigned int inputs_num)
{
    int s = 0;
    size_t sizeBuf = 50 * 2;
    size_t sizeBuf2 = 10 * 5;
    size_t lenBuf = sizeBuf;
    size_t lenBuf2 = sizeBuf2;
    char *buf = malloc(sizeof(*buf) * sizeBuf);
    memset(buf,0,sizeof(*buf) * sizeBuf);
    char *buf2 = malloc(sizeof(*buf2) * sizeBuf2);
    memset(buf2,0,sizeof(*buf2) * sizeBuf2);
    for (unsigned int i = 0; i < inputs_num; i++) {
        FILE *fp = fopen(inputs[i], "r");
        lenBuf2 = 0;
        s = alder_fastq_chunk2(&lenBuf, buf, sizeBuf,
                              &lenBuf2, buf2, sizeBuf2,
                              fp);
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
        
        while (s == ALDER_STATUS_SUCCESS) {
            s = alder_fastq_chunk2(&lenBuf, buf, sizeBuf,
                                  &lenBuf2, buf2, sizeBuf2,
                                  fp);
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
 * RECOMMENDED: sizeBuf > 10 x average read buffer size
 *              sizebuf2 >= 5 x average read buffers size
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
 * 2. search the main buffer for a second '@'. The search range is either the 
 *    size of the secondary buffer or the main buffer size, whichever is 
 *    smaller. 
 * 3  From there, I'd check if it is a valid read in FASTQ format. A valid
 *    read should have the following properties:
 *    @ sign at the start of the 1st line.
 *    ACGTN at the 2nd line.
 *    + sign at the start of the 3rd line.
 *    quality score at the 4th line.
 *    @ sign again after the 4th line.
 *    The 2nd and 4th lines must be of the same length.
 *
 * 4. If the block is not a vaild FASTQ read, then I'd have to find a third '@'
 *    and repeat the procedure of checking whether a block is a valid FASTQ
 *    read. Repeat these until I find one. If I did not find one, there is
 *    some problem in the FASTQ file.
 
 
 * 5. return the function with just before '@' if '@' is found.
 *    When returning, copy buf2 in the front of the buf.
 */

static int
alder_fastq_chunk_check_read(size_t *lenBuf, char *buf, size_t sizeBuf,
                             bool finalBlock)
{
    int nFoundAtsign;
    if (finalBlock == false) {
        nFoundAtsign = 0;
    } else {
        nFoundAtsign = 1;
    }
    size_t atAtsign = 0;
again:
    if (finalBlock == false) {
        for (; atAtsign < sizeBuf; atAtsign++) {
            if (buf[sizeBuf - 1 - atAtsign] == '@' &&
                buf[sizeBuf - 2 - atAtsign] == '\n') {
                nFoundAtsign++;
                if (nFoundAtsign == 2) {
                    nFoundAtsign = 1;
                    break;
                }
            }
        }
    } else {
        assert(buf[0] == '@');
//        atAtsign = sizeBuf - 1; // To avoid sizeBuf sizeBuf - 2 - atAtsign < 0
        for (; atAtsign < sizeBuf; atAtsign++) {
            if (buf[sizeBuf - 1 - atAtsign] == '@') {
                if (sizeBuf > 1 + atAtsign) {
                    if (buf[sizeBuf - 2 - atAtsign] == '\n') {
                        nFoundAtsign++;
                        if (nFoundAtsign == 2) {
                            nFoundAtsign = 1;
                            break;
                        }
                    }
                } else {
                    nFoundAtsign = 1;
                    break;
                }
            }
        }
    }
    if (atAtsign == sizeBuf) {
        // Error: no valid read is found.
        assert(0);
        return ALDER_STATUS_ERROR;
    }
    
    size_t posAtsign = sizeBuf - 1 - atAtsign;
    assert(buf[posAtsign] == '@');
    if (buf[posAtsign] != '@') {
        // Not a fastq read block!
        atAtsign++;
        goto again;
    }
    alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    while (posAtsign < sizeBuf && buf[posAtsign++] != '\n') {
        alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    }
    alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    size_t lenSeq = 0;
    while (posAtsign < sizeBuf && buf[posAtsign++] != '\n') {
        alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
        lenSeq++;
    }
    alder_log5("seq len: %zu", lenSeq);
    if (buf[posAtsign] != '+') {
        // Not a fastq read block!
        atAtsign++;
        goto again;
    }
    alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    while (posAtsign < sizeBuf && buf[posAtsign++] != '\n') {
        alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    }
    alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
    size_t lenQual = 0;
    while (posAtsign < sizeBuf && buf[posAtsign++] != '\n') {
        alder_log5("[%zu] %c", posAtsign, buf[posAtsign]);
        lenQual++;
    }
    alder_log5("qual len: %zu", lenQual);
    if (lenSeq != lenQual) {
        // Not a fastq read block!
        atAtsign++;
        goto again;
    }
    if (finalBlock == false && buf[posAtsign] != '@') {
        // Not a fastq read block!
        atAtsign++;
        goto again;
    }
   
    *lenBuf = posAtsign;
    return 0;
}

int
alder_fastq_chunk(size_t *lenBuf, char *buf, size_t sizeBuf,
                  size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                  void *fx, int gz_flag)
{
    assert(sizeBuf2 < sizeBuf);
    assert(*lenBuf2 <= sizeBuf2);
    char *startBuf = buf + (*lenBuf2);
    // 1
    if (*lenBuf2 > 0) {
        memcpy(buf, buf2, sizeof(*buf) * (*lenBuf2));
        alder_loga4("buf0", (uint8_t*)buf, (*lenBuf2));
    }
    int lenRead;
    if (gz_flag) {
        lenRead = gzread((gzFile)fx, startBuf,
                         (unsigned)(sizeof(*buf)*(sizeBuf - *lenBuf2)));
    } else {
        lenRead = (int) read((int)((intptr_t)fx), startBuf,
                             sizeof(*buf)*(sizeBuf - *lenBuf2));
//        lenRead = fread(startBuf, sizeof(*buf), sizeBuf - *lenBuf2, fp);
    }
    
//        ks->end = gzread ((gzFile)ks->f, ks->buf, 4096);
//        if (ks->end < 4096)
//            ks->is_eof = 1;
//        if (ks->end == 0)
//            return -1;
//
    
    
//        ks->end = (int)read ((int)((intptr_t)ks->f), ks->buf, 4096);
//        if (ks->end == -1) {
//            fprintf(stderr, "file_ks_getc: %s,", strerror(errno));
//        }
//        if (ks->end < 4096)
//            ks->is_eof = 1;
//        if (ks->end == 0)
//            return -1;
    
    if (lenRead == 0 && *lenBuf2 == 0) {
        if (*lenBuf2 > 0) {
            *lenBuf = *lenBuf2;
            *lenBuf2 = 0;
            return ALDER_STATUS_SUCCESS;
        } else {
            // check if this is error or end of file
//            if (feof(fp)) {
                alder_log3("End of File!");
//            } else if (ferror(fp)) {
//                alder_loge(1,"Error - fread fasta!");
//            }
            *lenBuf = 0;
            *lenBuf2 = 0;
            return ALDER_STATUS_ERROR;
        }
    } else if ((size_t)lenRead < sizeBuf - *lenBuf2) {
        // I'd reach the end of the file.
#if !defined(NDEBUG)
        bstring bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
        alder_fastq_chunk_check_read(lenBuf, buf, (*lenBuf2 + lenRead), true);
#if !defined(NDEBUG)
        bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
    } else {
        // 2
        assert(lenRead + *lenBuf2 == sizeBuf);
#if !defined(NDEBUG)
        bstring bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
        alder_fastq_chunk_check_read(lenBuf, buf, (*lenBuf2 + lenRead), false);
#if !defined(NDEBUG)
        bbuf = blk2bstr(buf, (int)(*lenBuf));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
    }
    // 3
    *lenBuf2 = (*lenBuf2 + lenRead) - *lenBuf;
    if (*lenBuf2 > 0) {
        memcpy(buf2, buf + *lenBuf, *lenBuf2);
    }
    
    assert(*lenBuf2 <= sizeBuf2);
    return ALDER_STATUS_SUCCESS;
}



int
alder_fastq_chunk2(size_t *lenBuf, char *buf, size_t sizeBuf,
                   size_t *lenBuf2, char *buf2, size_t sizeBuf2,
                   FILE *fp)
{
    assert(sizeBuf2 < sizeBuf);
    assert(*lenBuf2 <= sizeBuf2);
    char *startBuf = buf + (*lenBuf2);
    // 1
    if (*lenBuf2 > 0) {
        memcpy(buf, buf2, sizeof(*buf) * (*lenBuf2));
        alder_loga4("buf0", (uint8_t*)buf, (*lenBuf2));
    }
    size_t lenRead = fread(startBuf, sizeof(*buf), sizeBuf - *lenBuf2, fp);
    if (lenRead == 0 && *lenBuf2 == 0) {
        if (*lenBuf2 > 0) {
            *lenBuf = *lenBuf2;
            *lenBuf2 = 0;
            return ALDER_STATUS_SUCCESS;
        } else {
            // check if this is error or end of file
            if (feof(fp)) {
                alder_log3("End of File!");
            } else if (ferror(fp)) {
                alder_loge(1,"Error - fread fasta!");
            }
            *lenBuf = 0;
            *lenBuf2 = 0;
            return ALDER_STATUS_ERROR;
        }
    } else if (lenRead < sizeBuf - *lenBuf2) {
        // I'd reach the end of the file.
#if !defined(NDEBUG)
        bstring bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
        alder_fastq_chunk_check_read(lenBuf, buf, (*lenBuf2 + lenRead), true);
#if !defined(NDEBUG)
        bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
    } else {
        // 2
        assert(lenRead + *lenBuf2 == sizeBuf);
#if !defined(NDEBUG)
        bstring bbuf = blk2bstr(buf, (int)(*lenBuf2 + lenRead));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
        alder_fastq_chunk_check_read(lenBuf, buf, (*lenBuf2 + lenRead), false);
#if !defined(NDEBUG)
        bbuf = blk2bstr(buf, (int)(*lenBuf));
        alder_log5("buf1 (%02d): %s", blength(bbuf), bdata(bbuf));
        bdestroy(bbuf);
#endif
    }
    // 3
    *lenBuf2 = (*lenBuf2 + lenRead) - *lenBuf;
    if (*lenBuf2 > 0) {
        memcpy(buf2, buf + *lenBuf, *lenBuf2);
    }
    
    assert(*lenBuf2 <= sizeBuf2);
    return ALDER_STATUS_SUCCESS;
}

