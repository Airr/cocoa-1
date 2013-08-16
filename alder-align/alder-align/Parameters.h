//
//  Parameters.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/9/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_Parameters_h
#define alder_align_Parameters_h

#include <stdint.h>
#include <stdbool.h>
#include "alder_const.h"
//#include "IncludeDefine.h"
//#include "InOutStreams.h"
//#include "ParameterInfo.h"
//#include <map>
#include <gsl/gsl_vector.h>
#include "gsl_vector_string.h"

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

struct Parameters_struct
{
//    vector <ParameterInfoBase*> parArray, parArrayInitial;
    gsl_vector_string *parameterInputName;
    
    char *commandLine;
    char *commandLineFull;
    
    //version
    uint64_t versionSTAR;
    gsl_vector_ulong *versionGenome;
    
    // run parameters
    char *runMode;
    int   runThreadN;
    
    //parameters
    gsl_vector_string *parametersFiles;
    
    //genome, SA, ...
    gsl_vector_ulong *chrStart;
    gsl_vector_ulong *chrLength;
    char *genomeDir;
    char *genomeLoad;
    gsl_vector_string *genomeFastaFiles;
    uint64_t genomeSAsparseD;//sparsity=distance between indices
    //binning,windows,anchors
    uint64_t genomeChrBinNbits;
    uint64_t genomeChrBinNbases;
    uint64_t chrBinN;
    gsl_vector_ulong *chrBin;
    uint64_t winBinChrNbits;
    uint64_t winBinNbits;
    uint64_t winAnchorDistNbins;
    uint64_t winFlankNbins;
    uint64_t winBinN;
    uint64_t winAnchorMultimapNmax; //max number of alignments for anchors
    
    uint64_t genomeSAindexNbases; //length of the SA pre-index strings
    uint64_t *genomeSAindexStart;//starts of the L-mer indices in the SAindex, 1<=L<=genomeSAindexNbases
    
    
    //read parameters

    gsl_vector_string *readFilesIn;
    gsl_vector_string *readFilesInTmp;
    uint64_t readNmates;
    char *readMatesLengthsIn;
    gsl_vector_string *readFilesCommand;
    
    gsl_vector_ulong *clip5pNbases;
    gsl_vector_ulong *clip3pNbases;
    gsl_vector_ulong *clip3pAfterAdapterNbases;
    gsl_vector *clip3pAdapterMMp; // double vector
    gsl_vector_string *clip3pAdapterSeq;
    char *clip3pAdapterSeqNum[MAX_N_MATES];//adapter sequence - numerical
    bool readMatesEqualLengths; //whether or not the read mates have the same length, true if onyl one mate
    
    //align parameters
    uint64_t alignSJoverhangMin;
    uint64_t alignSJDBoverhangMin;
    uint64_t alignSplicedMateMapLmin; //min SJ donor/acceptor length
    double alignSplicedMateMapLminOverLmate;
    uint64_t alignWindowsPerReadNmax; //max number of alignment windows per read
    uint64_t alignTranscriptsPerWindowNmax; //maximum number of transcripts recorded per window
    uint64_t alignTranscriptsPerReadNmax;   //max number of alignments per read
    uint64_t alignIntronMin;//min length to call a gap an intron
    uint64_t alignIntronMax;//max length to call
    uint64_t alignMatesGapMax;//max gap between the mates (if paired-end)
    
    //seed parameters
    uint64_t seedMultimapNmax; //max number of multiple alignments per piece
    uint64_t seedSearchLmax; //max length of the seed
    uint64_t seedPerReadNmax; //max number of pieces per Read
    uint64_t seedPerWindowNmax; //max number of aligns per window
    uint64_t seedNoneLociPerWindow; //max number of aligns from one piece per window
    uint64_t seedSearchStartLmax;
    double seedSearchStartLmaxOverLread; //length of split start points
    
    //chunk parameters
    uint64_t chunkInSizeBytes;
    uint64_t chunkInSizeBytesArray;
    uint64_t chunkOutSAMsizeBytes;
    uint64_t chunkOutSJsize;
    
    //output
    char *outFileNamePrefix;
    char *outStd;
    char *outSAMmode;
    char *outSAMstrandField;
    char *outSAMattributes;
    char *outSAMunmapped;
    char *outReadsUnmapped;
    int outQSconversionAdd;
    char *outFileTmp;
    
    //output filtering
    uint64_t outFilterMatchNmin;
    uint64_t outFilterMismatchNmax;
    uint64_t outFilterMultimapNmax;//min number of matches
    double outFilterScoreMinOverLread;
    double outFilterMatchNminOverLread;//normalzied to read length
    intScore outFilterScoreMin;
    intScore outFilterMultimapScoreRange;//min score to output
    double outFilterMismatchNoverLmax; //max proportion of all MM within all bases
    char *outFilterIntronMotifs;
    char *outFilterType; //type of filtering
    int outFilterBySJoutStage; //indicates the stage of filtering by SJout
    
    //output filtering SJs
    char *outSJfilterReads;
    // The following 5 variables are of type: vector <int32>
    gsl_vector_int *outSJfilterCountUniqueMin;
    gsl_vector_int *outSJfilterCountTotalMin;
    gsl_vector_int *outSJfilterOverhangMin;
    gsl_vector_int *outSJfilterDistToOtherSJmin; //min allowed distance to other SJ's donor/acceptor
    gsl_vector_int *outSJfilterIntronMaxVsReadN;
    
    //storage limits
    uint64_t limitGenomeGenerateRAM;
    uint64_t limitIObufferSize; //max size of the in/out buffer, bytes
    
    // penalties
    intScore scoreGap;
    intScore scoreGapNoncan;
    intScore scoreGapGCAG;
    intScore scoreGapATAC;
    intScore scoreDelBase;
    intScore scoreDelOpen;
    intScore scoreInsBase;
    intScore scoreInsOpen;
    intScore scoreStitchSJshift;//Max negative score when
    double scoreGenomicLengthLog2scale;
    
    //old variables: CLEAN-up needed
    char outputBED[MAX_OUTPUT_FLAG]; //output flags
    
    //SW search
    uint64_t swMode, swWinCoverageMinP;
    //SW penalties
    uint64_t swPeoutFilterMatchNmin, swPenMismatch, swPenGapOpen, swPenGapExtend;
    uint64_t swHsize;
    
    int annotScoreScale;//overall multiplication factor for the annotation
    char *annotSignalFile;//binary file with annotation signal
    
    //SJ database parameters
    char *sjdbFileChrStartEnd;
    char *sjdbGTFfile;
    char *sjdbGTFchrPrefix;
    char *sjdbGTFfeatureExon;
    char *sjdbGTFtagExonParentTranscript;
    uint64_t sjdbOverhang;
    uint64_t sjdbLength; //length of the donor/acceptor, length of the sj "chromosome" =2*sjdbOverhang+1 including spacer
    int sjdbScore;
    
    uint64_t sjChrStart;
    uint64_t sjdbN; //first sj-db chr
    uint64_t sjGstart; //start of the sj-db genome sequence
    uint64_t *sjDstart;
    uint64_t *sjAstart;
    uint64_t *sjStr;
    uint64_t *sjdbStart;
    uint64_t *sjdbEnd; //sjdb loci
    uint8_t *sjdbMotif; //motifs of annotated junctions
    uint8_t *sjdbShiftLeft;
    uint8_t *sjdbShiftRight; //shifts of junctions
    uint8_t *sjdbStrand; //junctions strand, not used yet
    
    uint64_t sjNovelN;
    uint64_t *sjNovelStart;
    uint64_t *sjNovelEnd; //novel junctions collapased and filtered
    
    //chimeric
    uint64_t chimSegmentMin;
    uint64_t chimJunctionOverhangMin; //min chimeric donor/acceptor length
    uint64_t maxChimReadGap; //max read gap for stitching chimeric windows
    int chimScoreMin;
    int chimScoreDropMax;
    int chimScoreSeparation;
    int chimScoreJunctionNonGTAG; //min chimeric score
    
    //splitting
    char Qsplit;
    uint64_t maxNsplit;
    uint64_t minLsplit;
    uint64_t minLmap;
    
    //limits
    
    
    ////////////////////// CLEAN-UP needed
//    InOutStreams *inOut; //main input output streams
    // replaced by logger.
    
    uint64_t Lread;
    
    //Genome parameters
    uint64_t nGenome;
    uint64_t nSA;
    uint64_t nSAbyte;
    uint64_t nChrReal;//genome length, SA length, # of chromosomes, vector of chromosome start loci
    uint64_t nSAi; //size of the SAindex
    gsl_vector_string *chrName;
//    map <string,uint64_t> chrNameIndex;
    gsl_vector_string *chrNameIndex;
    unsigned char GstrandBit;
    unsigned char SAiMarkNbit;
    unsigned char SAiMarkAbsentBit; //SA index bit for strand information
    uint64_t GstrandMask;
    uint64_t SAiMarkAbsentMask;
    uint64_t SAiMarkAbsentMaskC;
    uint64_t SAiMarkNmask;
    uint64_t SAiMarkNmaskC;//maske to remove strand bit from SA index, to remove mark from SAi index
};

typedef struct Parameters_struct Parameters;

Parameters *alder_parameter_init();
void alder_parameter_free(Parameters *p);

void alder_chrInfoLoad(); //find nChr and chrStart from genome
void alder_chrBinFill(); //file chrBin array
int alder_readParsFromFile(FILE*, FILE*, int); //read parameters from one file
int alder_readPars(); // read parameters from all files
int alder_scanOneLine (char *lineIn, int inputLevel, int inputLevelRequested);
void alder_scanAllLines (FILE *streamIn, int inputLevel, int inputLevelRequested);
void alder_inputParameters (int argInN, char* argIn[]); //input parameters: default, from files, from command line

__END_DECLS

#endif
