cmake_minimum_required (VERSION 2.8)
project (fastq)

set(SRCS
alder_fastq.c
alder_fastq.h
alder_fastq_chunk.c
alder_fastq_chunk.h
alder_fastq_concat.c
alder_fastq_concat.h
alder_fastq_count_kmer.c
alder_fastq_count_kmer.h
alder_fastq_kseq_summary.c
alder_fastq_kseq_summary.h
alder_fastq_length.c
alder_fastq_length.h
alder_fastq_numbase.c
alder_fastq_numbase.h
alder_fastq_pair.c
alder_fastq_pair.h
alder_fastq_ragel.c
alder_fastq_ragel.h
alder_fastq_ragel_summary.c
alder_fastq_ragel_summary.h
alder_fastq_sequenceiterator.c
alder_fastq_sequenceiterator.h
alder_fastq_streamer.c
alder_fastq_streamer.h
alder_fastq_write.c
alder_fastq_write.h)

ADD_LIBRARY(fastq STATIC ${SRCS})
