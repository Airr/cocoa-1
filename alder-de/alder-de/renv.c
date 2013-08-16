//
//  renv.c
//  alder-de
//
//  Created by Sang Chul Choi on 8/7/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdlib.h>

void renv(void)
{
    putenv("LN_S=ln -s");
    putenv("R_TEXI2DVICMD=/usr/bin/texi2dvi");
    putenv("BIBINPUTS=.:/Users/goshng/Downloads/R/lib/R/share/texmf/bibtex/bib:");
    putenv("SED=/usr/bin/sed");
    putenv("R_PDFVIEWER=/usr/bin/open");
    putenv("R_INCLUDE_DIR=/Users/goshng/Downloads/R/lib/R/include");
    putenv("BSTINPUTS=.:/Users/goshng/Downloads/R/lib/R/share/texmf/bibtex/bst:");
    putenv("DYLD_FALLBACK_LIBRARY_PATH=/Users/goshng/Downloads/R/lib/R/lib");
    putenv("R_PRINTCMD=lpr");
    putenv("R_VERSION=3.0.1");
    putenv("R_SYSTEM_ABI=osx,gcc,gxx,gfortran,?");
    putenv("R_RD4PDF=times,inconsolata,hyper");
    putenv("R_PAPERSIZE=letter");
    putenv("TAR=/usr/bin/gnutar");
    putenv("R_ZIPCMD=/usr/bin/zip");
    putenv("PAGER=/usr/bin/less");
    putenv("R_GZIPCMD=/usr/bin/gzip");
    putenv("R_SHARE_DIR=/Users/goshng/Downloads/R/lib/R/share");
    putenv("EDITOR=vi");
    putenv("R_OSTYPE=unix");
    putenv("R_BROWSER=/usr/bin/open");
    putenv("R_CMD=/Users/goshng/Downloads/R/lib/R/bin/Rcmd");
    putenv("TEXINPUTS=.:/Users/goshng/Downloads/R/lib/R/share/texmf/tex/latex:");
    putenv("SHLVL=2");
    putenv("R_ARCH=");
    putenv("MAKE=make");
    putenv("R_PAPERSIZE_USER=");
    putenv("R_UNZIPCMD=/usr/bin/unzip");
    putenv("R_BZIPCMD=/usr/bin/bzip2");
    putenv("R_HOME=/Users/goshng/Downloads/R/lib/R");
    putenv("R_PLATFORM=x86_64-apple-darwin12.4.0");
    putenv("R_LIBS_USER=~/R/x86_64-apple-darwin12.4.0-library/3.0");
    putenv("R_DOC_DIR=/Users/goshng/Downloads/R/lib/R/doc");
}
