cmake_minimum_required (VERSION 2.8)
project (fasta)

set(SRCS
alder_fasta.c
alder_fasta.h
alder_fasta_chunk.c
alder_fasta_chunk.h
alder_fasta_kmer.c
alder_fasta_kmer.h
alder_fasta_sequenceiterator.c
alder_fasta_sequenceiterator.h
alder_fasta_streamer.c
alder_fasta_streamer.h
alder_fasta_token.c
alder_fasta_token.h
alder_fasta_write.c
alder_fasta_write.h)

ADD_LIBRARY(fasta STATIC ${SRCS})
