#!/bin/bash

#  alder_kmer_test_partition.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 1/10/14.
#  Copyright (c) 2014 Sang Chul Choi. All rights reserved.

###############################################################################
# Command arguments
###############################################################################
kmersize=$1
maxkmer=$2
duplicate=$3
format=$4
compress=$5
nthread=$6
version=$7
skipdata=$8

# Check arguments.
if [ $# -lt 2 ]; then
  echo "Two arguments, kmersize and maxkmer, are required arguments."
  echo "SYNOPSIS:"
  echo "alder_kmer_large.sh kmersize maxkmer duplicate format compress nthread version skip-data"
  echo "kmersize  : number between 1-64"
  echo "maxkmer   : number (1000-)"
  echo "duplicate : number (1-)"
  echo "format    : fq or fa"
  echo "compress  : [no|gz|bz2]"
  echo "nthread   : number (1-)"
  echo "version   : 1,2,3"
  echo "skip-data : no or yes"
  echo "e.g.,"
  echo "$ ./alder_kmer_test_partition.sh 4 1000 65535 fq no 1 3 no"
  exit
fi

if [ -z "$3" ]; then
  duplicate=65535
fi

if [ -z "$4" ]; then
  format=fa
fi

if [ -z "$5" ]; then
  compress=""
fi

if [ -z "$6" ]; then
  nthread=1
fi

if [ -z "$7" ]; then
  version=3
fi

if [ -z "$8" ]; then
  skipdata="no"
fi

hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "*** START ***"
echo "*** Simulation Setup ***"
echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Duplicates:              $duplicate"
echo "Format of sequence file: $format"
echo "Compressed:              $compress"
echo "Number of threads:       $nthread"
echo "Version:                 $version"
echo "Hash table size:         $hashtablesize"

###############################################################################
# Functions
###############################################################################

echoerr() { echo "$@" 1>&2; }

run_command() {
  echo "$ $command"
  $command
}

function multiply_file() {
  IN="${1}"
  OUT="${2}"
  N="${3}"
  for i in $(eval echo {1..$N}); do
    echo "${IN}"
  done | xargs cat > "${OUT}"
}

measure_start_time() {
  start_time=`date +%s`
}

measure_end_time() {
  end_time=`date +%s`
  echo Execution time was `expr $end_time - $start_time` s.
}

###############################################################################
# Main
###############################################################################
disk=1000
memory=200

###############################################################################
# Sequence Generation
###############################################################################
if [ "$skipdata" == "no" ]; then
echo "simulating a count data ..."
command="./alder-kmer simulate -k $kmersize --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer"
run_command
# No consistency check.
#if [ $? -ne 0 ]; then
#  echo "Crash!"
#  touch "crash-simulate-$kmersize"
#  exit
#fi

# bump the file up!
seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "bumping up ..."
multiply_file outfile-00.$format outfile.$format $duplicate

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size after x65535: $((seqfilesize / 10**6)) MB."

if [ "$compress" == "gz" ]; then
echo -n "creating a compressed file using gzip... "
command="gzip outfile.$format"
run_command
echo "done!"
zip=.gz
elif [ "$compress" == "bz2" ]; then
echo -n "creating a compressed file using bzip2... "
command="bzip2 outfile.$format"
run_command
echo "done!"
zip=.bz2
else
zip=""
fi
echo "  created file: outfile.$format$zip"
echo "  created file: outfile_0-0.par"
else
echo "skipped simulation"
fi

###############################################################################
# Sequence to Partition (Encoded Kmer)
###############################################################################
measure_start_time
echo "partitioning the sequence file... "
command="./alder-kmer partition -k $kmersize --progress --nthread=$nthread --select-version=$version --memory=$memory outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-partition-$kmersize"
  exit
fi
echo "done!"
echo "  created file: outfile-0-0.par"
measure_end_time

echo "*** END ***"
echo ""



