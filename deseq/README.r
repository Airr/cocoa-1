R and C
-------
I tried to use the palthora of R features in C by using C functions in R and its
packages. R packages can use R memeory management or garbage collector. Using
features of R and its packages requires the garbage collector. At the first try
of seeing if I could use C functions in R and its packages, I found out that I
needed to implement R memeory management functions unless I want to modify R
packages. Modifying R packages by having my own garbage collector seemed to be
interesting, which turned out to be not the case if I want to use many R
packages. If I want to use a few R packages, I could extract parts of R
packages. Thre are so many packages that I am not directly interested in. All
of the requirement that I imposed are met by R embedded library. A year and a
half ago, I had tried to implement DESeq in a graphic user-interface program.
I could not do that. See Peach/rnaseq for an xcode project, in which you can
read a objective-C file DDVWindowController.m. The file contains some code that
I wrote to implement a similar thing that I am considering. So, it did not work
before for some reasons. After much of time I spent figuring out how I could
implement this, and now have found out that I tried it before.

Goals
-----
I need to implement three components to make this work: a command line program
of DESeq using R as a library, a command line program of short read alignment,
and a graphical user interface program using Xcode. Each requires different
software programming techniques. 

Matrix operators
----------------
http://personality-project.org/r/sem.appendix.1.pdf
http://www.statmethods.net/advstats/matrix.html

%*% -> matrix multiplication

R and C/C++
-----------
http://dirk.eddelbuettel.com/code/rcpp.html
http://dirk.eddelbuettel.com/code/rcpp.armadillo.html

GLM in C
--------
http://apophenia.info
http://stackoverflow.com/questions/6173701/how-to-implement-r-model-in-c-code

GLM in C++
-----------
http://www.alglib.net/dataanalysis/linearregression.php

R packaging
-----------
http://www.stat.ufl.edu/system/r-pkg-tut/

Compiling C code in src directory in R package
----------------------------------------------
R CMD SHLIB *.c

R-ext headers
-------------
R.h
R_ext/
Rconfig.h
Rdefines.h
Rembedded.h
Rinterface.h
Rinternals.h
Rmath.h
Rversion.h
S.h
libintl.h

Compiling R math library
------------------------
R-3.0.1/b/src/nmath/standalone
$ make
