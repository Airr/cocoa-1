
/*-------------------------------------------------------------*/
/*--- Private header file for the library.                  ---*/
/*---                                       lzlib_private.h ---*/
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


#ifndef _LZLIB_PRIVATE_H
#define _LZLIB_PRIVATE_H

#include <stdlib.h>

#ifndef LZ_NO_STDIO
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#endif

/*-- General stuff. --*/

#define LZ_VERSION  "*** LZ4 Compression CLI 64-bits r112, by Yann Collet (Jan 18 2014) ***"

typedef char            Char;
typedef unsigned char   Bool;
typedef unsigned char   UChar;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef short           Int16;
typedef unsigned short  UInt16;

#define True  ((Bool)1)
#define False ((Bool)0)

#ifndef __GNUC__
#define __inline__  /* */
#endif

#ifndef LZ_NO_STDIO

extern void LZ4_lz__AssertH__fail ( int errcode );
#define AssertH(cond,errcode) \
{ if (!(cond)) LZ4_lz__AssertH__fail ( errcode ); }

#if LZ_DEBUG
#define AssertD(cond,msg) \
{ if (!(cond)) {       \
fprintf ( stderr,   \
"\n\nliblzip2(debug build): internal error\n\t%s\n", msg );\
exit(1); \
}}
#else
#define AssertD(cond,msg) /* */
#endif

#define VPrintf0(zf) \
fprintf(stderr,zf)
#define VPrintf1(zf,za1) \
fprintf(stderr,zf,za1)
#define VPrintf2(zf,za1,za2) \
fprintf(stderr,zf,za1,za2)
#define VPrintf3(zf,za1,za2,za3) \
fprintf(stderr,zf,za1,za2,za3)
#define VPrintf4(zf,za1,za2,za3,za4) \
fprintf(stderr,zf,za1,za2,za3,za4)
#define VPrintf5(zf,za1,za2,za3,za4,za5) \
fprintf(stderr,zf,za1,za2,za3,za4,za5)

#else

extern void lz_internal_error ( int errcode );
#define AssertH(cond,errcode) \
{ if (!(cond)) lz_internal_error ( errcode ); }
#define AssertD(cond,msg)                do { } while (0)
#define VPrintf0(zf)                     do { } while (0)
#define VPrintf1(zf,za1)                 do { } while (0)
#define VPrintf2(zf,za1,za2)             do { } while (0)
#define VPrintf3(zf,za1,za2,za3)         do { } while (0)
#define VPrintf4(zf,za1,za2,za3,za4)     do { } while (0)
#define VPrintf5(zf,za1,za2,za3,za4,za5) do { } while (0)

#endif


#define LZALLOC(nnn) (strm->lzalloc)(strm->opaque,(nnn),1)
#define LZFREE(ppp)  (strm->lzfree)(strm->opaque,(ppp))


/*-- Header bytes. --*/

#define LZ_HDR_B 0x42   /* 'B' */
#define LZ_HDR_Z 0x5a   /* 'Z' */
#define LZ_HDR_h 0x68   /* 'h' */
#define LZ_HDR_0 0x30   /* '0' */

/*-- Constants for the back end. --*/

#define LZ_MAX_ALPHA_SIZE 258
#define LZ_MAX_CODE_LEN    23

#define LZ_RUNA 0
#define LZ_RUNB 1

#define LZ_N_GROUPS 6
#define LZ_G_SIZE   50
#define LZ_N_ITERS  4

#define LZ_MAX_SELECTORS (2 + (900000 / LZ_G_SIZE))



/*-- Stuff for randomising repetitive blocks. --*/

extern Int32 LZ4_rNums[512];

#define LZ_RAND_DECLS                          \
Int32 rNToGo;                               \
Int32 rTPos                                 \

#define LZ_RAND_INIT_MASK                      \
s->rNToGo = 0;                              \
s->rTPos  = 0                               \

#define LZ_RAND_MASK ((s->rNToGo == 1) ? 1 : 0)

#define LZ_RAND_UPD_MASK                       \
if (s->rNToGo == 0) {                       \
s->rNToGo = LZ4_rNums[s->rTPos];         \
s->rTPos++;                              \
if (s->rTPos == 512) s->rTPos = 0;       \
}                                           \
s->rNToGo--;



/*-- Stuff for doing CRCs. --*/

extern UInt32 LZ4_crc32Table[256];

#define LZ_INITIALISE_CRC(crcVar)              \
{                                              \
crcVar = 0xffffffffL;                       \
}

#define LZ_FINALISE_CRC(crcVar)                \
{                                              \
crcVar = ~(crcVar);                         \
}

#define LZ_UPDATE_CRC(crcVar,cha)              \
{                                              \
crcVar = (crcVar << 8) ^                    \
LZ4_crc32Table[(crcVar >> 24) ^    \
((UChar)cha)];      \
}



/*-- States and modes for compression. --*/

#define LZ_M_IDLE      1
#define LZ_M_RUNNING   2
#define LZ_M_FLUSHING  3
#define LZ_M_FINISHING 4

#define LZ_S_OUTPUT    1
#define LZ_S_INPUT     2

#define LZ_N_RADIX 2
#define LZ_N_QSORT 12
#define LZ_N_SHELL 18
#define LZ_N_OVERSHOOT (LZ_N_RADIX + LZ_N_QSORT + LZ_N_SHELL + 2)




/*-- Structure holding all the compression-side stuff. --*/

typedef
struct {
    /* pointer back to the struct lz_stream */
    lz_stream* strm;
    
    /* mode this stream is in, and whether inputting */
    /* or outputting data */
    Int32    mode;
    Int32    state;
    
    /* remembers avail_in when flush/finish requested */
    UInt32   avail_in_expect;
    
    /* for doing the block sorting */
    UInt32*  arr1;
    UInt32*  arr2;
    UInt32*  ftab;
    Int32    origPtr;
    
    /* aliases for arr1 and arr2 */
    UInt32*  ptr;
    UChar*   block;
    UInt16*  mtfv;
    UChar*   zbits;
    
    /* for deciding when to use the fallback sorting algorithm */
    Int32    workFactor;
    
    /* run-length-encoding of the input */
    UInt32   state_in_ch;
    Int32    state_in_len;
    LZ_RAND_DECLS;
    
    /* input and output limits and current posns */
    Int32    nblock;
    Int32    nblockMAX;
    Int32    numZ;
    Int32    state_out_pos;
    
    /* map of bytes used in block */
    Int32    nInUse;
    Bool     inUse[256];
    UChar    unseqToSeq[256];
    
    /* the buffer for bit stream creation */
    UInt32   bsBuff;
    Int32    bsLive;
    
    /* block and combined CRCs */
    UInt32   blockCRC;
    UInt32   combinedCRC;
    
    /* misc administratium */
    Int32    verbosity;
    Int32    blockNo;
    Int32    blockSize100k;
    
    /* stuff for coding the MTF values */
    Int32    nMTF;
    Int32    mtfFreq    [LZ_MAX_ALPHA_SIZE];
    UChar    selector   [LZ_MAX_SELECTORS];
    UChar    selectorMtf[LZ_MAX_SELECTORS];
    
    UChar    len     [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    Int32    code    [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    Int32    rfreq   [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    /* second dimension: only 3 needed; 4 makes index calculations faster */
    UInt32   len_pack[LZ_MAX_ALPHA_SIZE][4];
    
}
EState;



/*-- externs for compression. --*/

extern void
LZ4_blockSort ( EState* );

extern void
LZ4_compressBlock ( EState*, Bool );

extern void
LZ4_bsInitWrite ( EState* );

extern void
LZ4_hbAssignCodes ( Int32*, UChar*, Int32, Int32, Int32 );

extern void
LZ4_hbMakeCodeLengths ( UChar*, Int32*, Int32, Int32 );



/*-- states for decompression. --*/

#define LZ_X_IDLE        1
#define LZ_X_OUTPUT      2

#define LZ_X_MAGIC_1     10
#define LZ_X_MAGIC_2     11
#define LZ_X_MAGIC_3     12
#define LZ_X_MAGIC_4     13
#define LZ_X_BLKHDR_1    14
#define LZ_X_BLKHDR_2    15
#define LZ_X_BLKHDR_3    16
#define LZ_X_BLKHDR_4    17
#define LZ_X_BLKHDR_5    18
#define LZ_X_BLKHDR_6    19
#define LZ_X_BCRC_1      20
#define LZ_X_BCRC_2      21
#define LZ_X_BCRC_3      22
#define LZ_X_BCRC_4      23
#define LZ_X_RANDBIT     24
#define LZ_X_ORIGPTR_1   25
#define LZ_X_ORIGPTR_2   26
#define LZ_X_ORIGPTR_3   27
#define LZ_X_MAPPING_1   28
#define LZ_X_MAPPING_2   29
#define LZ_X_SELECTOR_1  30
#define LZ_X_SELECTOR_2  31
#define LZ_X_SELECTOR_3  32
#define LZ_X_CODING_1    33
#define LZ_X_CODING_2    34
#define LZ_X_CODING_3    35
#define LZ_X_MTF_1       36
#define LZ_X_MTF_2       37
#define LZ_X_MTF_3       38
#define LZ_X_MTF_4       39
#define LZ_X_MTF_5       40
#define LZ_X_MTF_6       41
#define LZ_X_ENDHDR_2    42
#define LZ_X_ENDHDR_3    43
#define LZ_X_ENDHDR_4    44
#define LZ_X_ENDHDR_5    45
#define LZ_X_ENDHDR_6    46
#define LZ_X_CCRC_1      47
#define LZ_X_CCRC_2      48
#define LZ_X_CCRC_3      49
#define LZ_X_CCRC_4      50



/*-- Constants for the fast MTF decoder. --*/

#define MTFA_SIZE 4096
#define MTFL_SIZE 16



/*-- Structure holding all the decompression-side stuff. --*/

typedef
struct {
    /* pointer back to the struct lz_stream */
    lz_stream* strm;
    
    /* state indicator for this stream */
    Int32    state;
    
    /* for doing the final run-length decoding */
    UChar    state_out_ch;
    Int32    state_out_len;
    Bool     blockRandomised;
    LZ_RAND_DECLS;
    
    /* the buffer for bit stream reading */
    UInt32   bsBuff;
    Int32    bsLive;
    
    /* misc administratium */
    Int32    blockSize100k;
    Bool     smallDecompress;
    Int32    currBlockNo;
    Int32    verbosity;
    
    /* for undoing the Burrows-Wheeler transform */
    Int32    origPtr;
    UInt32   tPos;
    Int32    k0;
    Int32    unzftab[256];
    Int32    nblock_used;
    Int32    cftab[257];
    Int32    cftabCopy[257];
    
    /* for undoing the Burrows-Wheeler transform (FAST) */
    UInt32   *tt;
    
    /* for undoing the Burrows-Wheeler transform (SMALL) */
    UInt16   *ll16;
    UChar    *ll4;
    
    /* stored and calculated CRCs */
    UInt32   storedBlockCRC;
    UInt32   storedCombinedCRC;
    UInt32   calculatedBlockCRC;
    UInt32   calculatedCombinedCRC;
    
    /* map of bytes used in block */
    Int32    nInUse;
    Bool     inUse[256];
    Bool     inUse16[16];
    UChar    seqToUnseq[256];
    
    /* for decoding the MTF values */
    UChar    mtfa   [MTFA_SIZE];
    Int32    mtfbase[256 / MTFL_SIZE];
    UChar    selector   [LZ_MAX_SELECTORS];
    UChar    selectorMtf[LZ_MAX_SELECTORS];
    UChar    len  [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    
    Int32    limit  [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    Int32    base   [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    Int32    perm   [LZ_N_GROUPS][LZ_MAX_ALPHA_SIZE];
    Int32    minLens[LZ_N_GROUPS];
    
    /* save area for scalars in the main decompress code */
    Int32    save_i;
    Int32    save_j;
    Int32    save_t;
    Int32    save_alphaSize;
    Int32    save_nGroups;
    Int32    save_nSelectors;
    Int32    save_EOB;
    Int32    save_groupNo;
    Int32    save_groupPos;
    Int32    save_nextSym;
    Int32    save_nblockMAX;
    Int32    save_nblock;
    Int32    save_es;
    Int32    save_N;
    Int32    save_curr;
    Int32    save_zt;
    Int32    save_zn;
    Int32    save_zvec;
    Int32    save_zj;
    Int32    save_gSel;
    Int32    save_gMinlen;
    Int32*   save_gLimit;
    Int32*   save_gBase;
    Int32*   save_gPerm;
    
}
DState;



/*-- Macros for decompression. --*/

#define LZ_GET_FAST(cccc)                     \
/* c_tPos is unsigned, hence test < 0 is pointless. */ \
if (s->tPos >= (UInt32)100000 * (UInt32)s->blockSize100k) return True; \
s->tPos = s->tt[s->tPos];                 \
cccc = (UChar)(s->tPos & 0xff);           \
s->tPos >>= 8;

#define LZ_GET_FAST_C(cccc)                   \
/* c_tPos is unsigned, hence test < 0 is pointless. */ \
if (c_tPos >= (UInt32)100000 * (UInt32)ro_blockSize100k) return True; \
c_tPos = c_tt[c_tPos];                    \
cccc = (UChar)(c_tPos & 0xff);            \
c_tPos >>= 8;

#define SET_LL4(i,n)                                          \
{ if (((i) & 0x1) == 0)                                    \
s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0xf0) | (n); else    \
s->ll4[(i) >> 1] = (s->ll4[(i) >> 1] & 0x0f) | ((n) << 4);  \
}

#define GET_LL4(i)                             \
((((UInt32)(s->ll4[(i) >> 1])) >> (((i) << 2) & 0x4)) & 0xF)

#define SET_LL(i,n)                          \
{ s->ll16[i] = (UInt16)(n & 0x0000ffff);  \
SET_LL4(i, n >> 16);                    \
}

#define GET_LL(i) \
(((UInt32)s->ll16[i]) | (GET_LL4(i) << 16))

#define LZ_GET_SMALL(cccc)                            \
/* c_tPos is unsigned, hence test < 0 is pointless. */ \
if (s->tPos >= (UInt32)100000 * (UInt32)s->blockSize100k) return True; \
cccc = LZ4_indexIntoF ( s->tPos, s->cftab );    \
s->tPos = GET_LL(s->tPos);


/*-- externs for decompression. --*/

extern Int32
LZ4_indexIntoF ( Int32, Int32* );

extern Int32
LZ4_decompress ( DState* );

extern void
LZ4_hbCreateDecodeTables ( Int32*, Int32*, Int32*, UChar*,
                          Int32,  Int32, Int32 );


#endif


/*-- LZ_NO_STDIO seems to make NULL disappear on some platforms. --*/

#ifdef LZ_NO_STDIO
#ifndef NULL
#define NULL 0
#endif
#endif


/*-------------------------------------------------------------*/
/*--- end                                   lzlib_private.h ---*/
/*-------------------------------------------------------------*/
