#!/bin/bash

#  alder_kmer_test_table3.sh
#  alder-kmer
#
#  Created by Sang Chul Choi on 12/30/13.
#  Copyright (c) 2013,2014 Sang Chul Choi. All rights reserved.

# Total occurence of each Kmer must be 4369 x 15.

###############################################################################
# Command arguments
###############################################################################

duplicate=65535
#duplicate1=10
#duplicate2=1
duplicate1=4369
duplicate2=15

kmersize=$1
maxkmer=$2
format=$3
compress=$4
disk=$5
memory=$6
nthread=$7
version=$8
skip=$9

# Check arguments.
if [ $# -lt 2 ]; then
echo "kmersize maxkmer are required arguments."
echo "alder_kmer_table2.sh kmersize maxkmer duplicate format compress disk memory nthread version run"
echo "kmersize  : number between 1-64"
echo "maxkmer   : number (1000-)"
echo "format    : fq or fa (default:fa)"
echo "compress  : [no|gz|bz2] (default:no)"
echo "disk      : number (default:1000)"
echo "memory    : number (default:100)"
echo "nthread   : number (default:4)"
echo "version   : 1,2,3 (default:3)"
echo "run       : no (generate data) or yes (run on data) (default:no)"
echo "e.g.,"
echo "$ ./alder_kmer_test_table3.sh 25 50000 fq no 10000 1000 4 3 yes"
echo "$ ./alder_kmer_test_table3.sh 25 50000 fq no 10000 1000 4 3 no"
echo "About 9GB"
exit
fi

if [ -z "$3" ]; then
  format=fa
fi

if [ -z "$4" ]; then
  compress=""
fi

if [ -z "$5" ]; then
  disk=1000
fi

if [ -z "$6" ]; then
  memory=100
fi

if [ -z "$7" ]; then
  nthread=4
fi

if [ -z "$8" ]; then
  version=3
fi

if [ -z "$9" ]; then
  skip="no"
fi

###############################################################################
# Kmer -> Byte
###############################################################################
x=$((kmersize/31))
y=$((kmersize%31))
if [ "$y" == 0 ]; then
  x=$(echo $x - 1 | bc)
  y=31
fi
if [ "$kmersize" == 1 ]; then
  x=0
fi
#echo "$kmersize $x $y - $kmerbyte"

kmersize1=$(echo "$kmersize - 31 * $x" | bc)
kmerbyte=$(echo $kmersize1/4 | bc)
testkmersize=$((kmersize1%4))
if [ "$testkmersize" != 0 ]; then
  kmerbyte=$(echo $kmerbyte + 1| bc)
fi
kmerbyte=$(echo "$kmerbyte + 8 * $x" | bc)
#echo "KB:$kmersize - $kmerbyte"


hashtablesize=$(echo $maxkmer/0.7 | bc)

echo "*** START ***"
echo "*** Simulation Setup ***"
echo "Kmer size:               $kmersize"
echo "Kmer byte:               $kmerbyte"
echo "Number of kmers:         $maxkmer"
echo "Duplicates:              $duplicate"
echo "Format of sequence file: $format"
echo "Compressed:              $compress"
echo "Disk:                    $disk (MB)"
echo "Memory:                  $memory (MB)"
echo "Number of threads:       $nthread"
echo "Version:                 $version"
echo "Skip:                    $skip"
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

if [ "$skip" == "no" ]; then
echo "simulating a kmer count data ..."
command="./alder-kmer simulate -k $kmersize --select-version=$version --seed=1 --format=$format --maxkmer=$maxkmer --seqlen=1000"
run_command
if [ $? -ne 0 ]; then
  echo "Multiple occurences of some kmer! Try to increase the kmer size >$kmersize!"
  touch "crash-simulate-$kmersize"
  exit
fi

parfilesize=$(stat outfile_0-0.par | cut -d" " -f 8)
echo "File outfile_0-0.par size: $((parfilesize / 10**3)) KB."

echo "bumping up par file ..."
multiply_file outfile_0-0.par outfile-0-0.par $duplicate1

parfilesize=$(stat outfile-0-0.par | cut -d" " -f 8)
echo "File outfile-0-0.par size after x$duplicate1: $((parfilesize / 10**6)) MB."

echo "shuffling ... x$duplicate2"
measure_start_time
rm -f 2.par
for i in {1..15}; do
  xxd -p -c $kmerbyte outfile-0-0.par | gshuf - | xxd -r -p - 1.par
  cat 1.par >> 2.par
done
rm 1.par
mv 2.par outfile.par
echo "  created file: outfile.par"

parfilesize=$(stat outfile.par | cut -d" " -f 8)
echo "File outfile.par size after x$duplicate1, s$duplicate2: $((parfilesize / 10**6)) MB."
measure_end_time
mv outfile.par outfile-$kmersize.pas
exit

else

echo "skipping data generation"

fi

###############################################################################
# Partition to Table
###############################################################################
measure_start_time
echo "creating kmers using table command ... "
command="./alder-kmer table -k $kmersize --select-version=$version --progress --nh=$hashtablesize --disk=$disk --memory=$memory --nthread=$nthread outfile-$kmersize.pas"
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



