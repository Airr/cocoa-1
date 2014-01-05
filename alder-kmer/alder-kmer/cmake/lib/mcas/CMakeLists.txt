cmake_minimum_required (VERSION 2.8)
project (mcas)

set(SRCS
alder_mcas.c
alder_mcas.h
alder_mcas_wf.c
alder_mcas_wf.h)

ADD_LIBRARY(mcas STATIC ${SRCS})
