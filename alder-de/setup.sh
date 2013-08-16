#!/bin/sh

#  setup.sh
#  alder-de
#
#  Created by Sang Chul Choi on 8/7/13.
#  Copyright (c) 2013 Sang Chul Choi. All rights reserved.

################################################################################
#  Edit the following variables, and edit the last several lines of this file to
#  select which function you want to use. After the edit, run this script using
#  your SHELL such as bash (e.g., $ bash setup.sh).
#
#  We need to know where you want to save R libraries.
RSETUPDIR=$HOME/Downloads
#  We need to know what R version you use.
RVERSION=R-3.0.1
#  We need to know what gfortran command you installed. I could install gfortran
#  using either brew or mac port. See 1.setup.required
#GFORTRAN=gfortran-mp-4.7 with mac port
#GFORTRAN=gfortran # with brew

#  Do not edit these convenient variables.
#  Convenient variables.
SOURCEDIR=${PWD}
RSOURCE=${RSETUPDIR}/${RVERSION}
RTARGET=${RSOURCE}/b
RBIN=${RTARGET}/bin

function 1.setup.required {
echo Install gfortran using either brew or mac port
echo Use command lines: brew install gfortran
echo Use this one: sudo port install gcc47
}

function 2.setup.r {
cd $RSETUPDIR
# wget http://cran.fhcrc.org/src/base/R-3/${RVERSION}.tar.gz
tar zxf ${RVERSION}.tar.gz
}

function 2.1.setup.r {
mkdir -p ${RSETUPDIR}/R/share
cat>${RSETUPDIR}/R/share/config.site<<EOF
CC="llvm-gcc-4.2"
CXX="llvm-g++-4.2"
F77="gfortran-4.2 -arch x86_64"
FC=\$F77
OBJC="clang"
EOF
mkdir ${RTARGET}
cd ${RTARGET}
../configure --prefix=${RSETUPDIR}/R \
  --enable-R-shlib \
  --with-system-xz=no \
  --without-readline \
  --disable-R-framework \
  --disable-openmp \
  --without-tcltk \
  --disable-R-profiling \
  --disable-nls \
  --with-blas="-framework Accelerate" --with-lapack
R_PAPERSIZE=letter make
}

function 3.install.r.package {
${RSETUPDIR}/${RVERSION}/b/bin/Rscript ${SOURCEDIR}/install-package.R
${RSETUPDIR}/${RVERSION}/b/bin/Rscript ${SOURCEDIR}/install-package.R
}

function 4.copy.r.library {
cd ${RTARGET}
make install
#mkdir ${RSETUPDIR}/R
#cp -r ${RSETUPDIR}/${RVERSION}/b/library ${RSETUPDIR}/R
#cp -r ${RSETUPDIR}/${RVERSION}/b/lib ${RSETUPDIR}/R
#cp -r ${RSETUPDIR}/${RVERSION}/b/include ${RSETUPDIR}/R
#cp -r ${RSETUPDIR}/${RVERSION}/b/share ${RSETUPDIR}/R
#cp -r ${RSETUPDIR}/${RVERSION}/b/doc ${RSETUPDIR}/R
}

function 4.5.install.r.library {

install_name_tool -id ${RSETUPDIR}/R/lib/R/lib/libR.dylib \
  ${RSETUPDIR}/R/lib/R/lib/libR.dylib
for i in `find ${RSETUPDIR}/R/lib -name *.so`; do
  install_name_tool -change libR.dylib ${RSETUPDIR}/R/lib/R/lib/libR.dylib $i;
done
}

function 5.show-compile-options {
echo Use the following CFLAGS and LDFLAGS.
${RBIN}/R CMD config --cppflags
${RBIN}/R CMD config --ldflags
}

function 6.show-diff-environment {
env > 1
echo Comment the last command line in the Rcmd.
echo ${RBIN}/R/Rcmd to add a line \"env \> 2\" just before the last command line.
echo Use: diff --changed-group-format='%<' --unchanged-group-format='' 2 1
echo to find the added environment variables
echo Revert the Rcmd back to the original version.
echo Change each line of the diff output:
echo e.g, LN_S=ln -s to putenv\(\"LN_S=ln -s\"\);
echo create renv.c and renv.h with a function called renv.
}
# R_INCLUDE_DIR

# 1.setup.required
#2.setup.r
#2.1.setup.r
#3.install.r.package
#4.copy.r.library
4.5.install.r.library
#5.show-compile-options
#6.show-diff-environment

