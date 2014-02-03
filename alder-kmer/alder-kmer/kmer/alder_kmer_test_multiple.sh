#!/bin/bash

#  alder_kmer_test_simple.sh
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
disk=$6
memory=$7
nthread=$8
version=$9

# Check arguments.
if [ $# -lt 2 ]; then
echo "kmersize maxkmer are required arguments."
echo "alder_kmer_multiple.sh kmersize maxkmer duplicate format compress disk memory nthread version"
echo "kmersize  : number between 1-64"
echo "maxkmer   : number (1000-)"
echo "duplicate : number (1-65535)"
echo "format    : fq or fa"
echo "compress  : [no|gz|bz2]"
echo "disk      : number (default: 100 MB)"
echo "memory    : number (default: 10 MB)"
echo "nthread   : number (default: 1)"
echo "version   : 1,2,3,4,5,7 (default:7)"
echo "e.g.,"
echo "$ ./alder_kmer_test_count.sh 25 50000 1 fq no 10000 1000 1 7"
echo "About 9GB"
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

###############################################################################
# Sequence Generation
###############################################################################
touch START
echo "simulating a kmer count data ..."
command="./alder-kmer simulate -k $kmersize --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer"
run_command
if [ $? -ne 0 ]; then
  echo "Multiple occurences of some kmer! You could increase the kmer size >$kmersize!"
fi

seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "bumping up ..."
multiply_file outfile-00.$format outfile.$format $duplicate

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size after x$duplicate: $((seqfilesize / 10**6)) MB."

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
  touch "crash-binary-uncompress-$kmersize"
  exit
fi


###############################################################################
# Binary to Partition (Encoded Kmer)
###############################################################################
measure_start_time
echo "partitioning the sequence file... "
command="./alder-kmer partition -k $kmersize --select-version=$version --progress --disk=$disk --memory=$memory --log --nthread=$nthread outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-binary-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile-0-0.par"
measure_end_time

###############################################################################
# Check kmers in the partition
###############################################################################
measure_start_time
echo "partitioning the sequence file... "
command="./alder-kmer kmer -k $kmersize *.par -o outfile --select-version=$version"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-partition-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile-0-0.par"
measure_end_time

###############################################################################
# Partition to Table
###############################################################################
measure_start_time
echo "creating kmers using table command ... "
command="./alder-kmer table -k $kmersize --select-version=$version --progress --nh=$hashtablesize --disk=$disk --memory=$memory --nthread=$nthread outfile-0-0.par"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-table-$kmersize"
  exit
fi
echo "  done!"
echo "  created file: outfile.tbl"
measure_end_time

###############################################################################
# Table to Report
###############################################################################
echo "reporting the table ..."
echo "./alder-kmer report outfile.tbl > outfile.1"
./alder-kmer report outfile.tbl > outfile.1
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-report-$kmersize"
  exit
fi

###############################################################################
# Check number of kmers
###############################################################################
echo -n "checking number of kmers... "
lines=$(wc -l < outfile.1)
if [ $lines -ne $maxkmer ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
  touch "error1-$kmersize"
  exit
fi
echo "Passed!"

###############################################################################
# Check occurences
###############################################################################
echo -n "checking the count of kmers... "
nuniquekmer=$(cut -f 3 outfile.1|uniq|wc -l)
if [ $nuniquekmer -ne 1 ]; then
  echoerr "Multiple kmers with different counts."
  touch "error2-$kmersize"
  exit
fi
if [ $(cut -f 3 outfile.1|uniq) != "$duplicate" ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
  touch "error3-$kmersize"
  exit
fi
echo "Passed!"

touch FINISH

#rm *.tbl
#rm *.tb2
#rm *.par
#rm START

echo "*** END ***"
echo ""
