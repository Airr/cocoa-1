cmake_minimum_required (VERSION 2.8)
project (fastq)

set(SRCS
alder_fastq_kseq.c
alder_fastq_kseq.h
alder_fastq_pair.c
alder_fastq_pair.h)

ADD_LIBRARY(fastq STATIC ${SRCS})
