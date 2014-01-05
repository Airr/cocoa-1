cmake_minimum_required (VERSION 2.8)
project (token)

set(SRCS
alder_token.c
alder_token.h
alder_token_dna.c
alder_token_dna.h)

ADD_LIBRARY(token STATIC ${SRCS})
