/**
 * This file, alder_kmer_uncompress.c, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "alder_dna.h"
#include "alder_logger.h"
#include "alder_progress.h"
#include "alder_file_size.h"
#include "alder_kmer_binary.h"
#include "alder_kmer_common.h"
#include "alder_kmer_uncompress.h"

struct alder_kmer_binary_stream_struct {
    size_t len_buf;
    uint8_t *buf;
    size_t cur_buf;
    int e_counter;;
    uint8_t e_byte;;
    uint16_t i_len;;
    size_t size_buf;
    uint16_t len;
};

alder_kmer_binary_stream_t* alder_kmer_binary_buffer_create()
{
    alder_kmer_binary_stream_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    return o;
}

void alder_kmer_binary_buffer_destroy(alder_kmer_binary_stream_t *o)
{
    XFREE(o);
}

/**
 *  This function initializes a buffer from an alder-kmer binary input file.
 *
 *  @param bs_p    binary strem
 *  @param buffer  buffer
 *  @param len_buf length of the buffer (0 means no input)
 *
 *  cur_buf is the position of a pointer to a byte to decode
 *          (16th or ALDER_KMER_BINARY_READBLOCK_BODY pos in buffer).
 *  e_counter rotates 0,1,2,3.
 *  e_byte is a byte value to decode.
 *  i_len is the index of an encoded sequence.
 *  len is the length of an encoded sequence.
 *  size_buf is the length of valid buffer from the 16th position.
 *
 */
void alder_kmer_binary_buffer_open(alder_kmer_binary_stream_t *bs_p,
                                   uint8_t *buffer,
                                   size_t len_buf)
{
    assert(len_buf == 0 || len_buf == ALDER_KMER_BUFSIZE);
    
    bs_p->len_buf = len_buf;
    bs_p->buf = buffer;
    bs_p->e_counter = 0;
    bs_p->e_byte = 0;
    bs_p->i_len = 0;
    bs_p->cur_buf = 0;
    
    // Used to be 4, 12
    // Now, they are 8 and 16. See alder_kmer_binary.h
    bs_p->size_buf = to_size(buffer, ALDER_KMER_BINARY_READBLOCK_LEN);
    bs_p->len = to_uint16(bs_p->buf, bs_p->cur_buf + ALDER_KMER_BINARY_READBLOCK_BODY);
    
    bs_p->cur_buf = sizeof(bs_p->len);
}

int alder_kmer_binary_buffer_read(alder_kmer_binary_stream_t *bs_p,
                                  FILE *fp)
{
    int token=-1;
    
    if (bs_p->len_buf == 0) {
        return 5;
    }
//    if (bs_p->cur_buf >= bs_p->size_buf) {
//        return 5;
//    }
    
    if (bs_p->i_len < bs_p->len) {
        
        if (bs_p->e_counter == 4) {
            bs_p->e_counter = 0;
            bs_p->cur_buf++;
        }
        
        if (bs_p->cur_buf == bs_p->size_buf) {
            fprintf(stderr, "Error: cur_buf == size_buf. Must be <.\n");
            assert(0);
            abort();
            return 5;
        }
        
        if (bs_p->e_counter == 0) {
            bs_p->e_byte = bs_p->buf[bs_p->cur_buf + ALDER_KMER_BINARY_READBLOCK_BODY];
        }
        bs_p->e_counter++;
        token = (bs_p->e_byte >> (8 - 2*bs_p->e_counter)) & 3;
        
        if (fp != NULL) {
            char c = alder_dna_int2char(token);
            putc(c, fp);
        }
        bs_p->i_len++;
    } else {
        bs_p->cur_buf++;
        
        if (!(bs_p->cur_buf < bs_p->size_buf)) {
            return 5;
        }
        
        bs_p->len = to_uint16(bs_p->buf, bs_p->cur_buf + ALDER_KMER_BINARY_READBLOCK_BODY);
        bs_p->cur_buf += sizeof(bs_p->len);
        bs_p->e_counter = 0;
        bs_p->e_byte = 0;
        bs_p->i_len = 0;
        if (fp != NULL) {
            putc('\n', fp);
        }
        return 4;
    }
    
    return token;
}

static int int2char[6] = { 'A', 'C', 'T', 'G', '\n', '\n' };
    
/**
 *  This function converts binary files to simple sequences.
 *
 *  @param progress_flag progress
 *  @param infile        infile
 *  @param outfile_given outfile is given flag
 *  @param outdir        output directory
 *  @param outfile       outfile name
 *
 *  @return SUCCESS or FAIL
 */
int
alder_kmer_uncompress(int progress_flag,
                      struct bstrList *infile,
                      unsigned int outfile_given,
                      const char *outdir,
                      const char *outfile)
{
    size_t size_buf = ALDER_KMER_BUFSIZE;
    
    uint8_t *buffer = malloc(size_buf);
    if (buffer == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "failed to uncompress");
        return ALDER_STATUS_ERROR;
    }
    bstring boutfile = bformat("%s/%s.seq", outdir, outfile);
    if (boutfile == NULL) {
        alder_loge(ALDER_ERR_FILE, "failed to uncompress");
        XFREE(buffer);
        return ALDER_STATUS_ERROR;
    }
    
    int ofd = STDOUT_FILENO;
    if (outfile_given) {
        ofd = open(bdata(boutfile), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (ofd == -1) {
            alder_loge(ALDER_ERR_FILE, "failed to open %s", bdata(boutfile));
            bdestroy(boutfile);
            XFREE(buffer);
            return ALDER_STATUS_ERROR;
        }
    }
    
    int i_start = 0;
    if (infile->qty == 0) {
        i_start = -1;
    }
    for (int i = i_start; i < infile->qty; i++) {
        int fd;
        if (i == -1) {
            fd = STDIN_FILENO;
        } else {
            fd = open(bdata(infile->entry[i]), O_RDONLY, 0666);
            if (fd == -1) {
                alder_loge(ALDER_ERR_FILE, "failed to open %s",
                           bdata(infile->entry[i]));
                bdestroy(boutfile);
                XFREE(buffer);
                return ALDER_STATUS_ERROR;
            }
        }
        
        while (1) {
            ssize_t left_to_read = size_buf;
            size_t read_len = 0;
            while (left_to_read > 0) {
                ssize_t cur_read_len = read(fd, buffer + read_len, left_to_read);
                if (cur_read_len == 0) {
                    break;
                }
                read_len += cur_read_len;
                left_to_read -= cur_read_len;
            }
            
            if (read_len == size_buf) {
                assert(read_len == size_buf);
                int token = -1;
                alder_kmer_binary_stream_t bs;
                alder_kmer_binary_buffer_open(&bs, buffer, read_len);
                while (token < 5) {
                    token = alder_kmer_binary_buffer_read(&bs, NULL);
                    char c = int2char[token];
                    write(ofd, &c, 1);
                }
            } else {
                break;
            }
        }
        
        if (i >= 0) {
            close(fd);
        }
    }
    
    free(buffer);
    bdestroy(boutfile);
    return 0;
}
