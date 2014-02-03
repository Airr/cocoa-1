#!/bin/bash

#  alder_kmer_simple.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013,2014 Sang Chul Choi. All rights reserved.

###############################################################################
# Command arguments
###############################################################################

kmersize=$1
maxkmer=$2
duplicate=$3
format=$4
compress=$5
disk=$6
memory=$7
nthread=$8
version=$9

# Check arguments.
if [ $# -lt 2 ]; then
echo "kmersize maxkmer are required arguments."
echo "alder_kmer_simple.sh kmersize maxkmer duplicate format compress disk memory nthread version"
echo "kmersize  : number between 1-64"
echo "maxkmer   : number (10000)"
echo "duplicate : number (1)"
echo "format    : fq or fa"
echo "compress  : [no|gz|bz2]"
echo "disk      : number (default: 100 MB)"
echo "memory    : number (default: 10 MB)"
echo "nthread   : number (default: 1)"
echo "version   : 1,2,3,4,5,7 (default:7)"
echo "e.g.,"
echo "$ ./alder_kmer_test_count.sh 25 50000 1 fq no 100 10 1 7"
echo -n "Do you want to run it with default options [y] and press [ENTER]: "
read response
if [ "$response" == "y" ]; then
  kmersize=25
  maxkmer=50000
else
  exit
fi
fi

if [ -z "$3" ]; then
  duplicate=1
fi

if [ -z "$4" ]; then
  format=fq
fi

if [ -z "$5" ]; then
  compress="no"
fi

if [ -z "$6" ]; then
  disk=100
fi

if [ -z "$7" ]; then
  memory=10
fi

if [ -z "$8" ]; then
  nthread=1
fi

if [ -z "$9" ]; then
  version=7
fi

echo "*** START ***"
echo "*** Simulation Setup ***"
echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Duplicates:              $duplicate"
echo "Format of sequence file: $format"
echo "Compressed:              $compress"
echo "Disk:                    $disk (MB)"
echo "Memory:                  $memory (MB)"
echo "Number of threads:       $nthread"
echo "Version:                 $version"

# Functions

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

###############################################################################
# Sequence Generation
###############################################################################
touch START
echo "simulating a kmer count data ..."

command="./alder-kmer simulate -k $kmersize --seqlen=500 --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer --outfile=outfile"
run_command
if [ $? -ne 0 ]; then
  echo "Multiple occurences of some kmer! Try to increase the kmer size >$kmersize!"
  exit
fi

seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "bumping up ..."
multiply_file outfile-00.$format outfile.$format $duplicate

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size after x65535: $((seqfilesize / 10**6)) MB."

if [ "$compress" == "gz" ]; then
  echo "creating a compressed file using gzip... "
  command="gzip outfile.$format"
  run_command
  echo "done!"
  zip=.gz
elif [ "$compress" == "bz2" ]; then
  echo "creating a compressed file using bzip2... "
  command="bzip2 outfile.$format"
  run_command
  echo "done!"
  zip=.bz2
else
  zip=""
fi
echo "  created file: outfile.$format$zip"

###############################################################################
# Check the number of kmers in the sequence file.
###############################################################################

###############################################################################
# Sequence to kmers
###############################################################################
measure_start_time
echo "creating a binary file... "
command="./alder-kmer list -k $kmersize --select-version=$version outfile.$format$zip --outfile=outfile"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-list-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile.lst"
measure_end_time
cut -f 1 outfile.lst|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.lst.1

###############################################################################
# Sequence to Binary
###############################################################################
measure_start_time
echo "creating a binary file... "
command="./alder-kmer binary --select-version=$version --progress outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-binary-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile.bin"
measure_end_time

###############################################################################
# Binary to Sequence
###############################################################################
measure_start_time
echo "creating a sequence file... "
command="./alder-kmer uncompress outfile.bin -o outfile"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-kmer-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile.seq"
measure_end_time

echo "NR % 4 == 2' outfile.fq > outfile.seq.2"
awk 'NR % 4 == 2' outfile.fq > outfile.seq.2
echo "  created file: outfile.seq.2"

echo -n "comparing outfile.seq and outfile.seq.2 ... "
if ! diff -q outfile.seq outfile.seq.2 > /dev/null; then
  echoerr "Fail: K=$kmersize, format=$format Test failed because somethings are different."
  touch "error-binary-kmer-$kmersize"
  exit
fi

###############################################################################
# Sequence to Partition (Encoded Kmer)
###############################################################################
measure_start_time
echo "partitioning the binary file... "
command="./alder-kmer partition -k $kmersize --select-version=$version --nthread=$nthread --disk=$disk --memory=$memory outfile.bin"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-partition-$kmersize"
  exit
fi
echo "done!"
echo "  created file: outfile-0-0.par"
measure_end_time

###############################################################################
# Partition to Decoded Kmer
###############################################################################
echo "decoding outfile_0-0.par ... "
command="./alder-kmer decode -k $kmersize --select-version=$version outfile-0-0.par --outfile=outfile"
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
command="./alder-kmer table -k $kmersize --select-version=$version --nthread=$nthread --parfile=outfile"
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
# Report/Dump
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
