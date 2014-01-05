#!/bin/bash

#  alder_kmer_test_large_count.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013 Sang Chul Choi. All rights reserved.

# commmand kmersize maxkmer

kmersize=$1
maxkmer=$2
format=$3
nthread=$4
duplicate=$5

# Check arguments.
if [ $# -lt 2 ]; then
echo kmersize maxkmer are required arguments.
echo alder_kmer_large.sh kmersize maxkmer format nthread duplicate
echo e.g., alder_kmer_large.sh 25 50000 fq 4 65535
echo About 9GB
exit
fi

if [ -z "$3" ]; then
format=fa
fi

if [ -z "$4" ]; then
nthread=1
fi

if [ -z "$5" ]; then
duplicate=65535
fi


hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "Kmer size:               $kmersize"
echo "Number of kmers:         $maxkmer"
echo "Format of sequence file: $format"
echo "Number of threads:       $nthread"
echo "Hash table size:         $hashtablesize"

# Functions
echoerr() { echo "$@" 1>&2; }

function multiply_file() {
  IN="${1}"
  OUT="${2}"
  N="${3}"
  for i in $(eval echo {1..$N}); do
    echo "${IN}"
  done | xargs cat > "${OUT}"
}


# Main
echo "Start the simulation!"
touch START
echo "simulating a kmer count data ..."
./alder-kmer simulate -k $kmersize --seed=1 --format=$format --maxkmer=$maxkmer
if [ $? -ne 0 ]; then
  echo Multiple occurences of some kmer! Increase kmer size!
  exit
fi
# exit
# echo "created a $format sequence file!"
# ./alder-kmer partition -k $kmersize outfile-00.$format
# if ! diff -q outfile-0-0.par outfile_0-0.par > /dev/null; then
#   echoerr "Test failed because the expected value and the observed are different."
#   cp outfile-0-0.par outfile-0-0.par.$kmersize-$format
#   cp outfile_0-0.par outfile_0-0.par.$kmersize-$format
#   cp outfile-00.$format outfile-00.$format.$kmersize-$format
#   exit
# fi
# echo "decoding ..."
# ./alder-kmer decode -k $kmersize outfile_0-0.par
# cut -f 1 outfile.dec|sort|uniq -c|awk 'BEGIN{OFS=FS=" "} {tmp=$1;$1=$2;$2=tmp;print}' > outfile.2
# single=$(cut -d" " -f 2 outfile.2 |sort|uniq)
# if [ "$single" == "1" ]; then
# echo "checked single occurence for each kmer!"
# else
# echo "Test failed because of multiple occurences for some kmers."
# echo "Suggestion: increase maxkmer argument."
# exit
# fi

seqfilesize=$(stat outfile-00.$format | cut -d" " -f 8)
echo "File outfile-00.$format size: $((seqfilesize / 10**3)) KB."

echo "bumping up ..."
multiply_file outfile-00.$format outfile.$format $duplicate

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size after x65535: $((seqfilesize / 10**6)) MB."

echo "counting ..."
#countcommand="./alder-kmer count -k $kmersize --nthread=$nthread outfile.$format --disk=60000 --memory=2000 --log --loglevel=1 --progress"
countcommand="./alder-kmer count -k $kmersize --nthread=$nthread outfile.$format --disk=60000 --memory=2000 --progress"
echo $countcommand
$countcommand

# echo "Stopped the simulation."
# touch FINISH
# exit

echo "packing ..."
./alder-kmer pack outfile.tbl
#./alder-kmer report outfile.tb2 --summary | tail
#./alder-kmer report outfile.tb2|cut -f1,3|sort -k 1|expand -t 1 > outfile.1
echo "reporting ..."
./alder-kmer report outfile.tb2 > outfile.1

# Check number of kmers
echo -n "checking number of kmers... "
lines=$(wc -l < outfile.1)
if [ $lines -ne $maxkmer ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
fi
echo "Passed!"

# Check occurences
#if [ $(cut -d" " -f 2 outfile.1|sort|uniq) != "$duplicate" ]; then
echo -n "checking the count of kmers... "
if [ $(cut -f 3 outfile.1|uniq) != "$duplicate" ]; then
  echoerr "K=$kmersize, format=$format Test failed because are different."
  exit
fi
echo "Passed!"

seqfilesize=$(stat outfile.$format | cut -d" " -f 8)
echo "File outfile.$format size: $((seqfilesize / 10**6)) MB."

echo "Finished the simulation!"
touch FINISH

#rm *.tbl
#rm *.tb2
#rm *.par
#rm START
