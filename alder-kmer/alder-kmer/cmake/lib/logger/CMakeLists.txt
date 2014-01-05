cmake_minimum_required (VERSION 2.8)
project (logger)

set(SRCS
alder_logger.c
alder_logger.h
alder_logger_error.c
alder_logger_error.h)

ADD_LIBRARY(logger STATIC ${SRCS})
