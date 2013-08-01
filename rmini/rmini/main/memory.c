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
    assert(0);
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

#define GC_PROT(X) (X)

//#define GC_PROT(X) do { \
//int __wait__ = gc_force_wait; \
//int __gap__ = gc_force_gap;			   \
//Rboolean __release__ = gc_inhibit_release;	   \
//X;						   \
//gc_force_wait = __wait__;			   \
//gc_force_gap = __gap__;			   \
//gc_inhibit_release = __release__;		   \
//}  while(0)

static void R_gc_internal(R_size_t size_needed);
static void R_gc_full(R_size_t size_needed);
static void mem_err_heap(R_size_t size);
static void mem_err_malloc(R_size_t size);

static SEXPREC UnmarkedNodeTemplate;
#define NODE_IS_MARKED(s) (MARK(s)==1)
#define MARK_NODE(s) (MARK(s)=1)
#define UNMARK_NODE(s) (MARK(s)=0)


/* Tuning Constants. Most of these could be made settable from R,
 within some reasonable constraints at least.  Since there are quite
 a lot of constants it would probably make sense to put together
 several "packages" representing different space/speed tradeoffs
 (e.g. very aggressive freeing and small increments to conserve
 memory; much less frequent releasing and larger increments to
 increase speed). */

/* There are three levels of collections.  Level 0 collects only the
 youngest generation, level 1 collects the two youngest generations,
 and level 2 collects all generations.  Higher level collections
 occur at least after specified numbers of lower level ones.  After
 LEVEL_0_FREQ level zero collections a level 1 collection is done;
 after every LEVEL_1_FREQ level 1 collections a level 2 collection
 occurs.  Thus, roughly, every LEVEL_0_FREQ-th collection is a level
 1 collection and every (LEVEL_0_FREQ * LEVEL_1_FREQ)-th collection
 is a level 2 collection.  */
#define LEVEL_0_FREQ 20
#define LEVEL_1_FREQ 5
static int collect_counts_max[] = { LEVEL_0_FREQ, LEVEL_1_FREQ };

/* When a level N collection fails to produce at least MinFreeFrac *
 R_NSize free nodes and MinFreeFrac * R_VSize free vector space, the
 next collection will be a level N + 1 collection.
 
 This constant is also used in heap size adjustment as a minimal
 fraction of the minimal heap size levels that should be available
 for allocation. */
static double R_MinFreeFrac = 0.2;

/* When pages are released, a number of free nodes equal to
 R_MaxKeepFrac times the number of allocated nodes for each class is
 retained.  Pages not needed to meet this requirement are released.
 An attempt to release pages is made every R_PageReleaseFreq level 1
 or level 2 collections. */
static double R_MaxKeepFrac = 0.5;
static int R_PageReleaseFreq = 1;

/* The heap size constants R_NSize and R_VSize are used for triggering
 collections.  The initial values set by defaults or command line
 arguments are used as minimal values.  After full collections these
 levels are adjusted up or down, though not below the minimal values
 or above the maximum values, towards maintain heap occupancy within
 a specified range.  When the number of nodes in use reaches
 R_NGrowFrac * R_NSize, the value of R_NSize is incremented by
 R_NGrowIncrMin + R_NGrowIncrFrac * R_NSize.  When the number of
 nodes in use falls below R_NShrinkFrac, R_NSize is decremented by
 R_NShrinkIncrMin + R_NShrinkFrac * R_NSize.  Analogous adjustments
 are made to R_VSize.
 
 This mechanism for adjusting the heap size constants is very
 primitive but hopefully adequate for now.  Some modeling and
 experimentation would be useful.  We want the heap sizes to get set
 at levels adequate for the current computations.  The present
 mechanism uses only the size of the current live heap to provide
 information about the current needs; since the current live heap
 size can be very volatile, the adjustment mechanism only makes
 gradual adjustments.  A more sophisticated strategy would use more
 of the live heap history.
 
 Some of the settings can now be adjusted by environment variables.
 */
static double R_NGrowFrac = 0.70;
static double R_NShrinkFrac = 0.30;

static double R_VGrowFrac = 0.70;
static double R_VShrinkFrac = 0.30;

#ifdef SMALL_MEMORY
/* On machines with only 32M of memory (or on a classic Mac OS port)
 it might be a good idea to use settings like these that are more
 aggressive at keeping memory usage down. */
static double R_NGrowIncrFrac = 0.0, R_NShrinkIncrFrac = 0.2;
static int R_NGrowIncrMin = 50000, R_NShrinkIncrMin = 0;
static double R_VGrowIncrFrac = 0.0, R_VShrinkIncrFrac = 0.2;
static int R_VGrowIncrMin = 100000, R_VShrinkIncrMin = 0;
#else
static double R_NGrowIncrFrac = 0.05, R_NShrinkIncrFrac = 0.2;
static int R_NGrowIncrMin = 40000, R_NShrinkIncrMin = 0;
static double R_VGrowIncrFrac = 0.05, R_VShrinkIncrFrac = 0.2;
static int R_VGrowIncrMin = 80000, R_VShrinkIncrMin = 0;
#endif

static void init_gc_grow_settings()
{
    return;
}

/* Maximal Heap Limits.  These variables contain upper limits on the
 heap sizes.  They could be made adjustable from the R level,
 perhaps by a handler for a recoverable error.
 
 Access to these values is provided with reader and writer
 functions; the writer function insures that the maximal values are
 never set below the current ones. */
static R_size_t R_MaxVSize = R_SIZE_T_MAX;
static R_size_t R_MaxNSize = R_SIZE_T_MAX;
static int vsfac = 1; /* current units for vsize: changes at initialization */

R_size_t attribute_hidden R_GetMaxVSize(void)
{
    return 0;
}

void attribute_hidden R_SetMaxVSize(R_size_t size)
{
    return;
}

R_size_t attribute_hidden R_GetMaxNSize(void)
{
    return 0;
}

void attribute_hidden R_SetMaxNSize(R_size_t size)
{
    return;
}

void attribute_hidden R_SetPPSize(R_size_t size)
{
    return;
}

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

/* move all nodes on from_peg to to_peg */
#define BULK_MOVE(from_peg,to_peg) do { \
SEXP __from__ = (from_peg); \
SEXP __to__ = (to_peg); \
SEXP first_old = NEXT_NODE(__from__); \
SEXP last_old = PREV_NODE(__from__); \
SEXP first_new = NEXT_NODE(__to__); \
SET_PREV_NODE(first_old, __to__); \
SET_NEXT_NODE(__to__, first_old); \
SET_PREV_NODE(first_new, last_old); \
SET_NEXT_NODE(last_old, first_new); \
SET_NEXT_NODE(__from__, __from__); \
SET_PREV_NODE(__from__, __from__); \
} while (0);


/* Processing Node Children */

/* This macro calls dc__action__ for each child of __n__, passing
 dc__extra__ as a second argument for each call. */
/* When the CHARSXP hash chains are maintained through the ATTRIB
 field it is important that we NOT trace those fields otherwise too
 many CHARSXPs will be kept alive artificially. As a safety we don't
 ignore all non-NULL ATTRIB values for CHARSXPs but only those that
 are themselves CHARSXPs, which is what they will be if they are
 part of a hash chain.  Theoretically, for CHARSXPs the ATTRIB field
 should always be either R_NilValue or a CHARSXP. */
#ifdef PROTECTCHECK
# define HAS_GENUINE_ATTRIB(x) \
(TYPEOF(x) != FREESXP && ATTRIB(x) != R_NilValue && \
(TYPEOF(x) != CHARSXP || TYPEOF(ATTRIB(x)) != CHARSXP))
#else
# define HAS_GENUINE_ATTRIB(x) \
(ATTRIB(x) != R_NilValue && \
(TYPEOF(x) != CHARSXP || TYPEOF(ATTRIB(x)) != CHARSXP))
#endif

#ifdef PROTECTCHECK
#define FREE_FORWARD_CASE case FREESXP: if (gc_inhibit_release) break;
#else
#define FREE_FORWARD_CASE
#endif
#define DO_CHILDREN(__n__,dc__action__,dc__extra__) do { \
if (HAS_GENUINE_ATTRIB(__n__)) \
dc__action__(ATTRIB(__n__), dc__extra__); \
switch (TYPEOF(__n__)) { \
case NILSXP: \
case BUILTINSXP: \
case SPECIALSXP: \
case CHARSXP: \
case LGLSXP: \
case INTSXP: \
case REALSXP: \
case CPLXSXP: \
case WEAKREFSXP: \
case RAWSXP: \
case S4SXP: \
break; \
case STRSXP: \
case EXPRSXP: \
case VECSXP: \
{ \
int i; \
for (i = 0; i < LENGTH(__n__); i++) \
dc__action__(STRING_ELT(__n__, i), dc__extra__); \
} \
break; \
case ENVSXP: \
dc__action__(FRAME(__n__), dc__extra__); \
dc__action__(ENCLOS(__n__), dc__extra__); \
dc__action__(HASHTAB(__n__), dc__extra__); \
break; \
case CLOSXP: \
case PROMSXP: \
case LISTSXP: \
case LANGSXP: \
case DOTSXP: \
case SYMSXP: \
case BCODESXP: \
dc__action__(TAG(__n__), dc__extra__); \
dc__action__(CAR(__n__), dc__extra__); \
dc__action__(CDR(__n__), dc__extra__); \
break; \
case EXTPTRSXP: \
dc__action__(EXTPTR_PROT(__n__), dc__extra__); \
dc__action__(EXTPTR_TAG(__n__), dc__extra__); \
break; \
FREE_FORWARD_CASE \
default: \
register_bad_sexp_type(__n__, __LINE__);		\
} \
} while(0)


/* Forwarding Nodes.  These macros mark nodes or children of nodes and
 place them on the forwarding list.  The forwarding list is assumed
 to be in a local variable of the caller named named
 forwarded_nodes. */

#define FORWARD_NODE(s) do { \
SEXP fn__n__ = (s); \
if (fn__n__ && ! NODE_IS_MARKED(fn__n__)) { \
CHECK_FOR_FREE_NODE(fn__n__) \
MARK_NODE(fn__n__); \
UNSNAP_NODE(fn__n__); \
SET_NEXT_NODE(fn__n__, forwarded_nodes); \
forwarded_nodes = fn__n__; \
} \
} while (0)

#define FC_FORWARD_NODE(__n__,__dummy__) FORWARD_NODE(__n__)
#define FORWARD_CHILDREN(__n__) DO_CHILDREN(__n__,FC_FORWARD_NODE, 0)

/* This macro should help localize where a FREESXP node is encountered
 in the GC */
#ifdef PROTECTCHECK
#define CHECK_FOR_FREE_NODE(s) { \
SEXP cf__n__ = (s); \
if (TYPEOF(cf__n__) == FREESXP && ! gc_inhibit_release) \
register_bad_sexp_type(cf__n__, __LINE__); \
}
#else
#define CHECK_FOR_FREE_NODE(s)
#endif


/* Node Allocation. */

#define CLASS_GET_FREE_NODE(c,s) do { \
(s) = malloc ( NodeClassSize[(c)] + sizeof(VECTOR_SEXPREC) ); \
} while ( 0 )

#define NO_FREE_NODES() (R_NodesInUse >= R_NSize)
#define GET_FREE_NODE(s) CLASS_GET_FREE_NODE(0,s)


/* Debugging Routines. */

#ifdef DEBUG_GC
static void CheckNodeGeneration(SEXP x, int g)
{
    return;
}

static void DEBUG_CHECK_NODE_COUNTS(char *where)
{
    return;
}

static void DEBUG_GC_SUMMARY(int full_gc)
{
    return;
}
#else
#define DEBUG_CHECK_NODE_COUNTS(s)
#define DEBUG_GC_SUMMARY(x)
#endif /* DEBUG_GC */

#ifdef DEBUG_ADJUST_HEAP
static void DEBUG_ADJUST_HEAP_PRINT(double node_occup, double vect_occup)
{
    return;
}
#else
#define DEBUG_ADJUST_HEAP_PRINT(node_occup, vect_occup)
#endif /* DEBUG_ADJUST_HEAP */

#ifdef DEBUG_RELEASE_MEM
static void DEBUG_RELEASE_PRINT(int rel_pages, int maxrel_pages, int i)
{
    return;
}
#else
#define DEBUG_RELEASE_PRINT(rel_pages, maxrel_pages, i)
#endif /* DEBUG_RELEASE_MEM */


/* Page Allocation and Release. */

static void GetNewPage(int node_class)
{
    return;
}

static void ReleasePage(PAGE_HEADER *page, int node_class)
{
    return;
}

static void TryToReleasePages(void)
{
    return;
}

/* compute size in VEC units so result will fit in LENGTH field for FREESXPs */
static R_INLINE R_size_t getVecSizeInVEC(SEXP s)
{
    return 0;
}

static void ReleaseLargeFreeVectors(void)
{
    return;
}


/* Heap Size Adjustment. */

static void AdjustHeapSize(R_size_t size_needed)
{
    return;
}


/* Managing Old-to-New References. */

#define AGE_NODE(s,g) do { \
SEXP an__n__ = (s); \
int an__g__ = (g); \
if (an__n__ && NODE_GEN_IS_YOUNGER(an__n__, an__g__)) { \
if (NODE_IS_MARKED(an__n__)) \
R_GenHeap[NODE_CLASS(an__n__)].OldCount[NODE_GENERATION(an__n__)]--; \
else \
MARK_NODE(an__n__); \
SET_NODE_GENERATION(an__n__, an__g__); \
UNSNAP_NODE(an__n__); \
SET_NEXT_NODE(an__n__, forwarded_nodes); \
forwarded_nodes = an__n__; \
} \
} while (0)

static void AgeNodeAndChildren(SEXP s, int gen)
{
    return;
}

static void old_to_new(SEXP x, SEXP y)
{
    return;
}

#define CHECK_OLD_TO_NEW(x,y) do { \
if (NODE_IS_OLDER(CHK(x), CHK(y))) old_to_new(x,y);  } while (0)


/*******************************************************************************
 * MEMORY ALLOCATION FUNCTIONS
 ******************************************************************************/
/* Node Sorting.  SortNodes attempts to improve locality of reference
 by rearranging the free list to place nodes on the same place page
 together and order nodes within pages.  This involves a sweep of the
 heap, so it should not be done too often, but doing it at least
 occasionally does seem essential.  Sorting on each full colllection is
 probably sufficient.
 */

#define SORT_NODES
#ifdef SORT_NODES
static void SortNodes(void)
{
    return;
}
#endif


/* Finalization and Weak References */

/* The design of this mechanism is very close to the one described in
 "Stretching the storage manager: weak pointers and stable names in
 Haskell" by Peyton Jones, Marlow, and Elliott (at
 www.research.microsoft.com/Users/simonpj/papers/weak.ps.gz). --LT */

static SEXP R_weak_refs = NULL;

#define READY_TO_FINALIZE_MASK 1

#define SET_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp |= READY_TO_FINALIZE_MASK)
#define CLEAR_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp &= ~READY_TO_FINALIZE_MASK)
#define IS_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp & READY_TO_FINALIZE_MASK)

#define FINALIZE_ON_EXIT_MASK 2

#define SET_FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp |= FINALIZE_ON_EXIT_MASK)
#define CLEAR_FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp &= ~FINALIZE_ON_EXIT_MASK)
#define FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp & FINALIZE_ON_EXIT_MASK)

#define WEAKREF_SIZE 4
#define WEAKREF_KEY(w) VECTOR_ELT(w, 0)
#define SET_WEAKREF_KEY(w, k) SET_VECTOR_ELT(w, 0, k)
#define WEAKREF_VALUE(w) VECTOR_ELT(w, 1)
#define SET_WEAKREF_VALUE(w, v) SET_VECTOR_ELT(w, 1, v)
#define WEAKREF_FINALIZER(w) VECTOR_ELT(w, 2)
#define SET_WEAKREF_FINALIZER(w, f) SET_VECTOR_ELT(w, 2, f)
#define WEAKREF_NEXT(w) VECTOR_ELT(w, 3)
#define SET_WEAKREF_NEXT(w, n) SET_VECTOR_ELT(w, 3, n)

static SEXP MakeCFinalizer(R_CFinalizer_t cfun);

static SEXP NewWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit)
{
    return 0;
}

SEXP R_MakeWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit)
{
    return 0;
}

SEXP R_MakeWeakRefC(SEXP key, SEXP val, R_CFinalizer_t fin, Rboolean onexit)
{
    return 0;
}

static void CheckFinalizers(void)
{
    return;
}

/* C finalizers are stored in a CHARSXP.  It would be nice if we could
 use EXTPTRSXP's but these only hold a void *, and function pointers
 are not guaranteed to be compatible with a void *.  There should be
 a cleaner way of doing this, but this will do for now. --LT */
/* Changed to RAWSXP in 2.8.0 */
static Rboolean isCFinalizer(SEXP fun)
{
    return 0;
}

static SEXP MakeCFinalizer(R_CFinalizer_t cfun)
{
    return 0;
}

static R_CFinalizer_t GetCFinalizer(SEXP fun)
{
    return 0;
}

SEXP R_WeakRefKey(SEXP w)
{
    return 0;
}

SEXP R_WeakRefValue(SEXP w)
{
    return 0;
}

void R_RunWeakRefFinalizer(SEXP w)
{
    return;
}

static Rboolean RunFinalizers(void)
{
    return 0;
}

void R_RunExitFinalizers(void)
{
    return;
}

void R_RegisterFinalizerEx(SEXP s, SEXP fun, Rboolean onexit)
{
    return;
}

void R_RegisterFinalizer(SEXP s, SEXP fun)
{
    return;
}

void R_RegisterCFinalizerEx(SEXP s, R_CFinalizer_t fun, Rboolean onexit)
{
    return;
}

void R_RegisterCFinalizer(SEXP s, R_CFinalizer_t fun)
{
    return;
}

/* R interface function */

SEXP attribute_hidden do_regFinaliz(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


/* The Generational Collector. */

#define PROCESS_NODES() do { \
while (forwarded_nodes != NULL) { \
s = forwarded_nodes; \
forwarded_nodes = NEXT_NODE(forwarded_nodes); \
SNAP_NODE(s, R_GenHeap[NODE_CLASS(s)].Old[NODE_GENERATION(s)]); \
R_GenHeap[NODE_CLASS(s)].OldCount[NODE_GENERATION(s)]++; \
FORWARD_CHILDREN(s); \
} \
} while (0)

static void RunGenCollect(R_size_t size_needed)
{
    return;
}

/* public interface for controlling GC torture settings */
/* maybe, but in no header */
void R_gc_torture(int gap, int wait, Rboolean inhibit)
{
    return;
}

SEXP attribute_hidden do_gctorture(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

SEXP attribute_hidden do_gctorture2(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* initialize gctorture settings from environment variables */
static void init_gctorture(void)
{
    return;
}

SEXP attribute_hidden do_gcinfo(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}

/* reports memory use to profiler in eval.c */

void attribute_hidden get_current_mem(unsigned long *smallvsize,
                                      unsigned long *largevsize,
                                      unsigned long *nodes)
{
    return;
}

SEXP attribute_hidden do_gc(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    return 0;
}


static void mem_err_heap(R_size_t size)
{
    assert(0);
    errorcall(R_NilValue, _("vector memory exhausted (limit reached?)"));
}


static void mem_err_cons(void)
{
    assert(0);
    errorcall(R_NilValue, _("cons memory exhausted (limit reached?)"));
}

static void mem_err_malloc(R_size_t size)
{
    assert(0);
    errorcall(R_NilValue, _("memory exhausted (limit reached?)"));
}

/* InitMemory : Initialise the memory to be used in R. */
/* This includes: stack space, node space and vector space */

#define PP_REDZONE_SIZE 1000L
static int R_StandardPPStackSize, R_RealPPStackSize;

void attribute_hidden InitMemory()
{
    UNMARK_NODE(&UnmarkedNodeTemplate);
    SET_NODE_CLASS(&UnmarkedNodeTemplate, 0);
    
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

void attribute_hidden FinalizeMemory()
{
    freeVector(R_NilValue);
}

void attribute_hidden freeVector(SEXP s)
{
    R_xlen_t length;
#ifdef LONG_VECTOR_SUPPORT
    if (IS_LONG_VEC(s))
    {
        length = LONG_VEC_LENGTH(s);
        if (length > R_SHORT_LEN_MAX) {
            s -= sizeof(R_long_vec_hdr_t);
        }
    }
#endif
    free(s);
    return;
}

/* Since memory allocated from the heap is non-moving, R_alloc just
 allocates off the heap as RAWSXP/REALSXP and maintains the stack of
 allocations through the ATTRIB pointer.  The stack pointer R_VStack
 is traced by the collector. */
void *vmaxget(void)
{
    return 0;
}

void vmaxset(const void *ovmax)
{
    return;
}

char *R_alloc(size_t nelem, int eltsize)
{
    return 0;
}



/* S COMPATIBILITY */

char *S_alloc(long nelem, int eltsize)
{
    return 0;
}


char *S_realloc(char *p, long new, long old, int size)
{
    return 0;
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
SEXP NewEnvironment(SEXP namelist, SEXP valuelist, SEXP rho)
{
    return 0;
}

/* mkPROMISE is defined directly do avoid the need to protect its arguments
 unless a GC will actually occur. */
SEXP attribute_hidden mkPROMISE(SEXP expr, SEXP rho)
{
    return 0;
}

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
                    // WE MUST EXIT THE PROGRAM!
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
                assert(0);
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
            //            R_GenHeap[LARGE_NODE_CLASS].AllocCount++;
            //            R_NodesInUse++;
            //            SNAP_NODE(s, R_GenHeap[LARGE_NODE_CLASS].New);
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

SEXP allocS4Object(void)
{
    return 0;
}


/* "gc" a mark-sweep or in-place generational garbage collector */

void R_gc(void)
{
    return;
}

static void R_gc_full(R_size_t size_needed)
{
    assert(0);
    return;
}

extern double R_getClockIncrement(void);
extern void R_getProcTime(double *data);

static double gctimes[5], gcstarttimes[5];
static Rboolean gctime_enabled = FALSE;

/* this is primitive */
SEXP attribute_hidden do_gctime(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

static void gc_start_timing(void)
{
    return;
}

static void gc_end_timing(void)
{
    return;
}

#define R_MAX(a,b) (a) < (b) ? (b) : (a)

static void R_gc_internal(R_size_t size_needed)
{
    assert(0);
    return;
}

SEXP attribute_hidden do_memlimits(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

SEXP attribute_hidden do_memoryprofile(SEXP call, SEXP op, SEXP args, SEXP env)
{
    return 0;
}

/* "protect" push a single argument onto R_PPStack */

/* In handling a stack overflow we have to be careful not to use
 PROTECT. error("protect(): stack overflow") would call deparse1,
 which uses PROTECT and segfaults.*/

/* However, the traceback creation in the normal error handler also
 does a PROTECT, as does the jumping code, at least if there are
 cleanup expressions to handle on the way out.  So for the moment
 we'll allocate a slightly larger PP stack and only enable the added
 red zone during handling of a stack overflow error.  LT */

static void reset_pp_stack(void *data)
{
    return;
}

SEXP protect(SEXP s)
{
    return 0;
}


/* "unprotect" pop argument list from top of R_PPStack */

void unprotect(int l)
{
    return;
}


/* "unprotect_ptr" remove pointer from somewhere in R_PPStack */

void unprotect_ptr(SEXP s)
{
    return;
}

/* Debugging function:  is s protected? */

int Rf_isProtected(SEXP s)
{
    return 0;
}


void R_ProtectWithIndex(SEXP s, PROTECT_INDEX *pi)
{
    return;
}

void R_Reprotect(SEXP s, PROTECT_INDEX i)
{
    return;
}

#ifdef UNUSED
/* remove all objects from the protection stack from index i upwards
 and return them in a vector. The order in the vector is from new
 to old. */
SEXP R_CollectFromIndex(PROTECT_INDEX i)
{
    return 0;
}
#endif

/* "initStack" initialize environment stack */
attribute_hidden
void initStack(void)
{
    return;
}


/* S-like wrappers for calloc, realloc and free that check for error
 conditions */

void *R_chk_calloc(size_t nelem, size_t elsize)
{
    return 0;
}

void *R_chk_realloc(void *ptr, size_t size)
{
    return 0;
}

void R_chk_free(void *ptr)
{
    return;
}

/* This code keeps a list of objects which are not assigned to variables
 but which are required to persist across garbage collections.  The
 objects are registered with R_PreserveObject and deregistered with
 R_ReleaseObject. */

void R_PreserveObject(SEXP object)
{
    return;
}

static SEXP RecursiveRelease(SEXP object, SEXP list)
{
    return 0;
}

void R_ReleaseObject(SEXP object)
{
    return;
}


/* External Pointer Objects */
SEXP R_MakeExternalPtr(void *p, SEXP tag, SEXP prot)
{
    return 0;
}

void *R_ExternalPtrAddr(SEXP s)
{
    return 0;
}

SEXP R_ExternalPtrTag(SEXP s)
{
    return 0;
}

SEXP R_ExternalPtrProtected(SEXP s)
{
    return 0;
}

void R_ClearExternalPtr(SEXP s)
{
    return;
}

void R_SetExternalPtrAddr(SEXP s, void *p)
{
    return;
}

void R_SetExternalPtrTag(SEXP s, SEXP tag)
{
    return;
}

void R_SetExternalPtrProtected(SEXP s, SEXP p)
{
    return;
}

/* Work around casting issues: works where it is needed */
typedef union {void *p; DL_FUNC fn;} fn_ptr;

/* used in package methods */
SEXP R_MakeExternalPtrFn(DL_FUNC p, SEXP tag, SEXP prot)
{
    return 0;
}

attribute_hidden
DL_FUNC R_ExternalPtrAddrFn(SEXP s)
{
    return 0;
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
void DUPLICATE_ATTRIB(SEXP to, SEXP from) {
    SET_ATTRIB(CHK(to), duplicate(CHK(ATTRIB(CHK(from)))));
    SET_OBJECT(CHK(to), OBJECT(from));
    IS_S4_OBJECT(from) ?  SET_S4_OBJECT(to) : UNSET_S4_OBJECT(to);
}

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

/*******************************************/
/* Non-sampling memory use profiler
 reports all large vector heap
 allocations and all calls to GetNewPage */
/*******************************************/

#ifndef R_MEMORY_PROFILING

SEXP do_Rprofmem(SEXP args)
{
    return 0;
}

#else
static int R_IsMemReporting;  /* Rboolean more appropriate? */
static FILE *R_MemReportingOutfile;
static R_size_t R_MemReportingThreshold;

static void R_OutputStackTrace(FILE *file)
{
    return;
}

static void R_ReportAllocation(R_size_t size)
{
    return;
}

static void R_ReportNewPage(void)
{
    return;
}

static void R_EndMemReporting()
{
    return;
}

static void R_InitMemReporting(SEXP filename, int append,
                               R_size_t threshold)
{
    return;
}

SEXP do_Rprofmem(SEXP args)
{
    return 0;
}

#endif /* R_MEMORY_PROFILING */

/* RBufferUtils, moved from deparse.c */

#include "RBufferUtils.h"

void *R_AllocStringBuffer(size_t blen, R_StringBuffer *buf)
{
    return 0;
}

void
R_FreeStringBuffer(R_StringBuffer *buf)
{
    return;
}

void attribute_hidden
R_FreeStringBufferL(R_StringBuffer *buf)
{
    return;
}

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
