#!/bin/sh

#  alder_kmer_test_large_seqfile.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013 Sang Chul Choi. All rights reserved.

# Total occurence of each Kmer must be 4369 x 15.

# commmand kmersize maxkmer

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
  echoerr "Test failed because the expected value and the observed are different."
  cp outfile-0-0.par outfile-0-0.par.$kmersize-$format
  cp outfile_0-0.par outfile_0-0.par.$kmersize-$format
  cp outfile-00.$format outfile-00.$format.$kmersize-$format
exit
fi
echo "decoding ..."
./alder-kmer decode -k $kmersize outfile_0-0.par
cut -f 1 outfile.dec|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.2
single=$(cut -d" " -f 2 outfile.2 |sort|uniq)
if [ "$single" == "1" ]; then
  echo "checked single occurence for each kmer!"
else
  echo "Test failed because of multiple occurences for some kmers."
  echo "Suggestion: increase maxkmer argument."
  exit
fi

echo "bumping up ..."
for i in {1..65535}; do cat outfile-00.$format >> 1; done; mv 1 outfile-00.$format

echo "partitioning ..."
./alder-kmer partition -k $kmersize outfile-00.$format

echo "making a table ..."
./alder-kmer table -k $kmersize --nthread=$nthread --nh=$hashtablesize outfile-0-0.par --progress
./alder-kmer report outfile.tbl|cut -f1,3|sort -k 1|expand -t 1 > outfile.1

# Check number of kmers
lines=$(wc -l < outfile.1)
if [ $lines -ne $maxkmer ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
fi

# Check occurences
if [ $(cut -d" " -f 2 outfile.1|sort|uniq) != "65535" ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
  exit
fi

parfilesize=$(stat outfile-0-0.par | cut -d" " -f 8)
echo File outfile-0-0.par size: $((parfilesize / 10**6)) MB.
