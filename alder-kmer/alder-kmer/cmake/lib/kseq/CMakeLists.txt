cmake_minimum_required (VERSION 2.8)
project (kseq)

set(SRCS
alder_kseq.c
alder_kseq.h
alder_kseq_sequenceiterator.c
alder_kseq_sequenceiterator.h)

ADD_LIBRARY(kseq STATIC ${SRCS})
