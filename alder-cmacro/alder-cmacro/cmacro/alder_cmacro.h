/**
 * This file, alder_cmacro.h, is part of alder-cmacro.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-cmacro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-cmacro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-cmacro.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_cmacro_alder_cmacro_h
#define alder_cmacro_alder_cmacro_h

#include <stdio.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

enum {
    ALDER_STATUS_SUCCESS = 0,
    ALDER_STATUS_ERROR = 1
};
#define ALDER_HELP_LINE_SIZE 1024

#define XFCLOSE(x) do { if ((x) != NULL) {fclose(x); x=NULL;} } while(0)
#define XFREE(x) do { if ((x) != NULL) {free(x); x=NULL;} } while(0)
#define XRETURN(x) do { if ((x) == NULL) {return NULL;} } while(0)
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define ALDER_MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ALDER_MAX(a,b)  ((a) > (b) ? (a) : (b))

/* Strings */
#define ALDER_KMER_TABLE_EXTENSION "tbl"

/* Numbers */
#define ALDER_HASHTABLE_MCAS_EMPTYKEY 4919131752989213764LLU
#define ALDER_LOG_TEXTWIDTH -21
#define BLOCKSIZE 256
#define ALDER_NUMKMER_8BYTE 31
#define ALDER_NUMBIT_KMER_8BYTE 62
#define ALDER_MB2BYTE 1048576
#define ALDER_GB2BYTE 1073741824
#define ALDER_MB2BIT 8388608
#define ALDER_MB2BITTIMESPOINT7 5872026 /* 0.7 x ALDER_MB2BIT */
#define ALDER_BYTE2BIT 8
#define ALDER_RETURN_NULL_IF_NULL(x) do {if ((x) == NULL) {return NULL;}} while(0)
#define ALDER_RETURN_ERROR_IF_NULL(x,alder_errno) \
        do { \
            if ((x) == NULL) { \
                return alder_errno ; \
            } \
        } while (0)
#define ALDER_RETURN_ERROR_UNLESS_SUCCESSFUL(x,alder_errno) \
        do { \
            if ((x) != 0) { \
                return alder_errno ; \
            } \
        } while (0)
#define ALDER_RETURN_IF_EQUAL(x,y) \
        do { \
            if ((x) == (y)) { \
                return y; \
            } \
        } while (0)


#define ALDER_UC_CONNECT_NODE_UP_DOWN(upnode,downnode) \
        do { \
            (upnode)->down = (downnode); \
            (downnode)->up = (upnode); \
        } while (0)

#define ALDER_UC_DISCONNECT_NODE_DOWN(node) \
        do { \
            (node)->down->up = NULL; \
            (node)->down = NULL; \
        } while (0)

#define ALDER_UC_DISCONNECT_NODE_UP(node) \
        do { \
            (node)->up->down = NULL; \
            (node)->up = NULL; \
        } while (0)

#pragma mark encoding

#define ALDER_BYTESIZE_KMER(a,b) (((a)%(b)) ? (a)/(b) + 1: (a)/(b))
#define ALDER_ENCODING_ACGT(character) (((int)(character))>>1)&3

typedef int alder_comparison_func (const void *, const void *);
typedef void alder_print_func (FILE *, const void *);

#pragma mark file

#define ALDER_FILE_YES 1
#define ALDER_FILE_NO  0

#define ALDER_YES 1
#define ALDER_NO  0
#define ALDER_BUFSIZE_100 100
#define ALDER_BUFSIZE_8MB 8388608

#pragma mark buffer

#define to_uint64(buffer,n) (*(uint64_t*)(buffer + n))
#define to_uint32(buffer,n) (*(uint32_t*)(buffer + n))
#define to_uint16(buffer,n) (*(uint16_t*)(buffer + n))
#define to_size(buffer,n) (*(size_t*)(buffer + n))

__END_DECLS

#endif /* alder_cmacro_alder_cmacro_h */
