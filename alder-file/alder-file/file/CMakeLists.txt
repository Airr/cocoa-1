cmake_minimum_required (VERSION 2.8)
project (file)

set(SRCS
alder_file.c
alder_file.h
alder_file_creatable.c
alder_file_creatable.h
alder_file_exist.c
alder_file_exist.h
alder_file_isgzip.c
alder_file_isgzip.h
alder_file_isstdin.c
alder_file_isstdin.h
alder_file_writable.c
alder_file_writable.h)

ADD_LIBRARY(file STATIC ${SRCS})
