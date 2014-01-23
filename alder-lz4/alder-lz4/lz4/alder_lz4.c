/**
 * This file, alder_lz4.c, is part of alder-lz4.
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

#include "lz4.h"
#include "alder_lz4.h"





/*-------------------------------------------------------------*/
/*--- Library top-level functions.                          ---*/
/*---                                               lzlib.c ---*/
/*-------------------------------------------------------------*/

/* ------------------------------------------------------------------
 This file is part of lzip2/liblzip2, a program and library for
 lossless, block-sorting data compression.
 
 lzip2/liblzip2 version 1.0.6 of 6 September 2010
 Copyright (C) 1996-2010 Julian Seward <jseward@lzip.org>
 
 Please read the WARNING, DISCLAIMER and PATENTS sections in the
 README file.
 
 This program is released under the terms of the license contained
 in the file LICENSE.
 ------------------------------------------------------------------ */

/* CHANGES
 0.9.0    -- original version.
 0.9.0a/b -- no changes in this file.
 0.9.0c   -- made zero-length LZ_FLUSH work correctly in lzCompress().
 fixed lzWrite/lzRead to ignore zero-length requests.
 fixed lzread to correctly handle read requests after EOF.
 wrong parameter order in call to lzDecompressInit in
 lzBuffToBuffDecompress.  Fixed.
 */

#include "lzlib_private.h"


/*---------------------------------------------------*/
/*--- Compression stuff                           ---*/
/*---------------------------------------------------*/


/*---------------------------------------------------*/
#ifndef LZ_NO_STDIO
void LZ4_lz__AssertH__fail ( int errcode )
{
    fprintf(stderr,
            "\n\nlzip2/liblzip2: internal error number %d.\n"
            "This is a bug in lzip2/liblzip2, %s.\n"
            "Please report it to me at: jseward@lzip.org.  If this happened\n"
            "when you were using some program which uses liblzip2 as a\n"
            "component, you should also report this bug to the author(s)\n"
            "of that program.  Please make an effort to report this bug;\n"
            "timely and accurate bug reports eventually lead to higher\n"
            "quality software.  Thanks.  Julian Seward, 10 December 2007.\n\n",
            errcode,
            LZ4_lzlibVersion()
            );
    
    if (errcode == 1007) {
        fprintf(stderr,
                "\n*** A special note about internal error number 1007 ***\n"
                "\n"
                "Experience suggests that a common cause of i.e. 1007\n"
                "is unreliable memory or other hardware.  The 1007 assertion\n"
                "just happens to cross-check the results of huge numbers of\n"
                "memory reads/writes, and so acts (unintendedly) as a stress\n"
                "test of your memory system.\n"
                "\n"
                "I suggest the following: try compressing the file again,\n"
                "possibly monitoring progress in detail with the -vv flag.\n"
                "\n"
                "* If the error cannot be reproduced, and/or happens at different\n"
                "  points in compression, you may have a flaky memory system.\n"
                "  Try a memory-test program.  I have used Memtest86\n"
                "  (www.memtest86.com).  At the time of writing it is free (GPLd).\n"
                "  Memtest86 tests memory much more thorougly than your BIOSs\n"
                "  power-on test, and may find failures that the BIOS doesn't.\n"
                "\n"
                "* If the error can be repeatably reproduced, this is a bug in\n"
                "  lzip2, and I would very much like to hear about it.  Please\n"
                "  let me know, and, ideally, save a copy of the file causing the\n"
                "  problem -- without which I will be unable to investigate it.\n"
                "\n"
                );
    }
    
    exit(3);
}
#endif


/*---------------------------------------------------*/
static
int lz_config_ok ( void )
{
    if (sizeof(int)   != 4) return 0;
    if (sizeof(short) != 2) return 0;
    if (sizeof(char)  != 1) return 0;
    return 1;
}


/*---------------------------------------------------*/
static
void* default_lzalloc ( void* opaque, Int32 items, Int32 size )
{
    void* v = malloc ( items * size );
    return v;
}

static
void default_lzfree ( void* opaque, void* addr )
{
    if (addr != NULL) free ( addr );
}



/* SOMETHING DELETED HERE */




/*---------------------------------------------------*/
int LZ_API(LZ4_lzCompressInit)
(lz_stream* strm,
 int        blockSize100k,
 int        verbosity,
 int        workFactor )
{
//    Int32   n;
    
    if (!lz_config_ok()) return LZ_CONFIG_ERROR;
    
    if (strm == NULL ||
        blockSize100k < 1 || blockSize100k > 9 ||
        workFactor < 0 || workFactor > 250)
        return LZ_PARAM_ERROR;
    
    if (workFactor == 0) workFactor = 30;
        
//        return LZ_MEM_ERROR;
    
    return LZ_OK;
}


/*---------------------------------------------------*/
static
Bool handle_compress ( lz_stream* strm )
{
    Bool progress_in  = False;
    Bool progress_out = False;
    
    return progress_in || progress_out;
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzCompress) (lz_stream *strm, int action)
{
    Bool progress;
    return LZ_OK; /*--not reached--*/
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzCompressEnd)  ( lz_stream *strm )
{
    return LZ_OK;
}


/*---------------------------------------------------*/
/*--- Decompression stuff                         ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int LZ_API(LZ4_lzDecompressInit)
( lz_stream* strm,
 int        verbosity,
 int        small )
{
    if (!lz_config_ok()) return LZ_CONFIG_ERROR;
    
    if (strm == NULL) return LZ_PARAM_ERROR;
    if (small != 0 && small != 1) return LZ_PARAM_ERROR;
    if (verbosity < 0 || verbosity > 4) return LZ_PARAM_ERROR;
    
    return LZ_OK;
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzDecompress) ( lz_stream *strm )
{
    Bool    corrupt;
    return 0;  /*NOTREACHED*/
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzDecompressEnd)  ( lz_stream *strm )
{
    return LZ_OK;
}


#ifndef LZ_NO_STDIO
/*---------------------------------------------------*/
/*--- File I/O stuff                              ---*/
/*---------------------------------------------------*/

#define LZ_SETERR(eee)                    \
{                                         \
if (lzerror != NULL) *lzerror = eee;   \
if (lzf != NULL) lzf->lastErr = eee;   \
}

typedef struct {
    FILE*     handle;
    Char      buf[LZ_MAX_UNUSED];
    Int32     bufN;
    Bool      writing;
    lz_stream strm;
    Int32     lastErr;
    Bool      initialisedOk;
}
lzFile;


/*---------------------------------------------*/
static Bool myfeof ( FILE* f )
{
    Int32 c = fgetc ( f );
    if (c == EOF) return True;
    ungetc ( c, f );
    return False;
}


/*---------------------------------------------------*/
LZFILE* LZ_API(LZ4_lzWriteOpen)
(int*  lzerror,
 FILE* f,
 int   blockSize100k,
 int   verbosity,
 int   workFactor )
{
    Int32   ret;
    lzFile* lzf = NULL;
    
    LZ_SETERR(LZ_OK);
    
    if (f == NULL ||
        (blockSize100k < 1 || blockSize100k > 9) ||
        (workFactor < 0 || workFactor > 250) ||
        (verbosity < 0 || verbosity > 4))
    { LZ_SETERR(LZ_PARAM_ERROR); return NULL; };
    
    if (ferror(f))
    { LZ_SETERR(LZ_IO_ERROR); return NULL; };
    
    lzf = malloc ( sizeof(lzFile) );
    if (lzf == NULL)
    { LZ_SETERR(LZ_MEM_ERROR); return NULL; };
    
    LZ_SETERR(LZ_OK);
    lzf->initialisedOk = False;
    lzf->bufN          = 0;
    lzf->handle        = f;
    lzf->writing       = True;
    lzf->strm.lzalloc  = NULL;
    lzf->strm.lzfree   = NULL;
    lzf->strm.opaque   = NULL;
    
    if (workFactor == 0) workFactor = 30;
        ret = LZ4_lzCompressInit ( &(lzf->strm), blockSize100k,
                                  verbosity, workFactor );
        if (ret != LZ_OK)
        { LZ_SETERR(ret); free(lzf); return NULL; };
    
    lzf->strm.avail_in = 0;
    lzf->initialisedOk = True;
    return lzf;
}



/*---------------------------------------------------*/
void LZ_API(LZ4_lzWrite)
(int*    lzerror,
 LZFILE* b,
 void*   buf,
 int     len )
{
    Int32 n, n2, ret;
    lzFile* lzf = (lzFile*)b;
    
    LZ_SETERR(LZ_OK);
    if (lzf == NULL || buf == NULL || len < 0)
    { LZ_SETERR(LZ_PARAM_ERROR); return; };
    if (!(lzf->writing))
    { LZ_SETERR(LZ_SEQUENCE_ERROR); return; };
    if (ferror(lzf->handle))
    { LZ_SETERR(LZ_IO_ERROR); return; };
    
    if (len == 0)
    { LZ_SETERR(LZ_OK); return; };
    
    lzf->strm.avail_in = len;
    lzf->strm.next_in  = buf;
    
    while (True) {
        lzf->strm.avail_out = LZ_MAX_UNUSED;
        lzf->strm.next_out = lzf->buf;
        ret = LZ4_lzCompress ( &(lzf->strm), LZ_RUN );
        if (ret != LZ_RUN_OK)
        { LZ_SETERR(ret); return; };
        
        if (lzf->strm.avail_out < LZ_MAX_UNUSED) {
            n = LZ_MAX_UNUSED - lzf->strm.avail_out;
            n2 = (Int32)fwrite ( (void*)(lzf->buf), sizeof(UChar),
                                n, lzf->handle );
            if (n != n2 || ferror(lzf->handle))
            { LZ_SETERR(LZ_IO_ERROR); return; };
        }
        
        if (lzf->strm.avail_in == 0)
        { LZ_SETERR(LZ_OK); return; };
    }
}


/*---------------------------------------------------*/
void LZ_API(LZ4_lzWriteClose)
(int*          lzerror,
 LZFILE*       b,
 int           abandon,
 unsigned int* nbytes_in,
 unsigned int* nbytes_out )
{
    LZ4_lzWriteClose64 ( lzerror, b, abandon,
                        nbytes_in, NULL, nbytes_out, NULL );
}


void LZ_API(LZ4_lzWriteClose64)
(int*          lzerror,
 LZFILE*       b,
 int           abandon,
 unsigned int* nbytes_in_lo32,
 unsigned int* nbytes_in_hi32,
 unsigned int* nbytes_out_lo32,
 unsigned int* nbytes_out_hi32 )
{
    Int32   n, n2, ret;
    lzFile* lzf = (lzFile*)b;
    
    if (lzf == NULL)
    { LZ_SETERR(LZ_OK); return; };
    if (!(lzf->writing))
    { LZ_SETERR(LZ_SEQUENCE_ERROR); return; };
    if (ferror(lzf->handle))
    { LZ_SETERR(LZ_IO_ERROR); return; };
    
    if (nbytes_in_lo32 != NULL) *nbytes_in_lo32 = 0;
        if (nbytes_in_hi32 != NULL) *nbytes_in_hi32 = 0;
            if (nbytes_out_lo32 != NULL) *nbytes_out_lo32 = 0;
                if (nbytes_out_hi32 != NULL) *nbytes_out_hi32 = 0;
                    
                    if ((!abandon) && lzf->lastErr == LZ_OK) {
                        while (True) {
                            lzf->strm.avail_out = LZ_MAX_UNUSED;
                            lzf->strm.next_out = lzf->buf;
                            ret = LZ4_lzCompress ( &(lzf->strm), LZ_FINISH );
                            if (ret != LZ_FINISH_OK && ret != LZ_STREAM_END)
                            { LZ_SETERR(ret); return; };
                            
                            if (lzf->strm.avail_out < LZ_MAX_UNUSED) {
                                n = LZ_MAX_UNUSED - lzf->strm.avail_out;
                                n2 = (Int32)fwrite ( (void*)(lzf->buf), sizeof(UChar),
                                                    n, lzf->handle );
                                if (n != n2 || ferror(lzf->handle))
                                { LZ_SETERR(LZ_IO_ERROR); return; };
                            }
                            
                            if (ret == LZ_STREAM_END) break;
                        }
                    }
    
    if ( !abandon && !ferror ( lzf->handle ) ) {
        fflush ( lzf->handle );
        if (ferror(lzf->handle))
        { LZ_SETERR(LZ_IO_ERROR); return; };
    }
    
    if (nbytes_in_lo32 != NULL)
        *nbytes_in_lo32 = lzf->strm.total_in_lo32;
        if (nbytes_in_hi32 != NULL)
            *nbytes_in_hi32 = lzf->strm.total_in_hi32;
            if (nbytes_out_lo32 != NULL)
                *nbytes_out_lo32 = lzf->strm.total_out_lo32;
                if (nbytes_out_hi32 != NULL)
                    *nbytes_out_hi32 = lzf->strm.total_out_hi32;
                    
                    LZ_SETERR(LZ_OK);
                    LZ4_lzCompressEnd ( &(lzf->strm) );
                    free ( lzf );
                    }


/*---------------------------------------------------*/
LZFILE* LZ_API(LZ4_lzReadOpen)
(int*  lzerror,
 FILE* f,
 int   verbosity,
 int   small,
 void* unused,
 int   nUnused )
{
    lzFile* lzf = NULL;
    int     ret;
    
    LZ_SETERR(LZ_OK);
    
    if (f == NULL ||
        (small != 0 && small != 1) ||
        (verbosity < 0 || verbosity > 4) ||
        (unused == NULL && nUnused != 0) ||
        (unused != NULL && (nUnused < 0 || nUnused > LZ_MAX_UNUSED)))
    { LZ_SETERR(LZ_PARAM_ERROR); return NULL; };
    
    if (ferror(f))
    { LZ_SETERR(LZ_IO_ERROR); return NULL; };
    
    lzf = malloc ( sizeof(lzFile) );
    if (lzf == NULL)
    { LZ_SETERR(LZ_MEM_ERROR); return NULL; };
    
    LZ_SETERR(LZ_OK);
    
    lzf->initialisedOk = False;
    lzf->handle        = f;
    lzf->bufN          = 0;
    lzf->writing       = False;
    lzf->strm.lzalloc  = NULL;
    lzf->strm.lzfree   = NULL;
    lzf->strm.opaque   = NULL;
    
    while (nUnused > 0) {
        lzf->buf[lzf->bufN] = *((UChar*)(unused)); lzf->bufN++;
        unused = ((void*)( 1 + ((UChar*)(unused))  ));
        nUnused--;
    }
    
    ret = LZ4_lzDecompressInit ( &(lzf->strm), verbosity, small );
    if (ret != LZ_OK)
    { LZ_SETERR(ret); free(lzf); return NULL; };
    
    lzf->strm.avail_in = lzf->bufN;
    lzf->strm.next_in  = lzf->buf;
    
    lzf->initialisedOk = True;
    return lzf;
}


/*---------------------------------------------------*/
void LZ_API(LZ4_lzReadClose) ( int *lzerror, LZFILE *b )
{
    lzFile* lzf = (lzFile*)b;
    
    LZ_SETERR(LZ_OK);
    if (lzf == NULL)
    { LZ_SETERR(LZ_OK); return; };
    
    if (lzf->writing)
    { LZ_SETERR(LZ_SEQUENCE_ERROR); return; };
    
    if (lzf->initialisedOk)
        (void)LZ4_lzDecompressEnd ( &(lzf->strm) );
        free ( lzf );
        }


/*---------------------------------------------------*/
int LZ_API(LZ4_lzRead) (int* lzerror, LZFILE* b, void* buf, int len)
{
    Int32   n, ret;
    lzFile* lzf = (lzFile*)b;
    
    LZ_SETERR(LZ_OK);
    
    if (lzf == NULL || buf == NULL || len < 0)
    { LZ_SETERR(LZ_PARAM_ERROR); return 0; };
    
    if (lzf->writing)
    { LZ_SETERR(LZ_SEQUENCE_ERROR); return 0; };
    
    if (len == 0)
    { LZ_SETERR(LZ_OK); return 0; };
    
    lzf->strm.avail_out = len;
    lzf->strm.next_out = buf;
    
    while (True) {
        
        if (ferror(lzf->handle))
        { LZ_SETERR(LZ_IO_ERROR); return 0; };
        
        if (lzf->strm.avail_in == 0 && !myfeof(lzf->handle)) {
            n = (Int32)fread ( lzf->buf, sizeof(UChar),
                              LZ_MAX_UNUSED, lzf->handle );
            if (ferror(lzf->handle))
            { LZ_SETERR(LZ_IO_ERROR); return 0; };
            lzf->bufN = n;
            lzf->strm.avail_in = lzf->bufN;
            lzf->strm.next_in = lzf->buf;
        }
        
        ret = LZ4_lzDecompress ( &(lzf->strm) );
        
        if (ret != LZ_OK && ret != LZ_STREAM_END)
        { LZ_SETERR(ret); return 0; };
        
        if (ret == LZ_OK && myfeof(lzf->handle) &&
            lzf->strm.avail_in == 0 && lzf->strm.avail_out > 0)
        { LZ_SETERR(LZ_UNEXPECTED_EOF); return 0; };
        
        if (ret == LZ_STREAM_END)
        { LZ_SETERR(LZ_STREAM_END);
            return len - lzf->strm.avail_out; };
        if (lzf->strm.avail_out == 0)
        { LZ_SETERR(LZ_OK); return len; };
        
    }
    
    return 0; /*not reached*/
}


/*---------------------------------------------------*/
void LZ_API(LZ4_lzReadGetUnused)
(int*    lzerror,
 LZFILE* b,
 void**  unused,
 int*    nUnused )
{
//    lzFile* lzf = (lzFile*)b;
//    if (lzf == NULL)
//    { LZ_SETERR(LZ_PARAM_ERROR); return; };
//    if (lzf->lastErr != LZ_STREAM_END)
//    { LZ_SETERR(LZ_SEQUENCE_ERROR); return; };
//    if (unused == NULL || nUnused == NULL)
//    { LZ_SETERR(LZ_PARAM_ERROR); return; };
//    
//    LZ_SETERR(LZ_OK);
//    *nUnused = lzf->strm.avail_in;
//    *unused = lzf->strm.next_in;
}
#endif


/*---------------------------------------------------*/
/*--- Misc convenience stuff                      ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int LZ_API(LZ4_lzBuffToBuffCompress)
(char*         dest,
 unsigned int* destLen,
 char*         source,
 unsigned int  sourceLen,
 int           blockSize100k,
 int           verbosity,
 int           workFactor )
{
    lz_stream strm;
    int ret;
    
    if (dest == NULL || destLen == NULL ||
        source == NULL ||
        blockSize100k < 1 || blockSize100k > 9 ||
        verbosity < 0 || verbosity > 4 ||
        workFactor < 0 || workFactor > 250)
        return LZ_PARAM_ERROR;
    
    if (workFactor == 0) workFactor = 30;
        strm.lzalloc = NULL;
        strm.lzfree = NULL;
        strm.opaque = NULL;
        ret = LZ4_lzCompressInit ( &strm, blockSize100k,
                                  verbosity, workFactor );
        if (ret != LZ_OK) return ret;
    
    strm.next_in = source;
    strm.next_out = dest;
    strm.avail_in = sourceLen;
    strm.avail_out = *destLen;
    
    ret = LZ4_lzCompress ( &strm, LZ_FINISH );
    if (ret == LZ_FINISH_OK) goto output_overflow;
    if (ret != LZ_STREAM_END) goto errhandler;
    
    /* normal termination */
    *destLen -= strm.avail_out;
    LZ4_lzCompressEnd ( &strm );
    return LZ_OK;
    
output_overflow:
    LZ4_lzCompressEnd ( &strm );
    return LZ_OUTBUFF_FULL;
    
errhandler:
    LZ4_lzCompressEnd ( &strm );
    return ret;
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzBuffToBuffDecompress)
(char*         dest,
 unsigned int* destLen,
 char*         source,
 unsigned int  sourceLen,
 int           small,
 int           verbosity )
{
    lz_stream strm;
    int ret;
    
    if (dest == NULL || destLen == NULL ||
        source == NULL ||
        (small != 0 && small != 1) ||
        verbosity < 0 || verbosity > 4)
        return LZ_PARAM_ERROR;
    
    strm.lzalloc = NULL;
    strm.lzfree = NULL;
    strm.opaque = NULL;
    ret = LZ4_lzDecompressInit ( &strm, verbosity, small );
    if (ret != LZ_OK) return ret;
    
    strm.next_in = source;
    strm.next_out = dest;
    strm.avail_in = sourceLen;
    strm.avail_out = *destLen;
    
    ret = LZ4_lzDecompress ( &strm );
    if (ret == LZ_OK) goto output_overflow_or_eof;
    if (ret != LZ_STREAM_END) goto errhandler;
    
    /* normal termination */
    *destLen -= strm.avail_out;
    LZ4_lzDecompressEnd ( &strm );
    return LZ_OK;
    
output_overflow_or_eof:
    if (strm.avail_out > 0) {
        LZ4_lzDecompressEnd ( &strm );
        return LZ_UNEXPECTED_EOF;
    } else {
        LZ4_lzDecompressEnd ( &strm );
        return LZ_OUTBUFF_FULL;
    };
    
errhandler:
    LZ4_lzDecompressEnd ( &strm );
    return ret;
}


/*---------------------------------------------------*/
/*--
 Code contributed by Yoshioka Tsuneo (tsuneo@rr.iij4u.or.jp)
 to support better zlib compatibility.
 This code is not _officially_ part of liblzip2 (yet);
 I haven't tested it, documented it, or considered the
 threading-safeness of it.
 If this code breaks, please contact both Yoshioka and me.
 --*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/*--
 return version like "0.9.5d, 4-Sept-1999".
 --*/
const char * LZ_API(LZ4_lzlibVersion)(void)
{
    return LZ_VERSION;
}


#ifndef LZ_NO_STDIO
/*---------------------------------------------------*/

#if defined(_WIN32) || defined(OS2) || defined(MSDOS)
#   include <fcntl.h>
#   include <io.h>
#   define SET_BINARY_MODE(file) setmode(fileno(file),O_BINARY)
#else
#   define SET_BINARY_MODE(file)
#endif
static
LZFILE * lzopen_or_lzdopen
(const char *path,   /* no use when lzdopen */
 int fd,             /* no use when lzdopen */
 const char *mode,
 int open_mode)      /* lzopen: 0, lzdopen:1 */
{
    int    lzerr;
    char   unused[LZ_MAX_UNUSED];
    int    blockSize100k = 9;
    int    writing       = 0;
    char   mode2[10]     = "";
    FILE   *fp           = NULL;
    LZFILE *lzfp         = NULL;
    int    verbosity     = 0;
    int    workFactor    = 30;
    int    smallMode     = 0;
    int    nUnused       = 0;
    
    if (mode == NULL) return NULL;
    while (*mode) {
        switch (*mode) {
            case 'r':
                writing = 0; break;
            case 'w':
                writing = 1; break;
            case 's':
                smallMode = 1; break;
            default:
                if (isdigit((int)(*mode))) {
                    blockSize100k = *mode-LZ_HDR_0;
                }
        }
        mode++;
    }
    strcat(mode2, writing ? "w" : "r" );
    strcat(mode2,"b");   /* binary mode */
    
    if (open_mode==0) {
        if (path==NULL || strcmp(path,"")==0) {
            fp = (writing ? stdout : stdin);
            SET_BINARY_MODE(fp);
        } else {
            fp = fopen(path,mode2);
        }
    } else {
#ifdef LZ_STRICT_ANSI
        fp = NULL;
#else
        fp = fdopen(fd,mode2);
#endif
    }
    if (fp == NULL) return NULL;
    
    if (writing) {
        /* Guard against total chaos and anarchy -- JRS */
        if (blockSize100k < 1) blockSize100k = 1;
        if (blockSize100k > 9) blockSize100k = 9;
        lzfp = LZ4_lzWriteOpen(&lzerr,fp,blockSize100k,
                               verbosity,workFactor);
    } else {
        lzfp = LZ4_lzReadOpen(&lzerr,fp,verbosity,smallMode,
                              unused,nUnused);
    }
    if (lzfp == NULL) {
        if (fp != stdin && fp != stdout) fclose(fp);
        return NULL;
    }
    return lzfp;
}


/*---------------------------------------------------*/
/*--
 open file for read or write.
 ex) lzopen("file","w9")
 case path="" or NULL => use stdin or stdout.
 --*/
LZFILE * LZ_API(LZ4_lzopen) (const char *path, const char *mode)
{
    return lzopen_or_lzdopen(path,-1,mode,/*lzopen*/0);
}


/*---------------------------------------------------*/
LZFILE * LZ_API(LZ4_lzdopen) (int fd, const char *mode)
{
    return lzopen_or_lzdopen(NULL,fd,mode,/*lzdopen*/1);
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzread) (LZFILE* b, void* buf, int len)
{
    int lzerr, nread;
    if (((lzFile*)b)->lastErr == LZ_STREAM_END) return 0;
    nread = LZ4_lzRead(&lzerr,b,buf,len);
    if (lzerr == LZ_OK || lzerr == LZ_STREAM_END) {
        return nread;
    } else {
        return -1;
    }
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzwrite) (LZFILE* b, void* buf, int len )
{
    int lzerr;
    
    LZ4_lzWrite(&lzerr,b,buf,len);
    if(lzerr == LZ_OK){
        return len;
    }else{
        return -1;
    }
}


/*---------------------------------------------------*/
int LZ_API(LZ4_lzflush) (LZFILE *b)
{
    /* do nothing now... */
    return 0;
}


/*---------------------------------------------------*/
void LZ_API(LZ4_lzclose) (LZFILE* b)
{
    int lzerr;
    FILE *fp;
    
    if (b==NULL) {return;}
    fp = ((lzFile *)b)->handle;
    if(((lzFile*)b)->writing){
        LZ4_lzWriteClose(&lzerr,b,0,NULL,NULL);
        if(lzerr != LZ_OK){
            LZ4_lzWriteClose(NULL,b,1,NULL,NULL);
        }
    }else{
        LZ4_lzReadClose(&lzerr,b);
    }
    if(fp!=stdin && fp!=stdout){
        fclose(fp);
    }
}


/*---------------------------------------------------*/
/*--
 return last error code 
 --*/
static const char *lzerrorstrings[] = {
    "OK"
    ,"SEQUENCE_ERROR"
    ,"PARAM_ERROR"
    ,"MEM_ERROR"
    ,"DATA_ERROR"
    ,"DATA_ERROR_MAGIC"
    ,"IO_ERROR"
    ,"UNEXPECTED_EOF"
    ,"OUTBUFF_FULL"
    ,"CONFIG_ERROR"
    ,"???"   /* for future */
    ,"???"   /* for future */
    ,"???"   /* for future */
    ,"???"   /* for future */
    ,"???"   /* for future */
    ,"???"   /* for future */
};


const char * LZ_API(LZ4_lzerror) (LZFILE *b, int *errnum)
{
    int err = ((lzFile *)b)->lastErr;
    
    if(err>0) err = 0;
        *errnum = err;
        return lzerrorstrings[err*-1];
}
#endif


/*-------------------------------------------------------------*/
/*--- end                                           lzlib.c ---*/
/*-------------------------------------------------------------*/
