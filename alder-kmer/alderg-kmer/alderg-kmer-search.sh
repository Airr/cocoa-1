#!/bin/bash

TABLE=$2
DNA=$3
$1/alder-kmer report \
$TABLE \
--query $DNA

