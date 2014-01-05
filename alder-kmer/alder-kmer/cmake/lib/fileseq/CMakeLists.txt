cmake_minimum_required (VERSION 2.8)
project (fileseq)

set(SRCS
alder_fileseq.c
alder_fileseq.h
alder_fileseq_chunk.c
alder_fileseq_chunk.h
alder_fileseq_kmer.c
alder_fileseq_kmer.h
alder_fileseq_sample.c
alder_fileseq_sample.h
alder_fileseq_sequenceiterator.h
alder_fileseq_streamer.c
alder_fileseq_streamer.h
alder_fileseq_token.c
alder_fileseq_token.h
alder_fileseq_type.c
alder_fileseq_type.h
alder_fileseq_write.c
alder_fileseq_write.h)

ADD_LIBRARY(fileseq STATIC ${SRCS})
