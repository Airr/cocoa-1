cmake_minimum_required (VERSION 2.8)
project (fasta)

set(SRCS
alder_fasta.c
alder_fasta.h)

ADD_LIBRARY(fasta STATIC ${SRCS})
