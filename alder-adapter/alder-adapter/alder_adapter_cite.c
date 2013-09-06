/**
 * This file, alder_adapter_cite.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_adapter_cite.h"

static const char *alder_adapter_cite_description = "This is a simpler version of cutadapt by Marcel Martin, and prinseq by\nSchmieder and Edwards. You could use more features in their software packages,\ncutadapt and prinseq, and cite their papers, if you use this alder-adapter\nsoftware package; 1) Martin, M. Cutadapt removes adapter sequences from\nhigh-throughput sequencing reads EMBnet.journal, 2011, 17. 2) Schmieder R and\nEdwards R: Quality control and preprocessing of metagenomic datasets.\nBioinformatics 2011, 27:8 63-864. [PMID: 21278185]. See their websites\nhttp://code.google.com/p/cutadapt/ or http://prinseq.sourceforge.net for the\nsoftware packages. I used some of the feature in a copy of\ncutadapt version 1.2.1. and prinseq version 0.20.3.\n";

void alder_adapter_cite()
{
    printf("%s", alder_adapter_cite_description);
}
