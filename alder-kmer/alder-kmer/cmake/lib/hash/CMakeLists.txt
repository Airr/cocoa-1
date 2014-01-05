cmake_minimum_required (VERSION 2.8)
project (hash)

set(SRCS
alder_hash.c
alder_hash.h)

ADD_LIBRARY(hash STATIC ${SRCS})
