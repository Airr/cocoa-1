/**
 * This file, alder_bzip2.c, is part of alder-bzip2.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-bzip2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-bzip2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-bzip2.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h> 
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include "bzlib.h"
#include "alder_bzip2.h"

/**
 *  This function decompress a bzip2 file.
 *
 *  @param srcfn bzip2 file name
 *  @param dstfn out file name
 */
void alder_bzip2_decompress(const char *srcfn, const char *dstfn)
{
    size_t size_buffer = 1LLU << 16;
    void *ibuffer = malloc(size_buffer);
    void *obuffer = malloc(size_buffer);
    
    int verbosity = 0;
    int small = 0;
    int ret = BZ_OK;
    bz_stream strm;
    strm.bzalloc = NULL;
    strm.bzfree = NULL;
    strm.opaque = NULL;
    ret = BZ2_bzDecompressInit(&strm, verbosity, small);
    if (ret != BZ_OK) {
        free(ibuffer);
        free(obuffer);
        return;
    }
    
    int ifd = open(srcfn, O_RDONLY , 0666);
    if (ifd == -1) {
        BZ2_bzDecompressEnd(&strm);
        free(ibuffer);
        free(obuffer);
        return;
    }
    int ofd = open(dstfn, O_WRONLY | O_CREAT, 0666);
    if (ofd == -1) {
        close(ifd);
        BZ2_bzDecompressEnd(&strm);
        free(ibuffer);
        free(obuffer);
        return;
    }
    while (1) {
        ssize_t read_len = read(ifd, ibuffer, size_buffer);
        if (read_len > 0) {
            //
            strm.next_in = (char *)ibuffer;
            strm.avail_in = (unsigned int)read_len;
            
            while (BZ_OK == ret && strm.avail_in > 0)
            {
                strm.next_out = (char*)obuffer;
                strm.avail_out = (unsigned int)size_buffer;
                ret = BZ2_bzDecompress(&strm);
                
                // Write out the bufs if we had no error.
                if (BZ_OK == ret || BZ_STREAM_END == ret)
                {
                    ssize_t written_len = write(ofd, obuffer, strm.next_out - (char*)obuffer);
//                    size_t written = fwrite(buf, sizeof(char), strm.next_out - buf, f);
                    if (written_len == 0) {
//                        printf("0 is written!\n");
                    }
                    if (written_len != strm.next_out - (char*)obuffer)
                    {
                        break;
                    }
                }
            }
            if (ret != BZ_OK) {
                // Error.
            }
        } else if (read_len == 0) {
            // End of file
            break;
        } else {
            // Error in reading...
            break;
        }
    }
    BZ2_bzDecompressEnd(&strm);
    free(ibuffer);
    free(obuffer);
    close(ifd);
    close(ofd);
}

