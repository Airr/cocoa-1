//
//  alder_align_reads.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <nglogc/log.h>
#include "alder_logger.h"
#include "alder_align_reads.h"

int alder_align_reads (Parameters *p)
{
    logc_log(MAIN_LOGGER, LOG_BASIC, "Load the genome.");
    // nGenome:
    // winBinN:
    // winBinNbits:
    // P->winBinN = P->nGenome/(1LLU << P->winBinNbits)+1;
    // outSAMmode: Full, None
    // chimSegmentMin: minimum number of chimeric segments.
    logc_log(MAIN_LOGGER, LOG_BASIC, "");
    logc_log(MAIN_LOGGER, LOG_BASIC, "Output SAM file header.");
    logc_log(MAIN_LOGGER, LOG_BASIC, "Output chimeric SAM file header.");
    // classes -
    // ReadAlignChunk?
    // ThreadControl class?
    // Stats class?
    logc_log(MAIN_LOGGER, LOG_BASIC, "Started mapping");
    // pthread_create
    // RAchunk[0]->
    // pthread_join: to wait for all other threads.
    // outputSJ after all the work of the threads.
    // ReadAlignChunk::processChunks seems to be the main of alignment procedure.
    // ReadAlign class
    // Do something about read alignment.
    // readNmates is 2(paired) or 1(single).
    // sequence read and remove adapters.
    //
    // processChunks -> mapChunk -> oneRead -> readLoad -> mapOneRead -> maxMappableLength2strands -> stitchPieces
    //
}
