#!/bin/bash

#  alder_kmer_test_count.sh
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
echo "alder_kmer_count.sh kmersize maxkmer duplicate format compress disk memory nthread version"
echo "kmersize  : number between 1-64"
echo "maxkmer   : number (1000-)"
echo "duplicate : number (1-)"
echo "format    : fq or fa"
echo "compress  : [no|gz|bz2]"
echo "disk      : number (default:1000MB)"
echo "memory    : number (default:100MB)"
echo "nthread   : number (default:1)"
echo "version   : 1,2,3,4,5,7 (default:7)"
echo "e.g.,"
echo "$ ./alder_kmer_test_count.sh 25 50000 65535 fq no 10000 1000 1 7"
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
  duplicate=65535
fi

if [ -z "$4" ]; then
  format=fq
fi

if [ -z "$5" ]; then
  compress=""
fi

if [ -z "$6" ]; then
  disk=10000
fi

if [ -z "$7" ]; then
  memory=1000
fi

if [ -z "$8" ]; then
  nthread=1
fi

if [ -z "$9" ]; then
  version=7
fi


# hashtablesize=$(echo $maxkmer/0.7 | bc)

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
# echo "Hash table size:         $hashtablesize"

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
echo "[1] creating a sequence file ..."

command="./alder-kmer simulate -k $kmersize --seqlen=500 --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer --outfile=outfile"
run_command
if [ $? -ne 0 ]; then
  echo "Multiple occurences of some kmer! Try to increase the kmer size >$kmersize!"
  exit
fi

seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "  bumping up ..."
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
# Sequence to Table or (Counting...)
###############################################################################
measure_start_time
echo "[2] counting kmers in the sequence data ..."
command="./alder-kmer count -k $kmersize --select-version=$version --progress --disk=$disk --memory=$memory --nthread=$nthread --log outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-count-$kmersize"
  exit
fi
echo "  created file: outfile.tbl"
measure_end_time

###############################################################################
# Table to Report
###############################################################################
echo "[3] reporting the resulting table ..."
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
echo -n "[4] checking number of kmers ........................................"
lines=$(wc -l < outfile.1)
if [ $lines -ne $maxkmer ]; then
  echoerr "K=$kmersize, format=$format Test failed because maxkmer is not $maxkmer."
  touch "error1-$kmersize"
  exit
fi
echo "Passed!"

###############################################################################
# Check occurences
###############################################################################
echo -n "[5] checking the count of kmers ....................................."
nuniquekmer=$(cut -f 3 outfile.1|uniq|wc -l)
if [ $nuniquekmer -ne 1 ]; then
  echoerr "Multiple kmers with different counts."
  touch "error2-$kmersize"
  exit
fi
if [ $(cut -f 3 outfile.1|uniq) != "$duplicate" ]; then
  echoerr "K=$kmersize, format=$format Test failed because $duplicate duplicates are different."
  touch "error3-$kmersize"
  exit
fi
echo "Passed!"

touch FINISH
echo "*** END ***"
echo ""

exit






###############################################################################
# Compare the sequence data and table file
###############################################################################

echo "comparing sequence data and table file... "
if [ "$format" == "fq" ]; then
measure_start_time
echo "./alder-kmer list outfile.fq -k $kmersize | ./alder-kmer match --tabfile=outfile.tbl"
./alder-kmer list outfile.fq -k $kmersize | ./alder-kmer match --tabfile=outfile.tbl
#./alder-kmer match --tabfile=outfile.tbl outfile.lst
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-match-$kmersize"
  exit
fi
echo "Passed!"
measure_end_time
fi
