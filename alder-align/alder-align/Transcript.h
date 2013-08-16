//
//  Transcript.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_Transcript_h
#define alder_align_Transcript_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_const.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

struct alder_transcript_struct
{
    uint64_t exons[MAX_N_EXONS][EX_SIZE]; //coordinates of all exons: r-start, g-start, length
    uint64_t shiftSJ[MAX_N_EXONS][2]; //shift of the SJ coordinates due to genomic micro-repeats
    int canonSJ[MAX_N_EXONS]; //canonicity of each junction
    uint8_t sjAnnot[MAX_N_EXONS]; //anotated or not
    uint8_t sjStr[MAX_N_EXONS]; //anotated or not
    
    uint64_t intronMotifs[3];
    uint8_t sjMotifStrand;
    
    uint64_t nExons; //number of exons in the read transcript
    
    uint64_t Lread, readLengthPairOriginal;
    uint64_t iRead; //read identifier
    
    int iFrag; //frag number of the transcript, if the the transcript contains only one frag
    
    //loci
    uint64_t rStart, roStart, rLength, gStart, gLength, cStart; //read, original read, and genomic start/length, chromosome start
    uint64_t Chr,Str,roStr; //chromosome and strand and original read Strand
    
    bool primaryFlag;
    
    //         uint64_t polyXlength[4], polyXnMM[4] ;
    uint64_t nWAmax; //number of genes, max number of alignments per gene, total number of alignments for this reads
    
    uint64_t nMatch;//min number of matches
    uint64_t nMM;//max number of mismatches
    uint64_t mappedLength; //total mapped length, sum of lengths of all blocks(exons)
    
    uint64_t extendL; //extension length
    intScore maxScore; //maximum Score
    intScore nextTrScore; //next after maximum Tr Score
    
    
    uint64_t nGap, lGap; //number of genomic gaps (>alignIntronMin) and their total length
    uint64_t nDel; //number of genomic deletions (ie genomic gaps)
    uint64_t nIns; //number of (ie read gaps)
    uint64_t lDel; //total genomic deletion length
    uint64_t lIns; //total genomic insertion length
    
    uint64_t nUnique, nAnchor; //number of unique pieces in the alignment, number of anchor pieces in the alignment
    
};

typedef struct alder_transcript_struct alder_transcript;

void alder_transcript_reset (alder_transcript*);
void alder_transcript_add (alder_transcript * t1, const alder_transcript * t2);

__END_DECLS

#endif
