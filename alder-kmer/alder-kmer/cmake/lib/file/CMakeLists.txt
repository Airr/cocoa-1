cmake_minimum_required (VERSION 2.8)
project (file)

set(SRCS
alder_file.c
alder_file.h
alder_file_availablediskspace.c
alder_file_availablediskspace.h
alder_file_availablememory.c
alder_file_availablememory.h
alder_file_availableopenfiles.c
alder_file_availableopenfiles.h
alder_file_creatable.c
alder_file_creatable.h
alder_file_exist.c
alder_file_exist.h
alder_file_firstline.c
alder_file_firstline.h
alder_file_format.c
alder_file_format.h
alder_file_getcwd.c
alder_file_getcwd.h
alder_file_info.c
alder_file_info.h
alder_file_isbinary.c
alder_file_isbinary.h
alder_file_isfasta.c
alder_file_isfasta.h
alder_file_isfastq.c
alder_file_isfastq.h
alder_file_isfile.c
alder_file_isfile.h
alder_file_isgzip.c
alder_file_isgzip.h
alder_file_isstdin.c
alder_file_isstdin.h
alder_file_makedtemp.c
alder_file_makedtemp.h
alder_file_mkdir.c
alder_file_mkdir.h
alder_file_numline.c
alder_file_numline.h
alder_file_rm.c
alder_file_rm.h
alder_file_sequencefilesize.c
alder_file_sequencefilesize.h
alder_file_size.c
alder_file_size.h
alder_file_sysctl.c
alder_file_sysctl.h
alder_file_whatformat.c
alder_file_whatformat.h
alder_file_writable.c
alder_file_writable.h)

ADD_LIBRARY(file STATIC ${SRCS})
