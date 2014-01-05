cmake_minimum_required (VERSION 2.8)
project (dna)

set(SRCS
alder_dna.c
alder_dna.h)

ADD_LIBRARY(dna STATIC ${SRCS})
