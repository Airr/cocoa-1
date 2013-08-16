//
//  Transcript.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "Transcript.h"

void alder_transcript_reset (alder_transcript * t1)
{
    t1->extendL=0;
    t1->primaryFlag = false;
    t1->nWAmax=0;
    t1->rStart=0;
    t1->roStart=0;
    t1->rLength=0;
    t1->gStart=0;
    t1->gLength=0;
    t1->maxScore=0;
    t1->nextTrScore=0;
    t1->nMatch=0;
    t1->nMM=0;
    t1->nGap=0;
    t1->lGap=0;
    t1->lDel=0;
    t1->lIns=0;
    t1->nDel=0;
    t1->nIns=0;
    t1->nUnique= 0;
    t1->nAnchor=0;
}

void alder_transcript_add (alder_transcript * t1, const alder_transcript * t2)
{
    t1->maxScore += t2->maxScore;
    t1->nMatch   += t2->nMatch;
    t1->nMM      += t2->nMM;
    t1->nGap     += t2->nGap;
    t1->lGap     += t2->lGap;
    t1->lDel     += t2->lDel;
    t1->nDel     += t2->nDel;
    t1->lIns     += t2->lIns;
    t1->nIns     += t2->nIns;
    t1->nUnique  += t2->nUnique;
}