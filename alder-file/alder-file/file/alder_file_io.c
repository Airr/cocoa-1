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

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h> 
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "alder_progress.h"
#include "alder_file_rm.h"
#include "alder_file_size.h"
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

/**
 *  This function uses file descriptors to test writing a giga-byte file.
 *
 *  @param filesize file size (e.g., 30 means 1 giga-byte file)
 *  @param b        buffer size (e.g., 10 means 1 kilo-byte)
 *  @param bufsize  content size (must be larger than b)
 *
 *  @return run time in seconds.
 */
double alder_file_io_test01_buf(int filesize, int b, int bufsize)
{
    time_t start = time(NULL);
    int fd = open ("1", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    uint8_t *buffer = malloc(1 << bufsize);
    ssize_t left_to_write = 1 << filesize;
//    size_t count = (1 << b) + 17;
    size_t count = (1 << b) + 123;
    while (left_to_write > 0) {
        if (left_to_write < count) {
            count = left_to_write;
        }
        ssize_t written = write(fd, buffer, count);
        /* An error occurred; bail. */
        if (written == -1)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_write -= written;
        
    }
//    printf("left_to_write: %zd\n", left_to_write);
    /* We should have written no more than COUNT bytes! */
    assert (left_to_write == 0);
    /* The number of bytes written is exactly COUNT. */
    
    close(fd);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
//    printf("Elapsed: b(%d) %.f\n", b, elapsed);
    return elapsed;
}

/* 
 *  Use FILE
 */
double alder_file_io_test02_buf(int filesize, int b, int bufsize)
{
    time_t start = time(NULL);
    FILE *fp = fopen("1", "w");
    
    uint8_t *buffer = malloc(1 << bufsize);
    ssize_t left_to_write = 1 << filesize;
    size_t count = (1 << b) + 123;
    while (left_to_write > 0) {
        if (left_to_write < count) {
            count = left_to_write;
        }
        size_t written = fwrite(buffer, 1, count, fp);
        /* An error occurred; bail. */
        if (written == 0)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_write -= written;
        
    }
//    printf("left_to_write: %zd\n", left_to_write);
    /* We should have written no more than COUNT bytes! */
    assert (left_to_write == 0);
    /* The number of bytes written is exactly COUNT. */
    
    fclose(fp);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
//    printf("Elapsed: b(%d) %.f\n", b, elapsed);
    return elapsed;
}

/**
 *  This funciton creates a few gigabytes of a file.
 *
 *  MacPro(regular disk): 1MB is the fastest writing buffer size.
 *  MacBook(SSD): from 4KB, it does not matter.
 */
void alder_file_io_test01()
{
    printf("Using FILE...\n");
    int N = 10;
    int bufsize = 27;
    for (int b = 4; b < bufsize; b++) {
        double t_e = 0;
        for (int i = 0; i < N; i++) {
//            double e = alder_file_io_test01_buf(30, b, bufsize);
            double e = alder_file_io_test02_buf(30, b, bufsize);
            t_e += e;
        }
        t_e /= N;
        printf("b(%d): %.2f\n", b, t_e);
    }
}

/**
 *  This function uses file descriptors to test writing a giga-byte file.
 *
 *  @param filesize file size (e.g., 30 means 1 giga-byte file)
 *  @param b        buffer size (e.g., 10 means 1 kilo-byte)
 *  @param bufsize  content size (must be larger than b)
 *
 *  @return run time in seconds.
 */
double alder_file_io_test02_readbuf_using_fd(const char *fn, int b, int bufsize)
{
    time_t start = time(NULL);
    int fd = open (fn, O_RDONLY, 0666);
    
    uint8_t *buffer = malloc(1 << bufsize);
//    size_t fsize;
//    alder_file_size(fn, &fsize);
//    ssize_t left_to_read = (ssize_t)fsize;
    
    ssize_t left_to_read = (ssize_t)(1LLU << 34);
//    printf("fd read: %zd\n", left_to_read);
    size_t count = (1 << b) + 123;
//    size_t count = (1 << b);
    while (left_to_read > 0) {
        if (left_to_read < count) {
            count = left_to_read;
        }
        ssize_t read_len = read(fd, buffer, count);
        /* An error occurred; bail. */
        if (read_len == -1)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_read -= read_len;
        
    }
//    printf("left_to_write: %zd\n", left_to_write);
    /* We should have written no more than COUNT bytes! */
    assert (left_to_read == 0);
    /* The number of bytes written is exactly COUNT. */
    
    close(fd);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
//    printf("Elapsed: b(%d) %.f\n", b, elapsed);
    return elapsed;
}

double alder_file_io_test02_readbuf_using_FILE(const char *fn, int b, int bufsize)
{
    time_t start = time(NULL);
//    int fd = open (fn, O_RDONLY, 0666);
    FILE *fp = fopen(fn, "rb");
    
    uint8_t *buffer = malloc(1 << bufsize);
//    size_t left_to_read;
//    alder_file_size(fn, &left_to_read);
    size_t left_to_read = (size_t)(1LLU << 34);
//    printf("FILE read: %zu\n", left_to_read);
    
    size_t count = (1 << b) + 123;
//    size_t count = (1 << b);
    while (left_to_read > 0) {
        if (left_to_read < count) {
            count = left_to_read;
        }
//        ssize_t read_len = read(fd, buffer, count);
        size_t read_len = fread(buffer, 1, count, fp);
        /* An error occurred; bail. */
        if (read_len == 0)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_read -= read_len;
        
    }
//    printf("left_to_write: %zd\n", left_to_write);
    /* We should have written no more than COUNT bytes! */
    assert (left_to_read == 0);
    /* The number of bytes written is exactly COUNT. */
    
//    close(fd);
    fclose(fp);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
//    printf("Elapsed: b(%d) %.f\n", b, elapsed);
    return elapsed;
}

void alder_file_io_test02()
{
    int N = 10;
    int bufsize = 18;
    
    // Create a file named "1"
//    alder_file_io_test02_buf(30, 20, 21);
    for (int b = 15; b < bufsize; b++) {
        double t_e = 0;
        for (int i = 0; i < N; i++) {
            double e = alder_file_io_test02_readbuf_using_fd("20", b, bufsize);
//            double e = alder_file_io_test02_readbuf_using_FILE("20", b, bufsize);
            t_e += e;
        }
        t_e /= N;
        printf("b(%d): %.2f\n", b, t_e);
    }
}

/**
 *  This function creates a file.
 *
 *  @param fn       file name
 *  @param filesize file size in log2; e.g., 30 for 1GB, 39 for 512GB.
 *  @param b        buffer size in log2; use 20 for 1MB
 *  @param bufsize  data size in log2; use 21 that is larger than b
 *
 *  @return time in sec.
 */
double alder_file_io_test03_create(const char *fn,
                                   int filesize, int b, int bufsize)
{
    time_t start = time(NULL);
    int fd = open (fn, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    uint8_t *buffer = malloc(1 << bufsize);
    unsigned long total_size = 1LLU << filesize;
    unsigned long total_written = 0;
    ssize_t left_to_write = 1LLU << filesize;
    size_t count = (1 << b);
    while (left_to_write > 0) {
        if (left_to_write < count) {
            count = left_to_write;
        }
        ssize_t written = write(fd, buffer, count);
        /* An error occurred; bail. */
        if (written == -1)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_write -= written;
        
        total_written = total_size - left_to_write;
        alder_progress_step((unsigned long) total_written,
                            (unsigned long) total_size, 0);
        
    }
    assert (left_to_write == 0);
    /* The number of bytes written is exactly COUNT. */
    alder_progress_end(0);
    
    close(fd);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    return elapsed;
}

double alder_file_io_test03_readbuf_using_fd(const char *fn, int b, int bufsize)
{
    time_t start = time(NULL);
    int fd = open (fn, O_RDONLY, 0666);
    printf("F_NOCACHE/F_RDAHEAD\n");
//    fcntl(fd, F_NOCACHE, 1);
//    fcntl(fd, F_RDAHEAD, 1);
    
    
    uint8_t *buffer = malloc(1 << bufsize);
    
    size_t total_size;
    alder_file_size(fn, &total_size);
    unsigned long total_read = 0;
    
    ssize_t left_to_read = (ssize_t) total_size;
    size_t count = (1 << b);
    while (left_to_read > 0) {
        if (left_to_read < count) {
            count = left_to_read;
        }
        ssize_t read_len = read(fd, buffer, count);
        /* An error occurred; bail. */
        if (read_len == -1)
            break;
        else /* Keep count of how much more we need to write. */
            left_to_read -= read_len;
        
        total_read = total_size - left_to_read;
        alder_progress_step((unsigned long) total_read,
                            (unsigned long) total_size, 0);
    }
    assert (left_to_read == 0);
    
    close(fd);
    free(buffer);
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    return elapsed;
}


/**
 *  This function tests reading a file of 500GB.
 */
void alder_file_io_test03(const char *fn)
{
//    const char *fn = "1";
    double d;
//    d = alder_file_io_test03_create(fn, 39, 20, 21);
//    printf("File(512GB): %s is created in %.1f\n", fn, d);
    d = alder_file_io_test03_readbuf_using_fd(fn, 16, 17);
    printf("File(512GB): %s is read in %.1f\n", fn, d);
}


/**
 *  This function determines the best buffer size for writing using write.
 *
 *  @param fn output file name
 *
 *  @return buffer size in log2
 */
size_t alder_file_io_best_write_buffer_size(const char *fn)
{
    size_t v = 0;
    unsigned long min_elapsed = ULONG_MAX;
    size_t filesize = 32;
    size_t max_size_buffer = 23;
    size_t size_buffer = 1 << max_size_buffer;
    void *buffer = malloc(size_buffer);
    int fd = open("/dev/random", O_RDONLY);
    read(fd, buffer, size_buffer);
    close(fd);
//    write(2, buffer, size_buffer);
    
    for (size_t b = 14; b < max_size_buffer; b++) {
        unsigned long elapsed_clock = 0;
        double elapsed_average = 0;
        for (size_t k = 0; k < 3; k++) {
            alder_file_rm(fn);
            clock_t start_clock = clock();
            int fd = open (fn, O_WRONLY|O_CREAT , 0666);
            ssize_t left_to_write = 1LLU << filesize;
            size_t write_size = 1 << b;
            while (left_to_write > 0) {
                if (left_to_write < write_size) {
                    write_size = left_to_write;
                }
                ssize_t written = write(fd, buffer, write_size);
                if (written == -1)
                    break;
                else /* Keep count of how much more we need to write. */
                    left_to_write -= written;
            }
            assert (left_to_write == 0);
            
            close(fd);
            clock_t end_clock = clock();
            elapsed_clock += (end_clock - start_clock);
        }
        printf("%zu: %.1f sec. %ld\n", b, elapsed_average, elapsed_clock);
        if (elapsed_clock <= min_elapsed) {
            min_elapsed = elapsed_clock;
            v = b;
        }
    }
    XFREE(buffer);
    alder_file_rm(fn);

    return v;
}


int alder_file_io_best_buffer_size(const char *fn,
                                   size_t *best_write_buffer_size,
                                   size_t *best_read_buffer_size)
{
    *best_write_buffer_size = 0;
    *best_read_buffer_size = 0;
    unsigned long min_elapsed_write = ULONG_MAX;
    unsigned long min_elapsed_read = ULONG_MAX;
    size_t filesize = 32;
    size_t max_size_buffer = 23;
    size_t size_buffer = 1 << max_size_buffer;
    void *buffer = malloc(size_buffer);
    int fd = open("/dev/random", O_RDONLY);
    read(fd, buffer, size_buffer);
    close(fd);
    
    for (size_t b = 14; b < max_size_buffer; b++) {
        unsigned long elapsed_clock_write = 0;
        unsigned long elapsed_clock_read = 0;
        for (size_t k = 0; k < 3; k++) {
            alder_file_rm(fn);
            
            // Measure write speed.
            clock_t start_clock = clock();
            int fd = open (fn, O_WRONLY|O_CREAT , 0666);
            ssize_t left_to_write = 1LLU << filesize;
            size_t write_size = 1 << b;
            while (left_to_write > 0) {
                if (left_to_write < write_size) {
                    write_size = left_to_write;
                }
                ssize_t written = write(fd, buffer, write_size);
                if (written == -1)
                    break;
                else /* Keep count of how much more we need to write. */
                    left_to_write -= written;
            }
            assert (left_to_write == 0);
            close(fd);
            clock_t end_clock = clock();
            elapsed_clock_write += (end_clock - start_clock);
            
            // Measure read speed.
            start_clock = clock();
            fd = open (fn, O_RDONLY , 0666);
            ssize_t left_to_read = 1LLU << filesize;
            size_t read_size = 1 << b;
            while (left_to_read > 0) {
                if (left_to_read < read_size) {
                    read_size = left_to_read;
                }
                ssize_t read_len = read(fd, buffer, read_size);
                if (read_len == -1)
                    break;
                else /* Keep count of how much more we need to write. */
                    left_to_read -= read_len;
            }
            assert (left_to_read == 0);
            
            close(fd);
            end_clock = clock();
            elapsed_clock_read += (end_clock - start_clock);
        }
        printf("%zu: %ld (write), %ld (read)\n", b, elapsed_clock_write, elapsed_clock_read);
        if (elapsed_clock_write <= min_elapsed_write) {
            min_elapsed_write = elapsed_clock_write;
            *best_write_buffer_size = b;
        }
        if (elapsed_clock_read <= min_elapsed_read) {
            min_elapsed_read = elapsed_clock_read;
            *best_read_buffer_size = b;
        }
    }
    XFREE(buffer);
    alder_file_rm(fn);

    return ALDER_STATUS_SUCCESS;
}