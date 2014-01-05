#!/bin/sh

#  alder_kmer_large.sh
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

kmerbyte=$(echo $kmersize/4 | bc)
testkmersize=$((kmersize%4))
if [ "$testkmersize" != 0 ]; then
  echo kmersize must be multiple of 4.
  exit
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
for i in {1..4369}; do cat outfile_0-0.par >> 1; done; mv 1 outfile_0-0.par

# 4369 x 15

if [ -e 2.par ]; then
  rm 2.par
fi

echo shuffling ...
for i in {1..15}; do
xxd -p -c $kmerbyte outfile_0-0.par | gshuf - | xxd -r -p - 1.par; cat 1.par >> 2.par
done
rm 1.par
mv 2.par outfile_0-0.par

echo creating outfile.2 with updated count values ...
sed 's/1/65535/' < outfile.2 > 1; mv 1 outfile.2
# ./alder-kmer decode -k $kmersize outfile_0-0.par --progress
# cut -f 1 outfile.dec|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.2

echo making a table ...
./alder-kmer table -k $kmersize --nthread=1 --nh=$hashtablesize outfile_0-0.par --progress
./alder-kmer report outfile.tbl|cut -f1,3|sort -k 1|expand -t 1 > outfile.1
if ! diff -q outfile.1 outfile.2 > /dev/null; then
  echoerr "K=$kmersize Test failed because somethings are different."
  exit
fi

parfilesize=$(stat outfile_0-0.par | cut -d" " -f 8)
echo File outfile_0-0.par size: $((parfilesize / 10**6)) MB.
