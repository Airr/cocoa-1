#!/bin/bash

#  alder_kmer_partition.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013,2014 Sang Chul Choi. All rights reserved.

###############################################################################
# Command arguments
###############################################################################
kmersize=$1
maxkmer=$2
format=$3
compress=$4
nthread=$5
version=$6

# Check arguments.
if [ $# -lt 2 ]; then
  echo kmersize maxkmer are required arguments.
  echo "alder_kmer_large.sh kmersize maxkmer format compress nthread version"
  echo "kmersize  : number between 1-64"
  echo "maxkmer   : number (1000-)"
  echo "format    : fq or fa"
  echo "compress  : [no|gz|bz2]"
  echo "nthread   : number (1-)"
  echo "version   : 1,2,3"
  echo "e.g.,"
  echo "$ ./alder_kmer_test_partition.sh 4 1000 fq no 1 3"
  exit
fi

if [ -z "$3" ]; then
  format=fa
fi

if [ -z "$4" ]; then
  compress=""
fi

if [ -z "$5" ]; then
  nthread=1
fi

if [ -z "$6" ]; then
  version=3
fi

hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "*** START ***"
echo "*** Simulation Setup ***"
echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Format of sequence file: $format"
echo "Compressed:              $compress"
echo "Number of threads:       $nthread"
echo "Version:                 $version"
echo "Hash table size:         $hashtablesize"

# Functions

echoerr() { echo "$@" 1>&2; }

run_command() {
  echo "$ $command"
  $command
}

measure_start_time() {
  start_time=`date +%s`
}

measure_end_time() {
  end_time=`date +%s`
  echo Execution time was `expr $end_time - $start_time` s.
}

# Main
memory=20

###############################################################################
# Sequence Generation
###############################################################################
echo "simulating a count data ..."
command="./alder-kmer simulate -k $kmersize --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer"
run_command

if [ "$compress" == "gz" ]; then
  echo "creating a compressed file using gzip... "
  command="gzip outfile-00.$format"
  run_command
  echo "done!"
  zip=.gz
elif [ "$compress" == "bz2" ]; then
  echo "creating a compressed file using bzip2... "
  command="bzip2 outfile-00.$format"
  run_command
  echo "done!"
  zip=.bz2
else
  zip=""
fi
echo "  created file: outfile-00.$format$zip"
echo "  created file: outfile_0-0.par"

###############################################################################
# Sequence to Partition (Encoded Kmer)
###############################################################################
measure_start_time
echo "partitioning the sequence file... "
command="./alder-kmer partition -k $kmersize --select-version=$version --nthread=$nthread --memory=$memory outfile-00.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-decode-$kmersize"
  exit
fi
echo "done!"
echo "  created file: outfile-0-0.par"
measure_end_time

echo "comparing outfile_0-0.par and outfile-0-0.par... "
if ! diff -q outfile-0-0.par outfile_0-0.par > /dev/null; then
  echoerr "K=$kmersize, format=$format Test failed because the expected value and the observed are different."
  cp outfile-0-0.par outfile-0-0.par.$kmersize-$format
  cp outfile_0-0.par outfile_0-0.par.$kmersize-$format
  cp outfile-00.$format$zip outfile-00.$format.$kmersize-$format$zip
  exit
fi
echo "done!"


###############################################################################
# Partition to Decoded Kmer
###############################################################################
echo "decoding outfile_0-0.par ... "
command="./alder-kmer decode -k $kmersize --select-version=$version outfile_0-0.par"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-decode-$kmersize"
  exit
fi
echo "done!"
echo "  created file: outfile.dec"
cut -f 1 outfile.dec|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.2
echo "  created file: outfile.2"

###############################################################################
# Partition to Table
###############################################################################
measure_start_time
echo "creating kmers using table command ... "
command="./alder-kmer table -k $kmersize --select-version=$version --nthread=$nthread --nh=$hashtablesize outfile_0-0.par"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-table-$kmersize"
  exit
fi
echo "done!"
echo "  created file: outfile.tbl"
measure_end_time

###############################################################################
# Report
###############################################################################
measure_start_time
echo "$ ./alder-kmer report outfile.tbl |cut -f1,3|sort -k 1|expand -t 1 > outfile.1"
./alder-kmer report outfile.tbl |cut -f1,3|sort -k 1|expand -t 1 > outfile.1
echo "  created file: outfile.1"
measure_end_time

echo -n "comparing outfile.1 and outfile.2 ... "
if ! diff -q outfile.1 outfile.2 > /dev/null; then
  echoerr "Fail: K=$kmersize, format=$format Test failed because somethings are different."
  touch "error-$kmersize"
  exit
fi
echo "Pass!"
echo "*** END ***"
echo ""
