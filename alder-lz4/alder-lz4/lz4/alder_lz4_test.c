/**
 * This file, alder_lz4_test.c, is part of alder-lz4.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-lz4 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-lz4 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-lz4.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

static int progress_step(unsigned long cstep, unsigned long tstep, int tostderr)
{
    static int tick = 0;
    static time_t start;
    static time_t end;
    if (tick == 0) {
        start = time(NULL);
    }
    tick++;
    if (tick % 10000 == 0) {
        int progress = (int)((double)(cstep) * 100
                             /
                             (double)(tstep));
        if (tostderr) {
            fprintf(stderr,"%d\n", progress);
            fflush(stderr);
        } else {
            end = time(NULL);
            double elapsed = difftime(end, start);
            double remaining = elapsed * (tstep / cstep - 1) / 3600.0;
            fprintf(stdout,"\rProgress: %d%% %.1f (hours)", progress, remaining);
            //        fprintf(stdout,"\rProgress: %d%% %.1f (seconds)", progress, remaining);
            fflush(stdout);
        }
    }
    return 0;
}

static int progress_end(int tostderr)
{
    if (tostderr) {
        fprintf(stderr, "100\n");
        fflush(stderr);
    } else {
        fprintf(stdout, "\r                                                 ");
        fflush(stdout);
        fprintf(stdout, "\rDone!\n");
    }
    return 0;
}

static int file_size(const char *path, size_t *size)
{
    struct stat fileStat;
    if(stat(path, &fileStat) < 0) {
        fprintf(stderr, "Error - cannot find the info of file %s: %s",
                path, strerror(errno));
        return 1;
    }
    *size = (unsigned long)(fileStat.st_size);
    return 0;
}

void alder_lz4_test(const char *fn)
{
    time_t start = time(NULL);
    void *lz4state = NULL;
    const size_t n_bit = 16;
    int fd = open (fn, O_RDONLY, 0666);
//    fcntl(fd, F_NOCACHE, 1);
//    fcntl(fd, F_RDAHEAD, 1);
    int buf_size = 1 << n_bit;
//    int obuf_size = LZ4_COMPRESSBOUND(buf_size);
    int obuf_size = 1 << 20;
    int dbuf_size = 1 << (n_bit+3);
    uint8_t *buf = malloc(buf_size);
    uint8_t *obuf = malloc(obuf_size);
    uint8_t *dbuf = malloc(dbuf_size);
    assert(buf != NULL);
    assert(obuf != NULL);
    assert(dbuf != NULL);
    
    lz4state = malloc(LZ4_sizeofState());
    if(NULL == lz4state)
    {
        close(fd);
        return;
    }
    
//    int csize;
    
    size_t total_size;
    file_size(fn, &total_size);
    unsigned long total_read = 0;
    
    ssize_t left_to_read = (ssize_t) total_size;
    size_t count = (1ULL << n_bit);
    
    while (left_to_read > 0) {
        if (left_to_read < count) {
            count = left_to_read;
        }
        ssize_t read_len = read(fd, buf, count);
        /* An error occurred; bail. */
        if (read_len == -1)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_read -= read_len;
        
//        csize = LZ4_compress_withState(lz4state, (char*)buf, (char*)obuf, (int)read_len);
        
//        if (csize > 0) {
        
            int decomp_len = LZ4_decompress_safe((char*)buf, (char*)dbuf,
//            int decomp_len = LZ4_decompress_safe_withPrefix64k((char*)buf, (char*)dbuf,
                                                 (int)read_len, dbuf_size);
//                                                 csize, dbuf_size);
            if (decomp_len > 0) {
                fwrite(dbuf, 1, decomp_len, stdout);
            } else if (decomp_len == 0) {
                assert(read_len == 0);
            } else {
                assert(0);
            }
//        } else {
//            assert(0);
//            break;
//        }
        
        total_read = total_size - left_to_read;
        progress_step(total_read, total_size, 0);
    }
    assert (left_to_read == 0);
    progress_end(0);
    
    close(fd);
    free(buf);
    free(obuf);
    free(dbuf);
    free(lz4state);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    fprintf(stdout, "Run Time: %.f secdons.\n", elapsed);
    
}

