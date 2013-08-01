/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2013  The R Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/* <UTF8> byte-level access is only to compare with chars <= 0x7F */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define NEED_CONNECTION_PSTREAMS
#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <Rinterface.h>
#include <Rmath.h>
#include <Fileio.h>
#include <R_ext/RS.h>
#include <errno.h>
#include <ctype.h>		/* for isspace */

/* From time to time changes in R, such as the addition of a new SXP,
 * may require changes in the save file format.  Here are some
 * guidelines on handling format changes:
 *
 *    Starting with R 1.4.0 there is a version number associated with
 *    save file formats.  This version number should be incremented
 *    when the format is changed so older versions of R can recognize
 *    and reject the new format with a meaningful error message.
 *
 *    R should remain able to write older workspace formats.  An error
 *    should be signaled if the contents to be saved is not compatible
 *    with the requested format.
 *
 *    To allow older versions of R to give useful error messages, the
 *    header now contains the version of R that wrote the workspace
 *    and the oldest version that can read the workspace.  These
 *    versions are stored as an integer packed by the R_Version macro
 *    from Rversion.h.  Some workspace formats may only exist
 *    temporarily in the development stage.  If readers are not
 *    provided in a release version, then these should specify the
 *    oldest reader R version as -1.
 */

#define R_MAGIC_ASCII_V2   2001
#define R_MAGIC_BINARY_V2  2002
#define R_MAGIC_XDR_V2     2003
#define R_MAGIC_ASCII_V1   1001
#define R_MAGIC_BINARY_V1  1002
#define R_MAGIC_XDR_V1     1003
#define R_MAGIC_EMPTY      999
#define R_MAGIC_CORRUPT    998
#define R_MAGIC_MAYBE_TOONEW 997

/* pre-1 formats (R < 0.99.0) */
#define R_MAGIC_BINARY 1975
#define R_MAGIC_ASCII  1976
#define R_MAGIC_XDR    1977
#define R_MAGIC_BINARY_VERSION16 1971
#define R_MAGIC_ASCII_VERSION16	 1972


/* Static Globals, DIE, DIE, DIE! */


#include "RBufferUtils.h"

/* These are used by OffsetToNode & DataLoad.
 OffsetToNode is called by DataLoad() and RestoreSEXP()
 which itself is only called by RestoreSEXP.
 */
typedef struct {
 int NSymbol;		/* Number of symbols */
 int NSave;		/* Number of non-symbols */
 int NTotal;		/* NSymbol + NSave */
 int NVSize;		/* Number of vector cells */

 int *OldOffset;        /* Offsets in previous incarnation */

 SEXP NewAddress;       /* Addresses in this incarnation */
} NodeInfo;


#ifndef INT_32_BITS
/* The way XDR is used pretty much assumes that int is 32 bits and
   maybe even 2's complement representation--without that, NA_INTEGER
   is not likely to be preserved properly.  Since 32 bit ints (and 2's
   complement) are pretty much universal, we can worry about that when
   the need arises.  To be safe, we signal a compiler error if int is
   not 32 bits. There may be similar issues with doubles. */
*/
# error code requires that int have 32 bits
#endif


#include <rpc/types.h>
#include <rpc/xdr.h>

typedef struct {
/* These variables are accessed in the
   InInteger, InComplex, InReal, InString
   methods for Ascii, Binary, XDR.
   bufsize is only used in XdrInString!

The Ascii* routines could declare their own local
copy of smbuf and use that (non-static). That would
mean some of them wouldn't need the extra argument.
*/

    R_StringBuffer buffer;
    char smbuf[512];		/* Small buffer for temp use */
				/* smbuf is only used by Ascii. */
    XDR xdrs;
} SaveLoadData;


/* ----- I / O -- F u n c t i o n -- P o i n t e r s ----- */

typedef struct {
 void	(*OutInit)(FILE*, SaveLoadData *d);
 void	(*OutInteger)(FILE*, int, SaveLoadData *);
 void	(*OutReal)(FILE*, double, SaveLoadData *);
 void	(*OutComplex)(FILE*, Rcomplex, SaveLoadData *);
 void	(*OutString)(FILE*, const char*, SaveLoadData *);
 void	(*OutSpace)(FILE*, int, SaveLoadData *);
 void	(*OutNewline)(FILE*, SaveLoadData *);
 void	(*OutTerm)(FILE*, SaveLoadData *);
} OutputRoutines;

typedef struct {
 void	(*InInit)(FILE*, SaveLoadData *d);
 int	(*InInteger)(FILE*, SaveLoadData *);
 double	(*InReal)(FILE*, SaveLoadData *);
 Rcomplex	(*InComplex)(FILE*, SaveLoadData *);
 char*	(*InString)(FILE*, SaveLoadData *);
 void	(*InTerm)(FILE*, SaveLoadData *d);
} InputRoutines;

typedef struct {
  FILE *fp;
  OutputRoutines *methods;
  SaveLoadData *data;
} OutputCtxtData;

typedef struct {
  FILE *fp;
  InputRoutines *methods;
  SaveLoadData *data;
} InputCtxtData;


static SEXP DataLoad(FILE*, int startup, InputRoutines *m, int version, SaveLoadData *d);


/* ----- D u m m y -- P l a c e h o l d e r -- R o u t i n e s ----- */

static void DummyInit(FILE *fp, SaveLoadData *d)
{
  return;
}

static void DummyOutSpace(FILE *fp, int nspace, SaveLoadData *d)
{
  return;
}

static void DummyOutNewline(FILE *fp, SaveLoadData *d)
{
  return;
}

static void DummyTerm(FILE *fp, SaveLoadData *d)
{
  return;
}

/* ----- O l d - s t y l e  (p r e 1. 0)  R e s t o r e ----- */

/* This section is only used to load old-style workspaces / objects */


/* ----- L o w l e v e l -- A s c i i -- I / O ----- */

static int AsciiInInteger(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static double AsciiInReal(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static Rcomplex AsciiInComplex(FILE *fp, SaveLoadData *d)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}


static char *AsciiInString(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static SEXP AsciiLoad(FILE *fp, int startup, SaveLoadData *d)
{
  return 0;
}

static SEXP AsciiLoadOld(FILE *fp, int version, int startup, SaveLoadData *d)
{
  return 0;
}

/* ----- L o w l e v e l -- X D R -- I / O ----- */

static void XdrInInit(FILE *fp, SaveLoadData *d)
{
  return;
}

static void XdrInTerm(FILE *fp, SaveLoadData *d)
{
  return;
}

static int XdrInInteger(FILE * fp, SaveLoadData *d)
{
  return 0;
}

static double XdrInReal(FILE * fp, SaveLoadData *d)
{
  return 0;
}

static Rcomplex XdrInComplex(FILE * fp, SaveLoadData *d)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}

static char *XdrInString(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static SEXP XdrLoad(FILE *fp, int startup, SaveLoadData *d)
{
  return 0;
}


/* ----- L o w l e v e l -- B i n a r y -- I / O ----- */

static int BinaryInInteger(FILE * fp, SaveLoadData *unused)
{
  return 0;
}

static double BinaryInReal(FILE * fp, SaveLoadData *unused)
{
  return 0;
}

static Rcomplex BinaryInComplex(FILE * fp, SaveLoadData *unused)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}

static char *BinaryInString(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static SEXP BinaryLoad(FILE *fp, int startup, SaveLoadData *d)
{
  return 0;
}

static SEXP BinaryLoadOld(FILE *fp, int version, int startup, SaveLoadData *d)
{
  return 0;
}

static SEXP OffsetToNode(int offset, NodeInfo *node)
{
  return 0;
}

static unsigned int FixupType(unsigned int type, int VersionId)
{
  return 0;
}

static void RemakeNextSEXP(FILE *fp, NodeInfo *node, int version, InputRoutines *m, SaveLoadData *d)
{
  return;
}

static void RestoreSEXP(SEXP s, FILE *fp, InputRoutines *m, NodeInfo *node, int version, SaveLoadData *d)
{
  return;
}

static void RestoreError(/* const */ char *msg, int startup)
{
  return;
}

/* used for pre-version 1 formats */
static SEXP DataLoad(FILE *fp, int startup, InputRoutines *m,
		     int version, SaveLoadData *d)
{
  return 0;
}


/* ----- V e r s i o n -- O n e -- S a v e / R e s t o r e ----- */

/*  Code Developed by  Chris K. Young <cky@pobox.com>
 *  and Ross Ihaka for Chris' Honours project -- 1999.
 *  Copyright Assigned to the R Project.
 */

/*  An assert function which doesn't crash the program.
 *  Something like this might be useful in an R header file
 */

#ifdef NDEBUG
#define R_assert(e) ((void) 0)
#else
/* The line below requires an ANSI C preprocessor (stringify operator) */
#define R_assert(e) ((e) ? (void) 0 : error("assertion `%s' failed: file `%s', line %d\n", #e, __FILE__, __LINE__))
#endif /* NDEBUG */


static void NewWriteItem (SEXP s, SEXP sym_list, SEXP env_list, FILE *fp, OutputRoutines *, SaveLoadData *);
static SEXP NewReadItem (SEXP sym_table, SEXP env_table, FILE *fp, InputRoutines *, SaveLoadData *);


/*  We use special (negative) type codes to indicate the special
 *  values: R_NilValue, R_GlobalEnv, R_UnboundValue, R_MissingArg.
 *  The following routines handle these conversions (both
 *  directions). */

static int NewSaveSpecialHook (SEXP item)
{
  return 0;
}

static SEXP NewLoadSpecialHook (SEXPTYPE type)
{
  return 0;
}


/*  If "item" is a special value (as defined in "NewSaveSpecialHook")
 *  then a negative value is returned.
 *
 *  If "item" is present in "list" the a positive value is returned
 *  (the 1-based offset into the list).
 *
 *   Otherwise, a value of zero is returned.
 *
 *  The "list" is managed with a hash table.  This results in
 *  significant speedups for saving large amounts of code.  A fixed
 *  hash table size is used; this is not ideal but seems adequate for
 *  now.  The hash table representation consists of a (list . vector)
 *  pair.  The hash buckets are in the vector.  The list holds the
 *  list of keys.  This list is in reverse order to the way the keys
 *  were added (i.e. the most recently added key is first).  The
 *  indices produced by HashAdd are in order.  Since the list is
 *  written out in order, we either have to reverse the list or
 *  reverse the indices; to retain byte for byte compatibility the
 *  function FixHashEntries reverses the indices.  FixHashEntries must
 *  be called after filling the tables and before using them to find
 *  indices.  LT */

#define HASHSIZE 1099

#define PTRHASH(obj) (((R_size_t) (obj)) >> 2)

#define HASH_TABLE_KEYS_LIST(ht) CAR(ht)
#define SET_HASH_TABLE_KEYS_LIST(ht, v) SETCAR(ht, v)

#define HASH_TABLE_COUNT(ht) TRUELENGTH(CDR(ht))
#define SET_HASH_TABLE_COUNT(ht, val) SET_TRUELENGTH(CDR(ht), val)

#define HASH_TABLE_SIZE(ht) LENGTH(CDR(ht))

#define HASH_BUCKET(ht, pos) VECTOR_ELT(CDR(ht), pos)
#define SET_HASH_BUCKET(ht, pos, val) SET_VECTOR_ELT(CDR(ht), pos, val)

static SEXP MakeHashTable(void)
{
  return 0;
}

static void FixHashEntries(SEXP ht)
{
  return;
}

static void HashAdd(SEXP obj, SEXP ht)
{
  return;
}

static int HashGet(SEXP item, SEXP ht)
{
  return 0;
}

static int NewLookup (SEXP item, SEXP ht)
{
  return 0;
}

/*  This code carries out the basic inspection of an object, building
 *  the tables of symbols and environments.
 *
 *  We don't really need to build a table of symbols here, but it does
 *  prevent repeated "install"s.  On the other hand there will generally
 *  be huge delays because of disk or network latency ...
 *
 *  CKY: One thing I've found out is that you have to build all the
 *  lists together or you risk getting infinite loops.  Of course, the
 *  method used here somehow shoots functional programming in the
 *  head --- sorry.  */

static void NewMakeLists (SEXP obj, SEXP sym_list, SEXP env_list)
{
  return;
}

/* e.g., OutVec(fp, obj, INTEGER, OutInteger)
 The passMethods argument tells it whether to call outfunc with the
 other methods. This is only needed when calling OutCHARSXP
 since it needs to know how to write sub-elements!
*/
#define OutVec(fp, obj, accessor, outfunc, methods, d)	                \
	do {								\
		int cnt;						\
		for (cnt = 0; cnt < LENGTH(obj); ++cnt) {		\
			methods->OutSpace(fp, 1,d);			\
			outfunc(fp, accessor(obj, cnt), d);	        \
			methods->OutNewline(fp, d);                     \
		}							\
	} while (0)

#define LOGICAL_ELT(x,__i__)	LOGICAL(x)[__i__]
#define INTEGER_ELT(x,__i__)	INTEGER(x)[__i__]
#define REAL_ELT(x,__i__)	REAL(x)[__i__]
#define COMPLEX_ELT(x,__i__)	COMPLEX(x)[__i__]

/* Simply outputs the string associated with a CHARSXP, one day this
 * will handle null characters in CHARSXPs and not just blindly call
 * OutString.  */
static void OutCHARSXP (FILE *fp, SEXP s, OutputRoutines *m, SaveLoadData *d)
{
  return;
}

static void NewWriteVec (SEXP s, SEXP sym_list, SEXP env_list, FILE *fp, OutputRoutines *m, SaveLoadData *d)
{
  return;
}

static void NewWriteItem (SEXP s, SEXP sym_list, SEXP env_list, FILE *fp, OutputRoutines *m, SaveLoadData *d)
{
  return;
}

/*  General format: the total number of symbols, then the total number
 *  of environments.  Then all the symbol names get written out,
 *  followed by the environments, then the items to be saved.  If
 *  symbols or environments are encountered, references to them are
 *  made instead of writing them out totally.  */

static void newdatasave_cleanup(void *data)
{
  return;
}

static void NewDataSave (SEXP s, FILE *fp, OutputRoutines *m, SaveLoadData *d)
{
  return;
}

#define InVec(fp, obj, accessor, infunc, length, d)			\
	do {								\
		int cnt;						\
		for (cnt = 0; cnt < length; ++cnt)		\
			accessor(obj, cnt, infunc(fp, d));		\
	} while (0)



#define SET_LOGICAL_ELT(x,__i__,v)	(LOGICAL_ELT(x,__i__)=(v))
#define SET_INTEGER_ELT(x,__i__,v)	(INTEGER_ELT(x,__i__)=(v))
#define SET_REAL_ELT(x,__i__,v)		(REAL_ELT(x,__i__)=(v))
#define SET_COMPLEX_ELT(x,__i__,v)	(COMPLEX_ELT(x,__i__)=(v))

static SEXP InCHARSXP (FILE *fp, InputRoutines *m, SaveLoadData *d)
{
  return 0;
}

static SEXP NewReadVec(SEXPTYPE type, SEXP sym_table, SEXP env_table, FILE *fp, InputRoutines *m, SaveLoadData *d)
{
  return 0;
}

static SEXP NewReadItem (SEXP sym_table, SEXP env_table, FILE *fp,
			 InputRoutines *m, SaveLoadData *d)
{
  return 0;
}

static void newdataload_cleanup(void *data)
{
  return;
}

static SEXP NewDataLoad (FILE *fp, InputRoutines *m, SaveLoadData *d)
{
  return 0;
}

/* ----- L o w l e v e l -- A s c i i -- I / O ------ */

static void OutSpaceAscii(FILE *fp, int nspace, SaveLoadData *unused)
{
  return;
}
static void OutNewlineAscii(FILE *fp, SaveLoadData *unused)
{
  return;
}

static void OutIntegerAscii(FILE *fp, int x, SaveLoadData *unused)
{
  return;
}

static int InIntegerAscii(FILE *fp, SaveLoadData *unused)
{
  return 0;
}

static void OutStringAscii(FILE *fp, const char *x, SaveLoadData *unused)
{
  return;
}

static char *InStringAscii(FILE *fp, SaveLoadData *unused)
{
  return 0;
}

static void OutDoubleAscii(FILE *fp, double x, SaveLoadData *unused)
{
  return;
}

static double InDoubleAscii(FILE *fp, SaveLoadData *unused)
{
  return 0;
}

static void OutComplexAscii(FILE *fp, Rcomplex x, SaveLoadData *unused)
{
  return;
}

static Rcomplex InComplexAscii(FILE *fp, SaveLoadData *unused)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}

static void NewAsciiSave(SEXP s, FILE *fp, SaveLoadData *d)
{
  return;
}

static SEXP NewAsciiLoad(FILE *fp, SaveLoadData *d)
{
  return 0;
}

/* ----- L o w l e v e l -- B i n a r y -- I / O ----- */

static int InIntegerBinary(FILE * fp, SaveLoadData *unused)
{
  return 0;
}

static char *InStringBinary(FILE *fp, SaveLoadData *unused)
{
  return 0;
}

static double InRealBinary(FILE * fp, SaveLoadData *unused)
{
  return 0;
}

static Rcomplex InComplexBinary(FILE * fp, SaveLoadData *unused)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}

static SEXP NewBinaryLoad(FILE *fp, SaveLoadData *d)
{
  return 0;
}


/* ----- L o w l e v e l -- X D R -- I / O ----- */

static void InInitXdr(FILE *fp, SaveLoadData *d)
{
  return;
}

static void OutInitXdr(FILE *fp, SaveLoadData *d)
{
  return;
}

static void InTermXdr(FILE *fp, SaveLoadData *d)
{
  return;
}

static void OutTermXdr(FILE *fp, SaveLoadData *d)
{
  return;
}

static void OutIntegerXdr(FILE *fp, int i, SaveLoadData *d)
{
  return;
}

static int InIntegerXdr(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static void OutStringXdr(FILE *fp, const char *s, SaveLoadData *d)
{
  return;
}

static char *InStringXdr(FILE *fp, SaveLoadData *d)
{
  return 0;
}

static void OutRealXdr(FILE *fp, double x, SaveLoadData *d)
{
  return;
}

static double InRealXdr(FILE * fp, SaveLoadData *d)
{
  return 0;
}

static void OutComplexXdr(FILE *fp, Rcomplex x, SaveLoadData *d)
{
  return;
}

static Rcomplex InComplexXdr(FILE * fp, SaveLoadData *d)
{
    Rcomplex v = { 0.0, 0.0 };
    return v;
}

static void NewXdrSave(SEXP s, FILE *fp, SaveLoadData *d)
{
  return;
}

static SEXP NewXdrLoad(FILE *fp, SaveLoadData *d)
{
  return 0;
}


/* ----- F i l e -- M a g i c -- N u m b e r s ----- */

static void R_WriteMagic(FILE *fp, int number)
{
  return;
}

static int R_ReadMagic(FILE *fp)
{
  return 0;
}

static int R_DefaultSaveFormatVersion = 2;

/* ----- E x t e r n a l -- I n t e r f a c e s ----- */

void attribute_hidden R_SaveToFileV(SEXP obj, FILE *fp, int ascii, int version)
{
  return;
}

void attribute_hidden R_SaveToFile(SEXP obj, FILE *fp, int ascii)
{
  return;
}

    /* different handling of errors */

#define return_and_free(X) {r = X; R_FreeStringBuffer(&data.buffer); return r;}
SEXP attribute_hidden R_LoadFromFile(FILE *fp, int startup)
{
  return 0;
}

static void saveload_cleanup(void *data)
{
  return;
}

/* Only used for version 1 saves */
SEXP attribute_hidden do_save(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

static SEXP RestoreToEnv(SEXP ans, SEXP aenv)
{
  return 0;
}

static SEXP R_LoadSavedData(FILE *fp, SEXP aenv)
{
  return 0;
}

/* This is only used for version 1 or earlier formats */
SEXP attribute_hidden do_load(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* defined in Rinternals.h
#define R_XDR_DOUBLE_SIZE 8
#define R_XDR_INTEGER_SIZE 4
*/

void attribute_hidden R_XDREncodeDouble(double d, void *buf)
{
  return;
}

double attribute_hidden R_XDRDecodeDouble(void *buf)
{
  return 0;
}

void attribute_hidden R_XDREncodeInteger(int i, void *buf)
{
  return;
}

int attribute_hidden R_XDRDecodeInteger(void *buf)
{
  return 0;
}

/* Next two were used in gnomeGUI package, are in Rinterface.h  */
void R_SaveGlobalEnvToFile(const char *name)
{
  return;
}

void R_RestoreGlobalEnvFromFile(const char *name, Rboolean quiet)
{
  return;
}


#include <Rconnections.h>

static void con_cleanup(void *data)
{
  return;
}


/* Ideally it should be possible to do this entirely in R code with
   something like

	magic <- if (ascii) "RDA2\n" else ...
	writeChar(magic, con, eos = NULL)
	val <- lapply(list, get, envir = envir)
	names(val) <- list
	invisible(serialize(val, con, ascii = ascii))

   Unfortunately, this will result in too much duplication in the lapply
   (and any other way of doing this).  Hence we need an internal version. 

   In case anyone wants to do this another way, in fact it is a
   pairlist of objects that is serialized, but RestoreToEnv copes
   with either a pairlist or list.
*/

SEXP attribute_hidden do_saveToConn(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}

/* Read and checks the magic number, open the connection if needed */

extern int R_ReadItemDepth;
extern int R_InitReadItemDepth;

SEXP attribute_hidden do_loadFromConn2(SEXP call, SEXP op, SEXP args, SEXP env)
{
  return 0;
}
