cmake_minimum_required (VERSION 2.8)
project (ssa)

set(SRCS
alder_ssa.c
alder_ssa.h
alder_ssa_core.c
alder_ssa_core.h
alder_ssa_khan09.c
alder_ssa_khan09.h)

ADD_LIBRARY(ssa STATIC ${SRCS})
