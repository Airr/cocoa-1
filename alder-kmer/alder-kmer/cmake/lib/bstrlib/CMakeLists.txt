cmake_minimum_required (VERSION 2.8)
project (bstrlib)

# add_definitions(-DBSTRLIB_VSNP_OK)

set(SRCS
bstrmore.c
bstrmore.h
bstraux.c
bstraux.h
bstrlib.c
bstrlib.h)

ADD_LIBRARY(bstrlib STATIC ${SRCS})
