#!/bin/bash

# bash preprocess-block.sh anchor 

NAME=$1

CAPNAME=$(echo $NAME | tr '[a-z]' '[A-Z]')

cat>../../block/init.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "init_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../block/block.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "block_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>../../block/file.c<<EOF
#define BASE_ALDER_$CAPNAME
#include "templates_on.h"
#include "fwrite_source.c"
#include "fprintf_source.c"
#include "templates_off.h"
#undef  BASE_ALDER_$CAPNAME
EOF

cat>$NAME.h<<EOF
/* block/gsl_block.h */
#ifndef _GSL_BLOCK_${CAPNAME}_H_
#define _GSL_BLOCK_${CAPNAME}_H_

#include <stdlib.h>
#include <gsl/gsl_errno.h>
#include "gsl_vector_anchor.h"

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

struct alder_${NAME}_struct {
    char     *qname;
    uint16_t flag;
    char     *rname;
    uint32_t pos;
    uint8_t  mapq;
    char     *cigar;
    char     *rnext;
    uint32_t pnext;
    int32_t  tlen;
    char     *seq;
    char     *qual;
    char     *optional;
};

typedef struct alder_${NAME}_struct alder_${NAME}_t;

struct gsl_block_${NAME}_struct
{
  size_t size;
  alder_${NAME}_t *data;
};

typedef struct gsl_block_${NAME}_struct gsl_block_${NAME};

gsl_block_${NAME} *gsl_block_${NAME}_alloc (const size_t n);
gsl_block_${NAME} *gsl_block_${NAME}_calloc (const size_t n);
void gsl_block_${NAME}_free (gsl_block_${NAME} * b);

int gsl_block_${NAME}_fread (FILE * stream, gsl_block_${NAME} * b);
int gsl_block_${NAME}_fwrite (FILE * stream, const gsl_block_${NAME} * b);
int gsl_block_${NAME}_fscanf (FILE * stream, gsl_block_${NAME} * b);
int gsl_block_${NAME}_fprintf (FILE * stream, const gsl_block_${NAME} * b, const char *format);

int gsl_block_${NAME}_raw_fread (FILE * stream, alder_${NAME}_t * b, const size_t n, const size_t stride);
int gsl_block_${NAME}_raw_fwrite (FILE * stream, const alder_${NAME}_t * b, const size_t n, const size_t stride);
int gsl_block_${NAME}_raw_fscanf (FILE * stream, alder_${NAME}_t * b, const size_t n, const size_t stride);
int gsl_block_${NAME}_raw_fprintf (FILE * stream, const alder_${NAME}_t * b, const size_t n, const size_t stride, const char *format);

size_t gsl_block_${NAME}_size (const gsl_block_${NAME} * b);
alder_${NAME}_t * gsl_block_${NAME}_data (const gsl_block_${NAME} * b);
gsl_block_${NAME} * gsl_block_${NAME}_realloc (gsl_block_${NAME} *b, const size_t n);


__END_DECLS

#endif /* _GSL_BLOCK_${CAPNAME}_H_ */
EOF

for i in init block file test; do
gcc -DHAVE_CONFIG_H -I. -I../../block -I.. -I../.. -g -O2 -MT $i.lo -MD -MP -MF .deps/$i.Tpo -E ../../block/$i.c  -fno-common -DPIC -o $i.c
# cp ../../block/$i.c /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/block
done
cp ../../templates_on.h /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl
cp ../../templates_off.h /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl
cp preprocess-block.sh /Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/alder-align/gsl/block

mv $NAME.h gsl_block.h
cat init.c block.c file.c | grep -v "^#" > gsl_block.c
echo See gsl_block.*
