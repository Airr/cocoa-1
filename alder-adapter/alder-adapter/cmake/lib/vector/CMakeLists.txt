cmake_minimum_required (VERSION 2.8)
project (vector)

set(SRCS
alder_vector.c
alder_vector.h
alder_vector_pair.c
alder_vector_pair.h)

ADD_LIBRARY(vector STATIC ${SRCS})
