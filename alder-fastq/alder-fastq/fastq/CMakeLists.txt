cmake_minimum_required (VERSION 2.8)
project (fastq)

set(SRCS
alder_fastq.c
alder_fastq.h
alder_fastq_kseq.c
alder_fastq_kseq.h
alder_fastq_kseq_summary.c
alder_fastq_kseq_summary.h
alder_fastq_pair.c
alder_fastq_pair.h
alder_fastq_ragel.c
alder_fastq_ragel.h
alder_fastq_ragel_summary.c
alder_fastq_ragel_summary.h)

ADD_LIBRARY(fastq STATIC ${SRCS})
