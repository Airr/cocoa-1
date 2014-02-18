#!/bin/bash

#  alder_kmer_test_topkmer.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 2/12/14.
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

version=8


# Check arguments.
if [ $# -lt 2 ]; then
echo "kmersize maxkmer are required arguments."
echo "alder_kmer_test_topkmer.sh kmersize maxkmer duplicate format compress disk memory nthread"
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
echo "$ ./alder_kmer_test_topkmer.sh 25 50000 6553 fq no 10000 1000 1"
echo "About ?GB"
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
duplicate=6553
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

hashtablesize=$(echo $maxkmer/0.5 | bc)

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

###############################################################################
# Sequence Generation
###############################################################################
echo "[1] creating a sequence file with errors ..."

command="./alder-kmer simulate -k $kmersize --seqlen=500 --error-rate=1 --error-initial=1 --error-iteration=$duplicate --seed=1 --format=$format --maxkmer=$maxkmer --outfile=outfile"
run_command
mv outfile-00.$format outfile.$format

#if [ $? -ne 0 ]; then
#echo "Multiple occurences of some kmer! Try to increase the kmer size >$kmersize!"
#exit
#fi

#seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
#echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."
#
#echo "  bumping up ..."
#multiply_file outfile-00.$format outfile.$format $duplicate
#
#seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
#echo "File outfile.$format size after x65535: $((seqfilesize / 10**6)) MB."

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
# Sequence to Table or (Counting...) using dsk-mode
###############################################################################
measure_start_time
echo "[2] counting kmers in the sequence data using dsk-mode ..."
command="./alder-kmer count --outfile=outfile-dsk -k $kmersize --dsk --progress --disk=$disk --memory=$memory --nthread=$nthread --log outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-dsk-$kmersize"
  exit
fi
echo "  created file: outfile-dsk.tbl"
measure_end_time

###############################################################################
# Sequence to Table or (Counting...) using topkmer-mode
###############################################################################
measure_start_time
echo "[3] counting kmers in the sequence data using topkmer-mode ..."
command="./alder-kmer count --outfile=outfile -k $kmersize --progress --nh=$hashtablesize --nthread=1 --log outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-topkmer-$kmersize"
  exit
fi
echo "  created file: outfile.tbl"
measure_end_time

###############################################################################
# Compare dsk-mode and topkmer-mode tables
###############################################################################
measure_start_time
echo "[4] comparing dsk-mode and topkmer-mode tables ..."
command="./alder-kmer assess --tabfile=outfile-dsk.tbl --outfile=outfile outfile.tbl --log outfile.$format$zip"
run_command
if [ $? -ne 0 ]; then
  echo "Crash!"
  touch "crash-assess-$kmersize"
  exit
fi
echo "  created file: outfile.err"
measure_end_time

echo "*** END ***"
echo ""

exit