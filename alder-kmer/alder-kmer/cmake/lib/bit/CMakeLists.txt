cmake_minimum_required (VERSION 2.8)
project (bit)

set(SRCS
alder_bit.c
alder_bit.h)

ADD_LIBRARY(bit STATIC ${SRCS})
