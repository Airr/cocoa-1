cmake_minimum_required (VERSION 2.8)
project (p99)

set(SRCS
alder_p99.c
alder_p99.h)

ADD_LIBRARY(p99 STATIC ${SRCS})
