#!/bin/bash
# vi copy.c init.c prop.c subvector.c view.c file.c minmax.c oper.c swap.c 

# bash preprocess-vector.sh anchor 

NAME=$1

CAPNAME=$(echo $NAME | tr '[a-z]' '[A-Z]')

cat>../../vector/init.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "init_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/file.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "file_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/copy.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "copy_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/oper.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "oper_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/prop.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "prop_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/swap.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "swap_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/view.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "view_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/minmax.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "minmax_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../vector/subvector.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "subvector_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>$NAME.h<<EOF
/* vector/gsl_vector.h */
#ifndef _GSL_VECTOR_${CAPNAME}_H_
#define _GSL_VECTOR_${CAPNAME}_H_

#include <stdlib.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_inline.h>
#include <gsl/gsl_check_range.h>
#include "gsl_block_${NAME}.h"

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

/* C99 not for GNU */
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#ifndef INLINE_FUN
#define INLINE_FUN inline
#endif

#ifndef INLINE_DECL 
#define INLINE_DECL inline
#endif

typedef struct 
{
  size_t size;
  size_t stride;
  alder_${NAME}_t *data;
  gsl_block_${NAME} *block;
  int owner;
} 
gsl_vector_${NAME};

typedef struct
{
  gsl_vector_${NAME} vector;
} _gsl_vector_${NAME}_view;

typedef _gsl_vector_${NAME}_view gsl_vector_${NAME}_view;

typedef struct
{
  gsl_vector_${NAME} vector;
} _gsl_vector_${NAME}_const_view;

typedef const _gsl_vector_${NAME}_const_view gsl_vector_${NAME}_const_view;

/* Addition */
#ifndef GSLVECTORSIZE
#define GSLVECTORSIZE 10
#endif

#ifndef GSLVECTORINCSIZE
#define GSLVECTORINCSIZE 1.2
#endif

/* Allocation */
gsl_vector_${NAME} *gsl_vector_${NAME}_init ();

gsl_vector_${NAME} *gsl_vector_${NAME}_alloc (const size_t n);
gsl_vector_${NAME} *gsl_vector_${NAME}_calloc (const size_t n);

gsl_vector_${NAME} *gsl_vector_${NAME}_alloc_from_block (gsl_block_${NAME} * b,
                                                     const size_t offset, 
                                                     const size_t n, 
                                                     const size_t stride);

gsl_vector_${NAME} *gsl_vector_${NAME}_alloc_from_vector (gsl_vector_${NAME} * v,
                                                      const size_t offset, 
                                                      const size_t n, 
                                                      const size_t stride);

void gsl_vector_${NAME}_free (gsl_vector_${NAME} * v);

/* Views */

_gsl_vector_${NAME}_view 
gsl_vector_${NAME}_view_array (alder_${NAME}_t *v, size_t n);

_gsl_vector_${NAME}_view 
gsl_vector_${NAME}_view_array_with_stride (alder_${NAME}_t *base,
                                         size_t stride,
                                         size_t n);

_gsl_vector_${NAME}_const_view 
gsl_vector_${NAME}_const_view_array (const alder_${NAME}_t *v, size_t n);

_gsl_vector_${NAME}_const_view 
gsl_vector_${NAME}_const_view_array_with_stride (const alder_${NAME}_t *base,
                                               size_t stride,
                                               size_t n);

_gsl_vector_${NAME}_view 
gsl_vector_${NAME}_subvector (gsl_vector_${NAME} *v, 
                            size_t i, 
                            size_t n);

_gsl_vector_${NAME}_view 
gsl_vector_${NAME}_subvector_with_stride (gsl_vector_${NAME} *v, 
                                        size_t i,
                                        size_t stride,
                                        size_t n);

_gsl_vector_${NAME}_const_view 
gsl_vector_${NAME}_const_subvector (const gsl_vector_${NAME} *v, 
                                  size_t i, 
                                  size_t n);

_gsl_vector_${NAME}_const_view 
gsl_vector_${NAME}_const_subvector_with_stride (const gsl_vector_${NAME} *v, 
                                              size_t i, 
                                              size_t stride,
                                              size_t n);

/* Operations */

void gsl_vector_${NAME}_set_zero (gsl_vector_${NAME} * v);
void gsl_vector_${NAME}_set_all (gsl_vector_${NAME} * v, alder_${NAME}_t x);

int gsl_vector_${NAME}_fread (FILE * stream, gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_fwrite (FILE * stream, const gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_fscanf (FILE * stream, gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_fprintf (FILE * stream, const gsl_vector_${NAME} * v,
                              const char *format);

int gsl_vector_${NAME}_memcpy (gsl_vector_${NAME} * dest, const gsl_vector_${NAME} * src);

int gsl_vector_${NAME}_reverse (gsl_vector_${NAME} * v);

int gsl_vector_${NAME}_swap (gsl_vector_${NAME} * v, gsl_vector_${NAME} * w);
int gsl_vector_${NAME}_swap_elements (gsl_vector_${NAME} * v, const size_t i, const size_t j);

alder_${NAME}_t gsl_vector_${NAME}_max (const gsl_vector_${NAME} * v);
alder_${NAME}_t gsl_vector_${NAME}_min (const gsl_vector_${NAME} * v);
void gsl_vector_${NAME}_minmax (const gsl_vector_${NAME} * v, alder_${NAME}_t * min_out, alder_${NAME}_t * max_out);

size_t gsl_vector_${NAME}_max_index (const gsl_vector_${NAME} * v);
size_t gsl_vector_${NAME}_min_index (const gsl_vector_${NAME} * v);
void gsl_vector_${NAME}_minmax_index (const gsl_vector_${NAME} * v, size_t * imin, size_t * imax);

int gsl_vector_${NAME}_add (gsl_vector_${NAME} * a, const gsl_vector_${NAME} * b);
int gsl_vector_${NAME}_sub (gsl_vector_${NAME} * a, const gsl_vector_${NAME} * b);
int gsl_vector_${NAME}_mul (gsl_vector_${NAME} * a, const gsl_vector_${NAME} * b);
int gsl_vector_${NAME}_div (gsl_vector_${NAME} * a, const gsl_vector_${NAME} * b);
int gsl_vector_${NAME}_scale (gsl_vector_${NAME} * a, const double x);
int gsl_vector_${NAME}_add_constant (gsl_vector_${NAME} * a, const double x);

int gsl_vector_${NAME}_equal (const gsl_vector_${NAME} * u, 
                            const gsl_vector_${NAME} * v);

int gsl_vector_${NAME}_isnull (const gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_ispos (const gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_isneg (const gsl_vector_${NAME} * v);
int gsl_vector_${NAME}_isnonneg (const gsl_vector_${NAME} * v);

INLINE_DECL alder_${NAME}_t gsl_vector_${NAME}_get (const gsl_vector_${NAME} * v, const size_t i);
INLINE_DECL void gsl_vector_${NAME}_set (gsl_vector_${NAME} * v, const size_t i, alder_${NAME}_t x);
INLINE_DECL alder_${NAME}_t * gsl_vector_${NAME}_ptr (gsl_vector_${NAME} * v, const size_t i);
INLINE_DECL const alder_${NAME}_t * gsl_vector_${NAME}_const_ptr (const gsl_vector_${NAME} * v, const size_t i);

#ifdef HAVE_INLINE

INLINE_FUN
alder_${NAME}_t
gsl_vector_${NAME}_get (const gsl_vector_${NAME} * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VAL ("index out of range", GSL_EINVAL, 0);
    }
#endif
  return v->data[i * v->stride];
}

INLINE_FUN
void
gsl_vector_${NAME}_set (gsl_vector_${NAME} * v, const size_t i, alder_${NAME}_t x)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
    }
#endif
  v->data[i * v->stride] = x;
}

INLINE_FUN
alder_${NAME}_t *
gsl_vector_${NAME}_ptr (gsl_vector_${NAME} * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (alder_${NAME}_t *) (v->data + i * v->stride);
}

INLINE_FUN
const alder_${NAME}_t *
gsl_vector_${NAME}_const_ptr (const gsl_vector_${NAME} * v, const size_t i)
{
#if GSL_RANGE_CHECK
  if (GSL_RANGE_COND(i >= v->size))
    {
      GSL_ERROR_NULL ("index out of range", GSL_EINVAL);
    }
#endif
  return (const alder_${NAME}_t *) (v->data + i * v->stride);
}
#endif /* HAVE_INLINE */

__END_DECLS

#endif /* _GSL_VECTOR_${CAPNAME}_H_ */
EOF

for i in copy init prop subvector view file minmax oper swap; do
gcc -DHAVE_CONFIG_H -I. -I../../vector -I.. -I../.. -g -O2 -MT $i.lo -MD -MP -MF .deps/$i.Tpo -E ../../vector/$i.c  -fno-common -DPIC -o $i.c
# cp ../../vector/$i.c /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/vector
done
cp ../../templates_on.h /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl
cp ../../templates_off.h /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl
cp preprocess-vector.sh /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/vector

mv $NAME.h gsl_vector.h
cat init.c file.c copy.c swap.c | grep -v "^#" > gsl_vector.c
echo See gsl_vector.*

