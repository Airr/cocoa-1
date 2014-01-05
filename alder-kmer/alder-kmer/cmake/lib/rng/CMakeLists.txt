cmake_minimum_required (VERSION 2.8)
project (rng)

set(SRCS
alder_rng.c
alder_rng.h
rngs.c
rngs.h
rvgs.c
rvgs.h)

ADD_LIBRARY(rng STATIC ${SRCS})
