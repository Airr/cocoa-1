cmake_minimum_required (VERSION 2.8)
project (kmer)

set(SRCS
alder_kmer.c
alder_kmer.h
alder_kmer_count.c
alder_kmer_count.h
alder_kmer_decode.c
alder_kmer_decode.h
alder_kmer_encode.c
alder_kmer_encode.h
alder_kmer_report.c
alder_kmer_report.h
alder_kmer_simulate.c
alder_kmer_simulate.h
alder_kmer_thread.c
alder_kmer_thread.h)

ADD_LIBRARY(kmer STATIC ${SRCS})
