/*******************************************************************************
 * MEMORY ALLOCATION MACRO
 ******************************************************************************/
#include <assert.h>
/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2013  The R Core Team.
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

/*
 *	This code implements a non-moving generational collector
 *      with two or three generations.
 *
 *	Memory allocated by R_alloc is maintained in a stack.  Code
 *	that R_allocs memory must use vmaxget and vmaxset to obtain
 *	and reset the stack pointer.
 */

#define USE_RINTERNALS


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <R_ext/RS.h> /* for S4 allocation */
#include <R_ext/Print.h>

/* Declarations for Valgrind.
 
 These are controlled by the
 --with-valgrind-instrumentation=
 option to configure, which sets VALGRIND_LEVEL to the
 supplied value (default 0) and defines NVALGRIND if
 the value is 0.
 
 level 0 is no additional instrumentation
 level 1 marks uninitialized numeric, logical, integer, raw,
 complex vectors and R_alloc memory
 level 2 marks the data section of vector nodes as inaccessible
 when they are freed.
 level 3 marks the first three bytes of sxpinfo and the ATTRIB
 field on both vector and non-vector nodes, and the
 three words of data in non-vector nodes.
 
 It may be necessary to define NVALGRIND for a non-gcc
 compiler on a supported architecture if it has different
 syntax for inline assembly language from gcc.
 
 For Win32, Valgrind is useful only if running under Wine.
 */
#ifdef Win32
# ifndef USE_VALGRIND_FOR_WINE
# define NVALGRIND 1
#endif
#endif

#ifndef NVALGRIND
# include "memcheck.h"
#endif


#ifndef VALGRIND_LEVEL
#define VALGRIND_LEVEL 0
#endif

#define R_USE_SIGNALS 1
#include <Defn.h>
#include <Internal.h>
#include <R_ext/GraphicsEngine.h> /* GEDevDesc, GEgetDevice */
#include <R_ext/Rdynload.h>

#if defined(Win32) && defined(LEA_MALLOC)
/*#include <stddef.h> */
extern void *Rm_malloc(size_t n);
extern void *Rm_calloc(size_t n_elements, size_t element_size);
extern void Rm_free(void * p);
extern void *Rm_realloc(void * p, size_t n);
#define calloc Rm_calloc
#define malloc Rm_malloc
#define realloc Rm_realloc
#define free Rm_free
#endif

/* malloc uses size_t.  We are assuming here that size_t is at least
 as large as unsigned long.  Changed from int at 1.6.0 to (i) allow
 2-4Gb objects on 32-bit system and (ii) objects limited only by
 length on a 64-bit system.
 */

static int gc_reporting = 0;
static int gc_count = 0;

/* These are used in profiling to separete out time in GC */
static Rboolean R_in_gc = TRUE;
int R_gc_running() { return R_in_gc; }

#ifdef TESTING_WRITE_BARRIER
# define PROTECTCHECK
#endif

#ifdef PROTECTCHECK
/* This is used to help detect unprotected SEXP values.  It is most
 useful if the strict barrier is enabled as well. The strategy is:
 
 All GCs are full GCs
 
 New nodes are marked as NEWSXP
 
 After a GC all free nodes that are not of type NEWSXP are
 marked as type FREESXP
 
 Most calls to accessor functions check their SEXP inputs and
 SEXP outputs with CHK() to see if a reachable node is a
 FREESXP and signal an error if a FREESXP is found.
 
 Combined with GC torture this can help locate where an unprotected
 SEXP is being used.
 
 This approach will miss cases where an unprotected node has been
 re-allocated.  For these cases it is possible to set
 gc_inhibit_release to TRUE.  FREESXP nodes will not be reallocated,
 or large ones released, until gc_inhibit_release is set to FALSE
 again.  This will of course result in memory growth and should be
 used with care and typically in combination with OS mechanisms to
 limit process memory usage.  LT */

/* Before a node is marked as a FREESXP by the collector the previous
 type is recorded.  For now using the LEVELS field seems
 reasonable.  */
#define OLDTYPE(s) LEVELS(s)
#define SETOLDTYPE(s, t) SETLEVELS(s, t)

static const char *sexptype2char(SEXPTYPE type);

static R_INLINE SEXP CHK(SEXP x)
{
    /* **** NULL check because of R_CurrentExpr */
    if (x != NULL && TYPEOF(x) == FREESXP)
        error("unprotected object (%p) encountered (was %s)",
              x, sexptype2char(OLDTYPE(x)));
    return x;
}
#else
#define CHK(x) x
#endif

/* The following three variables definitions are used to record the
 address and type of the first bad type seen during a collection,
 and for FREESXP nodes they record the old type as well. */
static SEXPTYPE bad_sexp_type_seen = 0;
static SEXP bad_sexp_type_sexp = NULL;
#ifdef PROTECTCHECK
static SEXPTYPE bad_sexp_type_old_type = 0;
#endif
static int bad_sexp_type_line = 0;

static R_INLINE void register_bad_sexp_type(SEXP s, int line)
{
    if (bad_sexp_type_seen == 0) {
        bad_sexp_type_seen = TYPEOF(s);
        bad_sexp_type_sexp = s;
        bad_sexp_type_line = line;
#ifdef PROTECTCHECK
        if (TYPEOF(s) == FREESXP)
            bad_sexp_type_old_type = OLDTYPE(s);
#endif
    }
}

/* slight modification of typename() from install.c -- should probably merge */
static const char *sexptype2char(SEXPTYPE type) {
    switch (type) {
        case NILSXP:	return "NILSXP";
        case SYMSXP:	return "SYMSXP";
        case LISTSXP:	return "LISTSXP";
        case CLOSXP:	return "CLOSXP";
        case ENVSXP:	return "ENVSXP";
        case PROMSXP:	return "PROMSXP";
        case LANGSXP:	return "LANGSXP";
        case SPECIALSXP:	return "SPECIALSXP";
        case BUILTINSXP:	return "BUILTINSXP";
        case CHARSXP:	return "CHARSXP";
        case LGLSXP:	return "LGLSXP";
        case INTSXP:	return "INTSXP";
        case REALSXP:	return "REALSXP";
        case CPLXSXP:	return "CPLXSXP";
        case STRSXP:	return "STRSXP";
        case DOTSXP:	return "DOTSXP";
        case ANYSXP:	return "ANYSXP";
        case VECSXP:	return "VECSXP";
        case EXPRSXP:	return "EXPRSXP";
        case BCODESXP:	return "BCODESXP";
        case EXTPTRSXP:	return "EXTPTRSXP";
        case WEAKREFSXP:	return "WEAKREFSXP";
        case S4SXP:		return "S4SXP";
        case RAWSXP:	return "RAWSXP";
        case NEWSXP:	return "NEWSXP"; /* should never happen */
        case FREESXP:	return "FREESXP";
        default:	 	return "<unknown>";
    }
}

/* FORCE_GC is always 0. */
#define GC_TORTURE

#ifdef GC_TORTURE
/* **** if the user specified a wait before starting to force
 **** collections it might make sense to also wait before starting
 **** to inhibit releases */
static int gc_force_wait = 0;
static int gc_force_gap = 0;
static Rboolean gc_inhibit_release = FALSE;
#define FORCE_GC 0
//#define FORCE_GC (gc_force_wait > 0 ? (--gc_force_wait > 0 ? 0 : (gc_force_wait = gc_force_gap, 1)) : 0)
#else
# define FORCE_GC 0
#endif

#ifdef R_MEMORY_PROFILING
static void R_ReportAllocation(R_size_t);
static void R_ReportNewPage();
#endif

#define GC_PROT(X) do { \
int __wait__ = gc_force_wait; \
int __gap__ = gc_force_gap;			   \
Rboolean __release__ = gc_inhibit_release;	   \
X;						   \
gc_force_wait = __wait__;			   \
gc_force_gap = __gap__;			   \
gc_inhibit_release = __release__;		   \
}  while(0)

static void R_gc_internal(R_size_t size_needed);
static void R_gc_full(R_size_t size_needed);
static void mem_err_heap(R_size_t size);
static void mem_err_malloc(R_size_t size);

static SEXPREC UnmarkedNodeTemplate;
#define NODE_IS_MARKED(s) (MARK(s)==1)
#define MARK_NODE(s) (MARK(s)=1)
#define UNMARK_NODE(s) (MARK(s)=0)

/* Miscellaneous Globals. */

static SEXP R_VStack = NULL;		/* R_alloc stack pointer */
static SEXP R_PreciousList = NULL;      /* List of Persistent Objects */
static R_size_t R_LargeVallocSize = 0;
static R_size_t R_SmallVallocSize = 0;
static R_size_t orig_R_NSize;
static R_size_t orig_R_VSize;

static R_size_t R_N_maxused=0;
static R_size_t R_V_maxused=0;

/* Node Classes.  Non-vector nodes are of class zero. Small vector
 nodes are in classes 1, ..., NUM_SMALL_NODE_CLASSES, and large
 vector nodes are in class LARGE_NODE_CLASS.  For vector nodes the
 node header is followed in memory by the vector data, offset from
 the header by SEXPREC_ALIGN. */

#define NUM_NODE_CLASSES 8

/* sxpinfo allocates 3 bits for the node class, so at most 8 are allowed */
#if NUM_NODE_CLASSES > 8
# error NUM_NODE_CLASSES must be at most 8
#endif

#define LARGE_NODE_CLASS (NUM_NODE_CLASSES - 1)
#define NUM_SMALL_NODE_CLASSES (NUM_NODE_CLASSES - 1)

/* the number of VECREC's in nodes of the small node classes */
static int NodeClassSize[NUM_SMALL_NODE_CLASSES] = { 0, 1, 2, 4, 6, 8, 16 };

#define NODE_CLASS(s) ((s)->sxpinfo.gccls)
#define SET_NODE_CLASS(s,v) (((s)->sxpinfo.gccls) = (v))


/* Node Generations. */

#define NUM_OLD_GENERATIONS 2

/* sxpinfo allocates one bit for the old generation count, so only 1
 or 2 is allowed */
#if NUM_OLD_GENERATIONS > 2 || NUM_OLD_GENERATIONS < 1
# error number of old generations must be 1 or 2
#endif

#define NODE_GENERATION(s) ((s)->sxpinfo.gcgen)
#define SET_NODE_GENERATION(s,g) ((s)->sxpinfo.gcgen=(g))

#define NODE_GEN_IS_YOUNGER(s,g) \
(! NODE_IS_MARKED(s) || NODE_GENERATION(s) < (g))
#define NODE_IS_OLDER(x, y) \
(NODE_IS_MARKED(x) && \
(! NODE_IS_MARKED(y) || NODE_GENERATION(x) > NODE_GENERATION(y)))

static int num_old_gens_to_collect = 0;
static int gen_gc_counts[NUM_OLD_GENERATIONS + 1];
static int collect_counts[NUM_OLD_GENERATIONS];


/* Node Pages.  Non-vector nodes and small vector nodes are allocated
 from fixed size pages.  The pages for each node class are kept in a
 linked list. */

typedef union PAGE_HEADER {
    union PAGE_HEADER *next;
    double align;
} PAGE_HEADER;

#define BASE_PAGE_SIZE 2000
#define R_PAGE_SIZE \
(((BASE_PAGE_SIZE - sizeof(PAGE_HEADER)) / sizeof(SEXPREC)) \
* sizeof(SEXPREC) \
+ sizeof(PAGE_HEADER))
#define NODE_SIZE(c) \
((c) == 0 ? sizeof(SEXPREC) : \
sizeof(SEXPREC_ALIGN) + NodeClassSize[c] * sizeof(VECREC))

#define PAGE_DATA(p) ((void *) (p + 1))
#define VHEAP_FREE() (R_VSize - R_LargeVallocSize - R_SmallVallocSize)


/* The Heap Structure.  Nodes for each class/generation combination
 are arranged in circular doubly-linked lists.  The double linking
 allows nodes to be removed in constant time; this is used by the
 collector to move reachable nodes out of free space and into the
 appropriate generation.  The circularity eliminates the need for
 end checks.  In addition, each link is anchored at an artificial
 node, the Peg SEXPREC's in the structure below, which simplifies
 pointer maintenance.  The circular doubly-linked arrangement is
 taken from Baker's in-place incremental collector design; see
 ftp://ftp.netcom.com/pub/hb/hbaker/NoMotionGC.html or the Jones and
 Lins GC book.  The linked lists are implemented by adding two
 pointer fields to the SEXPREC structure, which increases its size
 from 5 to 7 words. Other approaches are possible but don't seem
 worth pursuing for R.
 
 There are two options for dealing with old-to-new pointers.  The
 first option is to make sure they never occur by transferring all
 referenced younger objects to the generation of the referrer when a
 reference to a newer object is assigned to an older one.  This is
 enabled by defining EXPEL_OLD_TO_NEW.  The second alternative is to
 keep track of all nodes that may contain references to newer nodes
 and to "age" the nodes they refer to at the beginning of each
 collection.  This is the default.  The first option is simpler in
 some ways, but will create more floating garbage and add a bit to
 the execution time, though the difference is probably marginal on
 both counts.*/
/*#define EXPEL_OLD_TO_NEW*/
static struct {
    SEXP Old[NUM_OLD_GENERATIONS], New, Free;
    SEXPREC OldPeg[NUM_OLD_GENERATIONS], NewPeg;
#ifndef EXPEL_OLD_TO_NEW
    SEXP OldToNew[NUM_OLD_GENERATIONS];
    SEXPREC OldToNewPeg[NUM_OLD_GENERATIONS];
#endif
    int OldCount[NUM_OLD_GENERATIONS], AllocCount, PageCount;
    PAGE_HEADER *pages;
} R_GenHeap[NUM_NODE_CLASSES];

static R_size_t R_NodesInUse = 0;

#define NEXT_NODE(s) (s)->gengc_next_node
#define PREV_NODE(s) (s)->gengc_prev_node
#define SET_NEXT_NODE(s,t) (NEXT_NODE(s) = (t))
#define SET_PREV_NODE(s,t) (PREV_NODE(s) = (t))


/* Node List Manipulation */

/* unsnap node s from its list */
#define UNSNAP_NODE(s) do { \
SEXP un__n__ = (s); \
SEXP next = NEXT_NODE(un__n__); \
SEXP prev = PREV_NODE(un__n__); \
SET_NEXT_NODE(prev, next); \
SET_PREV_NODE(next, prev); \
} while(0)

/* snap in node s before node t */
#define SNAP_NODE(s,t) do { \
SEXP sn__n__ = (s); \
SEXP next = (t); \
SEXP prev = PREV_NODE(next); \
SET_NEXT_NODE(sn__n__, next); \
SET_PREV_NODE(next, sn__n__); \
SET_NEXT_NODE(prev, sn__n__); \
SET_PREV_NODE(sn__n__, prev); \
} while (0)

/* Node Allocation. */

#define CLASS_GET_FREE_NODE(c,s) do { \
(s) = malloc ( sizeof(SEXPREC) ); \
} while ( 0 )

#define NO_FREE_NODES() (R_NodesInUse >= R_NSize)
#define GET_FREE_NODE(s) CLASS_GET_FREE_NODE(0,s)


static void old_to_new(SEXP x, SEXP y)
{
    return;
}

#define CHECK_OLD_TO_NEW(x,y) do { \
if (NODE_IS_OLDER(CHK(x), CHK(y))) old_to_new(x,y);  } while (0)


/*******************************************************************************
 * MEMORY ALLOCATION FUNCTIONS
 ******************************************************************************/

static void mem_err_heap(R_size_t size)
{
    errorcall(R_NilValue, _("vector memory exhausted (limit reached?)"));
}


static void mem_err_cons(void)
{
    errorcall(R_NilValue, _("cons memory exhausted (limit reached?)"));
}

static void mem_err_malloc(R_size_t size)
{
    errorcall(R_NilValue, _("memory exhausted (limit reached?)"));
}

/* InitMemory : Initialise the memory to be used in R. */
/* This includes: stack space, node space and vector space */

#define PP_REDZONE_SIZE 1000L
static int R_StandardPPStackSize, R_RealPPStackSize;

void attribute_hidden InitMemory()
{
    /* R_NilValue */
    /* THIS MUST BE THE FIRST CONS CELL ALLOCATED */
    /* OR ARMAGEDDON HAPPENS. */
    /* Field assignments for R_NilValue must not go through write barrier
     since the write barrier prevents assignments to R_NilValue's fields.
     because of checks for nil */
    GET_FREE_NODE(R_NilValue);
    R_NilValue->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(R_NilValue) = NILSXP;
    CAR(R_NilValue) = R_NilValue;
    CDR(R_NilValue) = R_NilValue;
    TAG(R_NilValue) = R_NilValue;
    ATTRIB(R_NilValue) = R_NilValue;

    return;
}

/* "allocSExp" allocate a SEXPREC */
/* call gc if necessary */

SEXP allocSExp(SEXPTYPE t)
{
    SEXP s;
    if (FORCE_GC || NO_FREE_NODES()) {
        R_gc_internal(0);
        if (NO_FREE_NODES())
            mem_err_cons();
    }
    GET_FREE_NODE(s);
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = t;
    CAR(s) = R_NilValue;
    CDR(s) = R_NilValue;
    TAG(s) = R_NilValue;
#if VALGRIND_LEVEL > 2
    VALGRIND_MAKE_WRITABLE(&ATTRIB(s), sizeof(void *));
    VALGRIND_MAKE_WRITABLE(&(s->u), 3*(sizeof(void *)));
    VALGRIND_MAKE_WRITABLE(s,3);
#endif
    ATTRIB(s) = R_NilValue;
    return s;
}

static SEXP allocSExpNonCons(SEXPTYPE t)
{
    SEXP s;
    if (FORCE_GC || NO_FREE_NODES()) {
        R_gc_internal(0);
        if (NO_FREE_NODES())
            mem_err_cons();
    }
    GET_FREE_NODE(s);
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = t;
    TAG(s) = R_NilValue;
#if VALGRIND_LEVEL > 2
    VALGRIND_MAKE_WRITABLE(&ATTRIB(s), sizeof(void *));
    VALGRIND_MAKE_WRITABLE(&(s->u), 3*(sizeof(void *)));
    VALGRIND_MAKE_WRITABLE(s,3);
#endif
    ATTRIB(s) = R_NilValue;
    return s;
}

/* cons is defined directly to avoid the need to protect its arguments
 unless a GC will actually occur. */
SEXP cons(SEXP car, SEXP cdr)
{
    SEXP s;
    if (FORCE_GC || NO_FREE_NODES()) {
        PROTECT(car);
        PROTECT(cdr);
        R_gc_internal(0);
        UNPROTECT(2);
        if (NO_FREE_NODES())
            mem_err_cons();
    }
    GET_FREE_NODE(s);
#if VALGRIND_LEVEL > 2
    VALGRIND_MAKE_WRITABLE(&ATTRIB(s), sizeof(void *));
    VALGRIND_MAKE_WRITABLE(&(s->u), 3*(sizeof(void *)));
    VALGRIND_MAKE_WRITABLE(s,3);
#endif
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = LISTSXP;
    CAR(s) = CHK(car);
    CDR(s) = CHK(cdr);
    TAG(s) = R_NilValue;
    ATTRIB(s) = R_NilValue;
    return s;
}

/*----------------------------------------------------------------------
 
 NewEnvironment
 
 Create an environment by extending "rho" with a frame obtained by
 pairing the variable names given by the tags on "namelist" with
 the values given by the elements of "valuelist".
 
 NewEnvironment is defined directly to avoid the need to protect its
 arguments unless a GC will actually occur.  This definition allows
 the namelist argument to be shorter than the valuelist; in this
 case the remaining values must be named already.  (This is useful
 in cases where the entire valuelist is already named--namelist can
 then be R_NilValue.)
 
 The valuelist is destructively modified and used as the
 environment's frame.
 */
// BLANK








/* All vector objects must be a multiple of sizeof(SEXPREC_ALIGN)
 bytes so that alignment is preserved for all objects */

/* Allocate a vector object (and also list-like objects).
 This ensures only validity of list-like (LISTSXP, VECSXP, EXPRSXP),
 STRSXP and CHARSXP types;  e.g., atomic types remain un-initialized
 and must be initialized upstream, e.g., in do_makevector().
 */
#define intCHARSXP 73

SEXP allocVector(SEXPTYPE type, R_xlen_t length)
{
    SEXP s;     /* For the generational collector it would be safer to
                 work in terms of a VECSEXP here, but that would
                 require several casts below... */
    R_len_t i;
    R_size_t size = 0, alloc_size, old_R_VSize;
    int node_class;
#if VALGRIND_LEVEL > 0
    R_size_t actual_size = 0;
#endif
    
    /* Handle some scalars directly to improve speed. */
    if (length == 1) {
        switch(type) {
            case REALSXP:
            case INTSXP:
            case LGLSXP:
                node_class = 1;
                alloc_size = NodeClassSize[1];
                if (FORCE_GC || NO_FREE_NODES() || VHEAP_FREE() < alloc_size) {
                    R_gc_internal(alloc_size);
                    if (NO_FREE_NODES())
                        mem_err_cons();
                    if (VHEAP_FREE() < alloc_size)
                        mem_err_heap(size);
                }
                
                CLASS_GET_FREE_NODE(node_class, s);
#if VALGRIND_LEVEL > 2
                VALGRIND_MAKE_WRITABLE(&ATTRIB(s), sizeof(void *));
                VALGRIND_MAKE_WRITABLE(s, 3);
#endif
#if VALGRIND_LEVEL > 1
                switch(type) {
                    case REALSXP: actual_size = sizeof(double); break;
                    case INTSXP: actual_size = sizeof(int); break;
                    case LGLSXP: actual_size = sizeof(int); break;
                }
                VALGRIND_MAKE_WRITABLE(DATAPTR(s), actual_size);
#endif
                s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
                SET_NODE_CLASS(s, node_class);
                R_SmallVallocSize += alloc_size;
                ATTRIB(s) = R_NilValue;
                TYPEOF(s) = type;
                SET_SHORT_VEC_LENGTH(s, (R_len_t) length); // is 1
                SET_SHORT_VEC_TRUELENGTH(s, 0);
                NAMED(s) = 0;
                return(s);
        }
    }
    
    if (length > R_XLEN_T_MAX)
        errorcall(R_GlobalContext->call,
                  _("vector is too large")); /**** put length into message */
    else if (length < 0 )
        errorcall(R_GlobalContext->call,
                  _("negative length vectors are not allowed"));
    /* number of vector cells to allocate */
    switch (type) {
        case NILSXP:
            return R_NilValue;
        case RAWSXP:
            size = BYTE2VEC(length);
#if VALGRIND_LEVEL > 0
            actual_size=length;
#endif
            break;
        case CHARSXP:
            error("use of allocVector(CHARSXP ...) is defunct\n");
        case intCHARSXP:
            size = BYTE2VEC(length + 1);
#if VALGRIND_LEVEL > 0
            actual_size = length + 1;
#endif
            break;
        case LGLSXP:
        case INTSXP:
            if (length <= 0)
                size = 0;
            else {
                if (length > R_SIZE_T_MAX / sizeof(int))
                    errorcall(R_GlobalContext->call,
                              _("cannot allocate vector of length %d"), length);
                size = INT2VEC(length);
#if VALGRIND_LEVEL > 0
                actual_size = length*sizeof(int);
#endif
            }
            break;
        case REALSXP:
            if (length <= 0)
                size = 0;
            else {
                if (length > R_SIZE_T_MAX / sizeof(double))
                    errorcall(R_GlobalContext->call,
                              _("cannot allocate vector of length %d"), length);
                size = FLOAT2VEC(length);
#if VALGRIND_LEVEL > 0
                actual_size = length * sizeof(double);
#endif
            }
            break;
        case CPLXSXP:
            if (length <= 0)
                size = 0;
            else {
                if (length > R_SIZE_T_MAX / sizeof(Rcomplex))
                    errorcall(R_GlobalContext->call,
                              _("cannot allocate vector of length %d"), length);
                size = COMPLEX2VEC(length);
#if VALGRIND_LEVEL > 0
                actual_size = length * sizeof(Rcomplex);
#endif
            }
            break;
        case STRSXP:
        case EXPRSXP:
        case VECSXP:
            if (length <= 0)
                size = 0;
            else {
                if (length > R_SIZE_T_MAX / sizeof(SEXP))
                    errorcall(R_GlobalContext->call,
                              _("cannot allocate vector of length %d"), length);
                size = PTR2VEC(length);
#if VALGRIND_LEVEL > 0
                actual_size = length * sizeof(SEXP);
#endif
            }
            break;
        case LANGSXP:
            if(length == 0) return R_NilValue;
#ifdef LONG_VECTOR_SUPPORT
            if (length > R_SHORT_LEN_MAX) error("invalid length for pairlist");
#endif
            s = allocList((int) length);
            TYPEOF(s) = LANGSXP;
            return s;
        case LISTSXP:
#ifdef LONG_VECTOR_SUPPORT
            if (length > R_SHORT_LEN_MAX) error("invalid length for pairlist");
#endif
            return allocList((int) length);
        default:
            error(_("invalid type/length (%s/%d) in vector allocation"),
                  type2char(type), length);
    }
    
    if (size <= NodeClassSize[1]) {
        node_class = 1;
        alloc_size = NodeClassSize[1];
    }
    else {
        node_class = LARGE_NODE_CLASS;
        alloc_size = size;
        for (i = 2; i < NUM_SMALL_NODE_CLASSES; i++) {
            if (size <= NodeClassSize[i]) {
                node_class = i;
                alloc_size = NodeClassSize[i];
                break;
            }
        }
    }
    
    /* save current R_VSize to roll back adjustment if malloc fails */
    old_R_VSize = R_VSize;
    
    /* we need to do the gc here so allocSExp doesn't! */
    if (FORCE_GC || NO_FREE_NODES() || VHEAP_FREE() < alloc_size) {
        R_gc_internal(alloc_size);
        if (NO_FREE_NODES())
            mem_err_cons();
        if (VHEAP_FREE() < alloc_size)
            mem_err_heap(size);
    }
    
    if (size > 0) {
        if (node_class < NUM_SMALL_NODE_CLASSES) {
            CLASS_GET_FREE_NODE(node_class, s);
#if VALGRIND_LEVEL > 2
            VALGRIND_MAKE_WRITABLE(&ATTRIB(s), sizeof(void *));
            VALGRIND_MAKE_WRITABLE(s, 3);
#endif
#if VALGRIND_LEVEL > 1
            VALGRIND_MAKE_WRITABLE(DATAPTR(s), actual_size);
#endif
            s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
            SET_NODE_CLASS(s, node_class);
            R_SmallVallocSize += alloc_size;
            SET_SHORT_VEC_LENGTH(s, (R_len_t) length);
        }
        else {
            Rboolean success = FALSE;
            R_size_t hdrsize = sizeof(SEXPREC_ALIGN);
#ifdef LONG_VECTOR_SUPPORT
            if (length > R_SHORT_LEN_MAX)
                hdrsize = sizeof(SEXPREC_ALIGN) + sizeof(R_long_vec_hdr_t);
#endif
            void *mem = NULL; /* initialize to suppress warning */
            if (size < (R_SIZE_T_MAX / sizeof(VECREC)) - hdrsize) { /*** not sure this test is quite right -- why subtract the header? LT */
                mem = malloc(hdrsize + size * sizeof(VECREC));
                if (mem == NULL) {
                    /* If we are near the address space limit, we
                     might be short of address space.  So return
                     all unused objects to malloc and try again. */
                    R_gc_full(alloc_size);
                    mem = malloc(hdrsize + size * sizeof(VECREC));
                }
                if (mem != NULL) {
#ifdef LONG_VECTOR_SUPPORT
                    if (length > R_SHORT_LEN_MAX) {
                        s = (SEXP) (((char *) mem) + sizeof(R_long_vec_hdr_t));
                        SET_SHORT_VEC_LENGTH(s, R_LONG_VEC_TOKEN);
                        SET_LONG_VEC_LENGTH(s, length);
                        SET_LONG_VEC_TRUELENGTH(s, 0);
                    }
                    else {
                        s = mem;
                        SET_SHORT_VEC_LENGTH(s, (R_len_t) length);
                    }
#else
                    s = mem;
                    SETLENGTH(s, length);
#endif
                    success = TRUE;
                }
                else s = NULL;
#ifdef R_MEMORY_PROFILING
                R_ReportAllocation(hdrsize + size * sizeof(VECREC));
#endif
            } else s = NULL; /* suppress warning */
            if (! success) {
                double dsize = (double)size * sizeof(VECREC)/1024.0;
                /* reset the vector heap limit */
                R_VSize = old_R_VSize;
                if(dsize > 1024.0*1024.0)
                    errorcall(R_NilValue,
                              _("cannot allocate vector of size %0.1f Gb"),
                              dsize/1024.0/1024.0);
                if(dsize > 1024.0)
                    errorcall(R_NilValue,
                              _("cannot allocate vector of size %0.1f Mb"),
                              dsize/1024.0);
                else
                    errorcall(R_NilValue,
                              _("cannot allocate vector of size %0.f Kb"),
                              dsize);
            }
            s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
            SET_NODE_CLASS(s, LARGE_NODE_CLASS);
            R_LargeVallocSize += size;
            R_GenHeap[LARGE_NODE_CLASS].AllocCount++;
            R_NodesInUse++;
            SNAP_NODE(s, R_GenHeap[LARGE_NODE_CLASS].New);
        }
        ATTRIB(s) = R_NilValue;
        TYPEOF(s) = type;
    }
    else {
        GC_PROT(s = allocSExpNonCons(type));
        SET_SHORT_VEC_LENGTH(s, (R_len_t) length);
    }
    SET_SHORT_VEC_TRUELENGTH(s, 0);
    NAMED(s) = 0;
    
    /* The following prevents disaster in the case */
    /* that an uninitialised string vector is marked */
    /* Direct assignment is OK since the node was just allocated and */
    /* so is at least as new as R_NilValue and R_BlankString */
    if (type == EXPRSXP || type == VECSXP) {
        SEXP *data = STRING_PTR(s);
#if VALGRIND_LEVEL > 1
        VALGRIND_MAKE_READABLE(STRING_PTR(s), actual_size);
#endif
        for (i = 0; i < length; i++)
            data[i] = R_NilValue;
    }
    else if(type == STRSXP) {
        SEXP *data = STRING_PTR(s);
#if VALGRIND_LEVEL > 1
        VALGRIND_MAKE_READABLE(STRING_PTR(s), actual_size);
#endif
        for (i = 0; i < length; i++)
            data[i] = R_BlankString;
    }
    else if (type == CHARSXP || type == intCHARSXP) {
#if VALGRIND_LEVEL > 0
        VALGRIND_MAKE_WRITABLE(CHAR(s), actual_size);
#endif
        CHAR_RW(s)[length] = 0;
    }
#if VALGRIND_LEVEL > 0
    else if (type == REALSXP)
        VALGRIND_MAKE_WRITABLE(REAL(s), actual_size);
    else if (type == INTSXP)
        VALGRIND_MAKE_WRITABLE(INTEGER(s), actual_size);
    else if (type == LGLSXP)
        VALGRIND_MAKE_WRITABLE(LOGICAL(s), actual_size);
    else if (type == CPLXSXP)
        VALGRIND_MAKE_WRITABLE(COMPLEX(s), actual_size);
    else if (type == RAWSXP)
        VALGRIND_MAKE_WRITABLE(RAW(s), actual_size);
#endif
    return s;
}

/* For future hiding of allocVector(CHARSXP) */
SEXP attribute_hidden allocCharsxp(R_len_t len)
{
    return allocVector(intCHARSXP, len);
}

SEXP allocList(int n)
{
    int i;
    SEXP result;
    result = R_NilValue;
    for (i = 0; i < n; i++)
        result = CONS(R_NilValue, result);
    return result;
}


/* "gc" a mark-sweep or in-place generational garbage collector */

void R_gc(void)
{
    return;
}

static void R_gc_full(R_size_t size_needed)
{
    return;
}

static void R_gc_internal(R_size_t size_needed)
{
    return;
}




/* The following functions are replacements for the accessor macros.
 They are used by code that does not have direct access to the
 internal representation of objects.  The replacement functions
 implement the write barrier. */

/* General Cons Cell Attributes */
SEXP (ATTRIB)(SEXP x) { return CHK(ATTRIB(CHK(x))); }
int (OBJECT)(SEXP x) { return OBJECT(CHK(x)); }
int (MARK)(SEXP x) { return MARK(CHK(x)); }
int (TYPEOF)(SEXP x) { return TYPEOF(CHK(x)); }
int (NAMED)(SEXP x) { return NAMED(CHK(x)); }
int (RTRACE)(SEXP x) { return RTRACE(CHK(x)); }
int (LEVELS)(SEXP x) { return LEVELS(CHK(x)); }

void (SET_ATTRIB)(SEXP x, SEXP v) {
    if(TYPEOF(v) != LISTSXP && TYPEOF(v) != NILSXP)
        error("value of 'SET_ATTRIB' must be a pairlist or NULL, not a '%s'",
              type2char(TYPEOF(x)));
        CHECK_OLD_TO_NEW(x, v);
        ATTRIB(x) = v;
        }
void (SET_OBJECT)(SEXP x, int v) { SET_OBJECT(CHK(x), v); }
void (SET_TYPEOF)(SEXP x, int v) { SET_TYPEOF(CHK(x), v); }
void (SET_NAMED)(SEXP x, int v) { SET_NAMED(CHK(x), v); }
void (SET_RTRACE)(SEXP x, int v) { SET_RTRACE(CHK(x), v); }
int (SETLEVELS)(SEXP x, int v) { return SETLEVELS(CHK(x), v); }
//void DUPLICATE_ATTRIB(SEXP to, SEXP from) {
//    SET_ATTRIB(CHK(to), duplicate(CHK(ATTRIB(CHK(from)))));
//    SET_OBJECT(CHK(to), OBJECT(from));
//    IS_S4_OBJECT(from) ?  SET_S4_OBJECT(to) : UNSET_S4_OBJECT(to);
//}

/* S4 object testing */
int (IS_S4_OBJECT)(SEXP x){ return IS_S4_OBJECT(CHK(x)); }
void (SET_S4_OBJECT)(SEXP x){ SET_S4_OBJECT(CHK(x)); }
void (UNSET_S4_OBJECT)(SEXP x){ UNSET_S4_OBJECT(CHK(x)); }

/* Vector Accessors */
int (LENGTH)(SEXP x) { return LENGTH(CHK(x)); }
int (TRUELENGTH)(SEXP x) { return TRUELENGTH(CHK(x)); }
void (SETLENGTH)(SEXP x, int v) { SETLENGTH(CHK(x), v); }
void (SET_TRUELENGTH)(SEXP x, int v) { SET_TRUELENGTH(CHK(x), v); }
R_xlen_t (XLENGTH)(SEXP x) { return XLENGTH(CHK(x)); }
R_xlen_t (XTRUELENGTH)(SEXP x) { return XTRUELENGTH(CHK(x)); }
int  (IS_LONG_VEC)(SEXP x) { return IS_LONG_VEC(CHK(x)); }

const char *(R_CHAR)(SEXP x) {
    if(TYPEOF(x) != CHARSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "CHAR", "CHARSXP", type2char(TYPEOF(x)));
        return (const char *)CHAR(x);
        }

SEXP (STRING_ELT)(SEXP x, R_xlen_t i) {
    if(TYPEOF(x) != STRSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "STRING_ELT", "character vector", type2char(TYPEOF(x)));
        return CHK(STRING_ELT(x, i));
        }

SEXP (VECTOR_ELT)(SEXP x, R_xlen_t i) {
    /* We need to allow vector-like types here */
    if(TYPEOF(x) != VECSXP &&
       TYPEOF(x) != EXPRSXP &&
       TYPEOF(x) != WEAKREFSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "VECTOR_ELT", "list", type2char(TYPEOF(x)));
        return CHK(VECTOR_ELT(x, i));
        }

int *(LOGICAL)(SEXP x) {
    if(TYPEOF(x) != LGLSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "LOGICAL",  "logical", type2char(TYPEOF(x)));
        return LOGICAL(x);
        }

/* Maybe this should exclude logicals, but it is widely used */
int *(INTEGER)(SEXP x) {
    if(TYPEOF(x) != INTSXP && TYPEOF(x) != LGLSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "INTEGER", "integer", type2char(TYPEOF(x)));
        return INTEGER(x);
        }

Rbyte *(RAW)(SEXP x) {
    if(TYPEOF(x) != RAWSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "RAW", "raw", type2char(TYPEOF(x)));
        return RAW(x);
        }

double *(REAL)(SEXP x) {
    if(TYPEOF(x) != REALSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "REAL", "numeric", type2char(TYPEOF(x)));
        return REAL(x);
        }

Rcomplex *(COMPLEX)(SEXP x) {
    if(TYPEOF(x) != CPLXSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "COMPLEX", "complex", type2char(TYPEOF(x)));
        return COMPLEX(x);
        }

SEXP *(STRING_PTR)(SEXP x) { return STRING_PTR(CHK(x)); }

SEXP *(VECTOR_PTR)(SEXP x)
{
    error(_("not safe to return vector pointer"));
    return NULL;
}

void (SET_STRING_ELT)(SEXP x, R_xlen_t i, SEXP v) {
    if(TYPEOF(x) != STRSXP)
        error("%s() can only be applied to a '%s', not a '%s'",
              "SET_STRING_ELT", "character vector", type2char(TYPEOF(x)));
        if(TYPEOF(v) != CHARSXP)
            error("Value of SET_STRING_ELT() must be a 'CHARSXP' not a '%s'",
                  type2char(TYPEOF(v)));
            CHECK_OLD_TO_NEW(x, v);
            STRING_ELT(x, i) = v;
            }

SEXP (SET_VECTOR_ELT)(SEXP x, R_xlen_t i, SEXP v) {
    /*  we need to allow vector-like types here */
    if(TYPEOF(x) != VECSXP &&
       TYPEOF(x) != EXPRSXP &&
       TYPEOF(x) != WEAKREFSXP) {
        error("%s() can only be applied to a '%s', not a '%s'",
              "SET_VECTOR_ELT", "list", type2char(TYPEOF(x)));
    }
    CHECK_OLD_TO_NEW(x, v);
    return VECTOR_ELT(x, i) = v;
}


/* List Accessors */
SEXP (TAG)(SEXP e) { return CHK(TAG(CHK(e))); }
SEXP (CAR)(SEXP e) { return CHK(CAR(CHK(e))); }
SEXP (CDR)(SEXP e) { return CHK(CDR(CHK(e))); }
SEXP (CAAR)(SEXP e) { return CHK(CAAR(CHK(e))); }
SEXP (CDAR)(SEXP e) { return CHK(CDAR(CHK(e))); }
SEXP (CADR)(SEXP e) { return CHK(CADR(CHK(e))); }
SEXP (CDDR)(SEXP e) { return CHK(CDDR(CHK(e))); }
SEXP (CADDR)(SEXP e) { return CHK(CADDR(CHK(e))); }
SEXP (CADDDR)(SEXP e) { return CHK(CADDDR(CHK(e))); }
SEXP (CAD4R)(SEXP e) { return CHK(CAD4R(CHK(e))); }
int (MISSING)(SEXP x) { return MISSING(CHK(x)); }

void (SET_TAG)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); TAG(x) = v; }

SEXP (SETCAR)(SEXP x, SEXP y)
{
    if (x == NULL || x == R_NilValue)
        error(_("bad value"));
        CHECK_OLD_TO_NEW(x, y);
        CAR(x) = y;
        return y;
}

SEXP (SETCDR)(SEXP x, SEXP y)
{
    if (x == NULL || x == R_NilValue)
        error(_("bad value"));
        CHECK_OLD_TO_NEW(x, y);
        CDR(x) = y;
        return y;
}

SEXP (SETCADR)(SEXP x, SEXP y)
{
    SEXP cell;
    if (x == NULL || x == R_NilValue ||
        CDR(x) == NULL || CDR(x) == R_NilValue)
        error(_("bad value"));
        cell = CDR(x);
        CHECK_OLD_TO_NEW(cell, y);
        CAR(cell) = y;
        return y;
}

SEXP (SETCADDR)(SEXP x, SEXP y)
{
    SEXP cell;
    if (x == NULL || x == R_NilValue ||
        CDR(x) == NULL || CDR(x) == R_NilValue ||
        CDDR(x) == NULL || CDDR(x) == R_NilValue)
        error(_("bad value"));
        cell = CDDR(x);
        CHECK_OLD_TO_NEW(cell, y);
        CAR(cell) = y;
        return y;
}

#define CDDDR(x) CDR(CDR(CDR(x)))

SEXP (SETCADDDR)(SEXP x, SEXP y)
{
    SEXP cell;
    if (CHK(x) == NULL || x == R_NilValue ||
        CHK(CDR(x)) == NULL || CDR(x) == R_NilValue ||
        CHK(CDDR(x)) == NULL || CDDR(x) == R_NilValue ||
        CHK(CDDDR(x)) == NULL || CDDDR(x) == R_NilValue)
        error(_("bad value"));
        cell = CDDDR(x);
        CHECK_OLD_TO_NEW(cell, y);
        CAR(cell) = y;
        return y;
}

#define CD4R(x) CDR(CDR(CDR(CDR(x))))

SEXP (SETCAD4R)(SEXP x, SEXP y)
{
    SEXP cell;
    if (CHK(x) == NULL || x == R_NilValue ||
        CHK(CDR(x)) == NULL || CDR(x) == R_NilValue ||
        CHK(CDDR(x)) == NULL || CDDR(x) == R_NilValue ||
        CHK(CDDDR(x)) == NULL || CDDDR(x) == R_NilValue ||
        CHK(CD4R(x)) == NULL || CD4R(x) == R_NilValue)
        error(_("bad value"));
        cell = CD4R(x);
        CHECK_OLD_TO_NEW(cell, y);
        CAR(cell) = y;
        return y;
}

void (SET_MISSING)(SEXP x, int v) { SET_MISSING(CHK(x), v); }

/* Closure Accessors */
SEXP (FORMALS)(SEXP x) { return CHK(FORMALS(CHK(x))); }
SEXP (BODY)(SEXP x) { return CHK(BODY(CHK(x))); }
SEXP (CLOENV)(SEXP x) { return CHK(CLOENV(CHK(x))); }
int (RDEBUG)(SEXP x) { return RDEBUG(CHK(x)); }
int (RSTEP)(SEXP x) { return RSTEP(CHK(x)); }

void (SET_FORMALS)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); FORMALS(x) = v; }
void (SET_BODY)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); BODY(x) = v; }
void (SET_CLOENV)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); CLOENV(x) = v; }
void (SET_RDEBUG)(SEXP x, int v) { SET_RDEBUG(CHK(x), v); }
void (SET_RSTEP)(SEXP x, int v) { SET_RSTEP(CHK(x), v); }

/* These are only needed with the write barrier on */
#ifdef TESTING_WRITE_BARRIER
/* Primitive Accessors */
int (PRIMOFFSET)(SEXP x) { return PRIMOFFSET(x); }
attribute_hidden
void (SET_PRIMOFFSET)(SEXP x, int v) { SET_PRIMOFFSET(x, v); }
#endif

/* Symbol Accessors */
SEXP (PRINTNAME)(SEXP x) { return CHK(PRINTNAME(CHK(x))); }
SEXP (SYMVALUE)(SEXP x) { return CHK(SYMVALUE(CHK(x))); }
SEXP (INTERNAL)(SEXP x) { return CHK(INTERNAL(CHK(x))); }
int (DDVAL)(SEXP x) { return DDVAL(CHK(x)); }

void (SET_PRINTNAME)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRINTNAME(x) = v; }
void (SET_SYMVALUE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); SYMVALUE(x) = v; }
void (SET_INTERNAL)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); INTERNAL(x) = v; }
void (SET_DDVAL)(SEXP x, int v) { SET_DDVAL(CHK(x), v); }

/* Environment Accessors */
SEXP (FRAME)(SEXP x) { return CHK(FRAME(CHK(x))); }
SEXP (ENCLOS)(SEXP x) { return CHK(ENCLOS(CHK(x))); }
SEXP (HASHTAB)(SEXP x) { return CHK(HASHTAB(CHK(x))); }
int (ENVFLAGS)(SEXP x) { return ENVFLAGS(CHK(x)); }

void (SET_FRAME)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); FRAME(x) = v; }
void (SET_ENCLOS)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); ENCLOS(x) = v; }
void (SET_HASHTAB)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); HASHTAB(x) = v; }
void (SET_ENVFLAGS)(SEXP x, int v) { SET_ENVFLAGS(x, v); }

/* Promise Accessors */
SEXP (PRCODE)(SEXP x) { return CHK(PRCODE(CHK(x))); }
SEXP (PRENV)(SEXP x) { return CHK(PRENV(CHK(x))); }
SEXP (PRVALUE)(SEXP x) { return CHK(PRVALUE(CHK(x))); }
int (PRSEEN)(SEXP x) { return PRSEEN(CHK(x)); }

void (SET_PRENV)(SEXP x, SEXP v){ CHECK_OLD_TO_NEW(x, v); PRENV(x) = v; }
void (SET_PRVALUE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRVALUE(x) = v; }
void (SET_PRCODE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRCODE(x) = v; }
void (SET_PRSEEN)(SEXP x, int v) { SET_PRSEEN(CHK(x), v); }

/* Hashing Accessors */
#ifdef TESTING_WRITE_BARRIER
attribute_hidden
int (HASHASH)(SEXP x) { return HASHASH(CHK(x)); }
attribute_hidden
int (HASHVALUE)(SEXP x) { return HASHVALUE(CHK(x)); }

attribute_hidden
void (SET_HASHASH)(SEXP x, int v) { SET_HASHASH(CHK(x), v); }
attribute_hidden
void (SET_HASHVALUE)(SEXP x, int v) { SET_HASHVALUE(CHK(x), v); }
#endif

attribute_hidden
SEXP (SET_CXTAIL)(SEXP x, SEXP v) {
#ifdef USE_TYPE_CHECKING
    if(TYPEOF(v) != CHARSXP && TYPEOF(v) != NILSXP)
        error("value of 'SET_CXTAIL' must be a char or NULL, not a '%s'",
              type2char(TYPEOF(v)));
#endif
    /*CHECK_OLD_TO_NEW(x, v); *//* not needed since not properly traced */
        ATTRIB(x) = v;
        return x;
}

/* Test functions */
Rboolean Rf_isNull(SEXP s) { return isNull(s); }
Rboolean Rf_isSymbol(SEXP s) { return isSymbol(s); }
Rboolean Rf_isLogical(SEXP s) { return isLogical(s); }
Rboolean Rf_isReal(SEXP s) { return isReal(s); }
Rboolean Rf_isComplex(SEXP s) { return isComplex(s); }
Rboolean Rf_isExpression(SEXP s) { return isExpression(s); }
Rboolean Rf_isEnvironment(SEXP s) { return isEnvironment(s); }
Rboolean Rf_isString(SEXP s) { return isString(s); }
Rboolean Rf_isObject(SEXP s) { return isObject(s); }

/* Bindings accessors */
Rboolean attribute_hidden
(IS_ACTIVE_BINDING)(SEXP b) {return IS_ACTIVE_BINDING(b);}
Rboolean attribute_hidden
(BINDING_IS_LOCKED)(SEXP b) {return BINDING_IS_LOCKED(b);}
void attribute_hidden
(SET_ACTIVE_BINDING_BIT)(SEXP b) {SET_ACTIVE_BINDING_BIT(b);}
void attribute_hidden (LOCK_BINDING)(SEXP b) {LOCK_BINDING(b);}
void attribute_hidden (UNLOCK_BINDING)(SEXP b) {UNLOCK_BINDING(b);}

/* R_FunTab accessors, only needed when write barrier is on */
/* Not hidden to allow experimentaiton without rebuilding R - LT */
/* attribute_hidden */
int (PRIMVAL)(SEXP x) { return PRIMVAL(x); }
/* attribute_hidden */
CCODE (PRIMFUN)(SEXP x) { return PRIMFUN(x); }
/* attribute_hidden */
void (SET_PRIMFUN)(SEXP x, CCODE f) { PRIMFUN(x) = f; }

/* for use when testing the write barrier */
int  attribute_hidden (IS_BYTES)(SEXP x) { return IS_BYTES(x); }
int  attribute_hidden (IS_LATIN1)(SEXP x) { return IS_LATIN1(x); }
int  attribute_hidden (IS_ASCII)(SEXP x) { return IS_ASCII(x); }
int  attribute_hidden (IS_UTF8)(SEXP x) { return IS_UTF8(x); }
void attribute_hidden (SET_BYTES)(SEXP x) { SET_BYTES(x); }
void attribute_hidden (SET_LATIN1)(SEXP x) { SET_LATIN1(x); }
void attribute_hidden (SET_UTF8)(SEXP x) { SET_UTF8(x); }
void attribute_hidden (SET_ASCII)(SEXP x) { SET_ASCII(x); }
int  (ENC_KNOWN)(SEXP x) { return ENC_KNOWN(x); }
void attribute_hidden (SET_CACHED)(SEXP x) { SET_CACHED(x); }
int  (IS_CACHED)(SEXP x) { return IS_CACHED(x); }



/* ======== This needs direct access to gp field for efficiency ======== */

/* this has NA_STRING = NA_STRING */
attribute_hidden
int Seql(SEXP a, SEXP b)
{
    /* The only case where pointer comparisons do not suffice is where
     we have two strings in different encodings (which must be
     non-ASCII strings). Note that one of the strings could be marked
     as unknown. */
    if (a == b) return 1;
    /* Leave this to compiler to optimize */
    if (IS_CACHED(a) && IS_CACHED(b) && ENC_KNOWN(a) == ENC_KNOWN(b))
        return 0;
    else {
        assert(0);
        return 0;
//    	SEXP vmax = R_VStack;
//    	int result = !strcmp(translateCharUTF8(a), translateCharUTF8(b));
//    	R_VStack = vmax; /* discard any memory used by translateCharUTF8 */
//    	return result;
    }
}


#ifdef LONG_VECTOR_SUPPORT
R_len_t R_BadLongVector(SEXP x, const char *file, int line)
{
    error(_("long vectors not supported yet: %s:%d"), file, line);
    return 0; /* not reached */
}
#endif
