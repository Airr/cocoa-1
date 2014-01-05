#!/bin/bash

KMERSIZE=$2
DISKSPACE=$3
MEMORYSPACE=$4
OUTDIR=$5
OUTFILE=$6
INFILE=$7
NTHREAD=$8

$1/alder-kmer count \
-k $KMERSIZE \
--nthread=$NTHREAD \
--disk=$DISKSPACE \
--memory=$MEMORYSPACE \
--outdir=$OUTDIR \
--outfile=$OUTFILE \
$INFILE \
--progress --progress-to-stderr \
--log

