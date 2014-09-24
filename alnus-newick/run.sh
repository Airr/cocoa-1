#!/bin/sh

#  run.sh
#  alnus-newick
#
#  Created by Sang Chul Choi on 9/15/14.
#  Copyright (c) 2014 Sang Chul Choi. All rights reserved.

clang++ --std=c++11 main.cpp -o alnus-newick
./alnus-newick -t > file.tre
./alnus-newick file.tre
./alnus-newick -g 10 > random.tre
./alnus-newick -o random.out random.tre > random.out2
diff random.out random.tre
./alnus-newick -g 5 -p 2 -e 1.0 -l 2.0 > random2.tre
./alnus-newick