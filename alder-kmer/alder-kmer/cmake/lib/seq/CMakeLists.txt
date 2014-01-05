cmake_minimum_required (VERSION 2.8)
project (seq)

set(SRCS
alder_seq.c
alder_seq.h
alder_seq_chunk.c
alder_seq_chunk.h
alder_seq_sample.c
alder_seq_sample.h
alder_seq_streamer.c
alder_seq_streamer.h
alder_seq_write.c
alder_seq_write.h)

ADD_LIBRARY(seq STATIC ${SRCS})
