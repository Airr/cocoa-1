//
//  alder_index_genome.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <nglogc/log.h>
#include <gsl/gsl_errno.h>
#include "alder_index_genome.h"
#include "alder_logger.h"
#include "libdivsufsort/suftest.h"
#include "libdivsufsort/mksary.h"
#include "libdivsufsort/sasearch.h"


int alder_index_genome (Parameters *p)
{
    int status = GSL_SUCCESS;
    
    logc_log(MAIN_LOGGER, LOG_BASIC, "begins to index a genome");
    assert(p->genomeChrBinNbits == 18LLU);
    logc_log(MAIN_LOGGER, LOG_BASIC, "creates %s/genomeParameters.txt",
             p->genomeDir);
    // nChrReal          : the number of real chromosomes
    // NbasesChrReal     : is number of bases plus some "padded" fictitous bases.
    // genomeChrBinNbases: chromosomes are packed to bins.
    // chrName           : chromosome name in the fasta file.
    // chrStart          : 0-based position of the "faked or padded" chromosome start.
    // chrLength         : real chromosome length
    // chrBin            : bin ID for all of the chromosomes
    logc_log(MAIN_LOGGER, LOG_BASIC, "reads the FASTA files for chromosome");
    // G1 is set to 5. Why 5? A,C,G,T, and N. 5 is the last position past the N.
    // G = G1 + L(=10000)
    // G contains padded bases for chromosomes.
    // G contains numbers (0,1,2,3,4 and 5).
    logc_log(MAIN_LOGGER, LOG_BASIC, "ignores splice junctions database");
    // nGenome : is the number of padded bases, plus splicing junction stuff.
    // chrStart.txt     : "padded" chromosome starts and the last end.
    // chrLength.txt    : real chromosome length
    // chrNameLength.txt: chromosome names and the real lengths
    // chrName.txt      : chromosome names
    // Genome           : file of G and splicing junction.
    logc_log(MAIN_LOGGER, LOG_BASIC, "Writing genome to disk...done.");
    // limitGenomeGenerateRAM: how to find the limit.
    logc_log(MAIN_LOGGER, LOG_BASIC, "Create G for reverse strand, see code");
    logc_log(MAIN_LOGGER, LOG_BASIC, "nSA is the number of bases.");
    logc_log(MAIN_LOGGER, LOG_BASIC, "Number of SA indices: P->nSA");
    logc_log(MAIN_LOGGER, LOG_BASIC, "SA size in bytes: << P->nSAbyte");
    logc_log(MAIN_LOGGER, LOG_BASIC, "start to sort  Suffix Array.");
    logc_log(MAIN_LOGGER, LOG_BASIC, "??? saChunkN number of chunks, ");
    logc_log(MAIN_LOGGER, LOG_BASIC, "??? saChunkSize*8 chuck size");
    logc_log(MAIN_LOGGER, LOG_BASIC, "Finished generating suffix array");
    logc_log(MAIN_LOGGER, LOG_BASIC, "starting to generate Suffix Array index...");
    logc_log(MAIN_LOGGER, LOG_BASIC, "writing SAindex to disk");
    // SA, and SAindex : two files of SA 
    logc_log(MAIN_LOGGER, LOG_BASIC, "Find a way of indexing the genome");
    
    
//    int argc = 2;
//    const char *argv[2] = {"alder-align", "data/test.fa"};
//    suftest(argc, argv);
//    int argc = 3;
//    const char *argv[3] = {"alder-align", "data/test.fa", "data/test.sa"};
//    mksary(argc, argv);
//    int argc = 4;
//    const char *argv[4] = {"alder-align", "TCCGAATTGAACCGCGCTAT", "data/test.fa", "data/test.sa"};
//    sasearch(argc, argv);
    
    return status;
}
