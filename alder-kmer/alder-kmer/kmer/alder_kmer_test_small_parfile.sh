#!/bin/sh

#  alder_kmer_small.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013 Sang Chul Choi. All rights reserved.

# commmand kmersize maxkmer format nthread
# ./alder_kmer_small.sh 1 10 fq 1

kmersize=$1
maxkmer=$2
format=$3
nthread=$4

# Check arguments.
if [ $# -lt 2 ]; then
  echo kmersize maxkmer are required arguments.
  echo alder_kmer_large.sh kmersize maxkmer format nthread
  exit
fi

if [ -z "$3" ]; then
  format=fa
fi

if [ -z "$4" ]; then
  nthread=1
fi

hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Format of sequence file: $format"
echo "Number of threads:       $nthread"
echo "Hash table size:         $hashtablesize"

# Functions
echoerr() { echo "$@" 1>&2; }


# Main
echo "simulating a count data ..."
./alder-kmer simulate -k $kmersize --seed=1 --format=$format --maxkmer=$maxkmer
echo "created a $format sequence file!"
./alder-kmer partition -k $kmersize outfile-00.$format
if ! diff -q outfile-0-0.par outfile_0-0.par > /dev/null; then
  echoerr "K=$kmersize, format=$format Test failed because the expected value and the observed are different."
  cp outfile-0-0.par outfile-0-0.par.$kmersize-$format
  cp outfile_0-0.par outfile_0-0.par.$kmersize-$format
  cp outfile-00.$format outfile-00.$format.$kmersize-$format
  exit
fi
echo "decoding ..."
./alder-kmer decode -k $kmersize outfile_0-0.par
cut -f 1 outfile.dec|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.2
echo "created kmer using decode command with the partition file!"
./alder-kmer table -k $kmersize --nthread=1 --nh=$hashtablesize outfile_0-0.par
echo "counted kmer using table command!"
./alder-kmer report outfile.tbl |cut -f1,3|sort -k 1|expand -t 1 > outfile.1
if ! diff -q outfile.1 outfile.2 > /dev/null; then
  echoerr "K=$kmersize, format=$format Test failed because somethings are different."
  exit
fi
echo "checked the count!"


