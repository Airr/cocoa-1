#!/bin/bash

#  alder_kmer_test_binary.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 1/21/14.
#  Copyright (c) 2014 Sang Chul Choi. All rights reserved.

###############################################################################
# Command arguments
###############################################################################

kmersize=$1
maxkmer=$2
duplicate=$3
format=$4
compress=$5
version=$6

# Check arguments.
if [ $# -lt 2 ]; then
echo "kmersize maxkmer are required arguments."
echo "alder_kmer_test_binary.sh kmersize maxkmer duplicate format compress version"
echo "kmersize  : number between 1-64"
echo "maxkmer   : number (1000-)"
echo "duplicate : number (1-)"
echo "format    : fq or fa"
echo "compress  : [no|gz|bz2]"
echo "version   : 1,2,3,4,5"
echo "e.g.,"
echo "$ ./alder_kmer_test_binary.sh 25 50000 65535 fq no 5"
echo "About 9GB"
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
version=5
fi


hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "*** START ***"
echo "*** Simulation Setup ***"
echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Duplicates:              $duplicate"
echo "Format of sequence file: $format"
echo "Compressed:              $compress"
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

# Options:
# K
# maxkmer
# memory
# disk
# nthread

###############################################################################
# Sequence Generation
###############################################################################
touch START
echo "simulating a kmer count data ..."

command="./alder-kmer simulate -k $kmersize --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer"
run_command

seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "bumping up ..."
multiply_file outfile-00.$format outfile.$format $duplicate

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size after x$duplicate: $((seqfilesize / 10**6)) MB."


###############################################################################
# Sequence to binary (Counting...)
###############################################################################
measure_start_time
echo "binary ..."
command="./alder-kmer binary --select-version=$version --progress --log outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
echo "Crash!"
touch "crash-partition-$kmersize"
exit
fi
echo "  created file: outfile.bin"
measure_end_time

###############################################################################
# Binary to a simple-form sequence file
###############################################################################
measure_start_time
echo "uncompress ..."
command="./alder-kmer uncompress --select-version=$version --progress --log outfile.bin"
./alder-kmer uncompress --select-version=$version --progress --log outfile.bin > outfile.unc
# run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-uncompress-$kmersize"
  exit
fi
echo "  created file: outfile.unc"
measure_end_time

###############################################################################
# Using Awk to convert the binary to a simple-form sequence file
###############################################################################
measure_start_time
echo "using awk ..."
echo "awk 'NR % 4 == 2' outfile.fq > outfile.seq"
awk 'NR % 4 == 2' outfile.fq > outfile.seq
#run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-uncompress-$kmersize"
  exit
fi
echo "  created file: outfile.seq"
measure_end_time

A=$(md5 outfile.unc | cut -d" " -f 4)
B=$(md5 outfile.seq | cut -d" " -f 4)

if [ "$A" == "$B" ]; then
  echo "Pass!"
else
  echo "Crash!"
  touch "crash-compare-$kmersize"
  exit
fi
touch FINISH

echo "*** END ***"
echo ""

