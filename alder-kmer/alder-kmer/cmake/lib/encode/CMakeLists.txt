cmake_minimum_required (VERSION 2.8)
project (encode)

set(SRCS
alder_encode.c
alder_encode.h
alder_encode_kmer.c
alder_encode_kmer.h
alder_encode_number.c
alder_encode_number.h
alder_encode_pack.c
alder_encode_pack.h)

ADD_LIBRARY(encode STATIC ${SRCS})
