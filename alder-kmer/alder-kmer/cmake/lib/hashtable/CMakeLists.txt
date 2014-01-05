cmake_minimum_required (VERSION 2.8)
project (hashtable)

set(SRCS
alder_hashtable.c
alder_hashtable.h
alder_hashtable_dsk.c
alder_hashtable_dsk.h
alder_hashtable_klib.c
alder_hashtable_klib.h
alder_hashtable_mcas.c
alder_hashtable_mcas.h
alder_hashtable_ulib.c
alder_hashtable_ulib.h
alder_hashtable_uthash.c
alder_hashtable_uthash.h)

ADD_LIBRARY(hashtable STATIC ${SRCS})
