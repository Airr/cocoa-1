cmake_minimum_required (VERSION 2.8)
project (divsufsort)

set(SRCS
alder_divsufsort.c
alder_divsufsort.h
divsufsort.c
divsufsort64.h
divsufsort_config.h
divsufsort_private.h
lfs.h
sssort.c
trsort.c
utils.c)

ADD_LIBRARY(divsufsort STATIC ${SRCS})
