/**
 * This file, alder_lz4.h, is part of alder-lz4.
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

#ifndef alder_lz4_alder_lz4_h
#define alder_lz4_alder_lz4_h

#include <stdint.h>

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

void alder_lz4_test(const char *fn);

#define LZ_RUN               0
#define LZ_FLUSH             1
#define LZ_FINISH            2

#define LZ_OK                0
#define LZ_RUN_OK            1
#define LZ_FLUSH_OK          2
#define LZ_FINISH_OK         3
#define LZ_STREAM_END        4
#define LZ_SEQUENCE_ERROR    (-1)
#define LZ_PARAM_ERROR       (-2)
#define LZ_MEM_ERROR         (-3)
#define LZ_DATA_ERROR        (-4)
#define LZ_DATA_ERROR_MAGIC  (-5)
#define LZ_IO_ERROR          (-6)
#define LZ_UNEXPECTED_EOF    (-7)
#define LZ_OUTBUFF_FULL      (-8)
#define LZ_CONFIG_ERROR      (-9)

typedef struct {
    char *next_in;
    unsigned int avail_in;
    unsigned int total_in_lo32;
    unsigned int total_in_hi32;
    
    char *next_out;
    unsigned int avail_out;
    unsigned int total_out_lo32;
    unsigned int total_out_hi32;
    
    void *state;
    
    void *(*lzalloc)(void *,int,int);
    void (*lzfree)(void *,void *);
    void *opaque;
} lz_stream;

#ifdef _WIN32
#   include <windows.h>
#   ifdef small
/* windows.h define small to char */
#      undef small
#   endif
#   ifdef LZ_EXPORT
#   define LZ_API(func) WINAPI func
#   define LZ_EXTERN extern
#   else
/* import windows dll dynamically */
#   define LZ_API(func) (WINAPI * func)
#   define LZ_EXTERN
#   endif
#else
#   define LZ_API(func) func
#   define LZ_EXTERN extern
#endif

/*-- High(er) level library functions --*/

#ifndef LZ_NO_STDIO
#define LZ_MAX_UNUSED 5000

typedef void LZFILE;

LZ_EXTERN LZFILE* LZ_API(LZ4_lzReadOpen) ( 
      int*  lzerror,   
      FILE* f, 
      int   verbosity, 
      int   small,
      void* unused,    
      int   nUnused 
   );

LZ_EXTERN void LZ_API(LZ4_lzReadClose) ( 
      int*    lzerror, 
      LZFILE* b 
   );

LZ_EXTERN void LZ_API(LZ4_lzReadGetUnused) ( 
      int*    lzerror, 
      LZFILE* b, 
      void**  unused,  
      int*    nUnused 
   );

LZ_EXTERN int LZ_API(LZ4_lzRead) ( 
      int*    lzerror, 
      LZFILE* b, 
      void*   buf, 
      int     len 
   );

LZ_EXTERN LZFILE* LZ_API(LZ4_lzWriteOpen) ( 
      int*  lzerror,      
      FILE* f, 
      int   blockSize100k, 
      int   verbosity, 
      int   workFactor 
   );

LZ_EXTERN void LZ_API(LZ4_lzWrite) ( 
      int*    lzerror, 
      LZFILE* b, 
      void*   buf, 
      int     len 
   );

LZ_EXTERN void LZ_API(LZ4_lzWriteClose) ( 
      int*          lzerror, 
      LZFILE*       b, 
      int           abandon, 
      unsigned int* nbytes_in, 
      unsigned int* nbytes_out 
   );

LZ_EXTERN void LZ_API(LZ4_lzWriteClose64) ( 
      int*          lzerror, 
      LZFILE*       b, 
      int           abandon, 
      unsigned int* nbytes_in_lo32, 
      unsigned int* nbytes_in_hi32, 
      unsigned int* nbytes_out_lo32, 
      unsigned int* nbytes_out_hi32
   );
#endif


/*--
   Code contributed by Yoshioka Tsuneo (tsuneo@rr.iij4u.or.jp)
   to support better zlib compatibility.
   This code is not _officially_ part of liblzip2 (yet);
   I haven't tested it, documented it, or considered the
   threading-safeness of it.
   If this code breaks, please contact both Yoshioka and me.
--*/

LZ_EXTERN const char * LZ_API(LZ4_lzlibVersion) (
      void
   );

#ifndef LZ_NO_STDIO
LZ_EXTERN LZFILE * LZ_API(LZ4_lzopen) (
      const char *path,
      const char *mode
   );

LZ_EXTERN LZFILE * LZ_API(LZ4_lzdopen) (
      int        fd,
      const char *mode
   );
         
LZ_EXTERN int LZ_API(LZ4_lzread) (
      LZFILE* b, 
      void* buf, 
      int len 
   );

LZ_EXTERN int LZ_API(LZ4_lzwrite) (
      LZFILE* b, 
      void*   buf, 
      int     len 
   );

LZ_EXTERN int LZ_API(LZ4_lzflush) (
      LZFILE* b
   );

LZ_EXTERN void LZ_API(LZ4_lzclose) (
      LZFILE* b
   );

LZ_EXTERN const char * LZ_API(LZ4_lzerror) (
      LZFILE *b, 
      int    *errnum
   );
#endif


__END_DECLS


#endif /* alder_lz4_alder_lz4_h */
