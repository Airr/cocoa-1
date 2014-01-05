cmake_minimum_required (VERSION 2.8)
project (vector)

set(SRCS
alder_gsl.c
alder_gsl.h
alder_vector.c
alder_vector.h
alder_vector_interval.c
alder_vector_interval.h
alder_vector_pair.c
alder_vector_pair.h
alder_vector_palign.c
alder_vector_palign.h
gsl_vector_anchor.c
gsl_vector_anchor.h
gsl_vector_match.c
gsl_vector_match.h
gsl_vector_sam.c
gsl_vector_sam.h
gsl_vector_seqpair.c
gsl_vector_seqpair.h)

find_package( GSL REQUIRED )
if ( GSL_FOUND )
  include_directories( ${GSL_INCLUDE_DIRS} )
endif( GSL_FOUND )

ADD_LIBRARY(vector STATIC ${SRCS})
