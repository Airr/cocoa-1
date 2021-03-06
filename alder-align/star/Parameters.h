#ifndef PARAMETERS_DEF
#define PARAMETERS_DEF

#include "IncludeDefine.h"
#include "InOutStreams.h"
#include "ParameterInfo.h"
#include <map>

class Parameters {
    
    public:
        vector <ParameterInfoBase*> parArray, parArrayInitial;
        vector <string> parameterInputName;
        
        string commandLine, commandLineFull;
        
        //version
        uint versionSTAR;
        vector <uint> versionGenome;
        
        // run parameters
        string runMode;
        int   runThreadN;
    
        //parameters
        vector <string> parametersFiles;
        
        //genome, SA, ...
        vector <uint> chrStart, chrLength;
        string genomeDir,genomeLoad;
        vector <string> genomeFastaFiles; 
        uint genomeSAsparseD;//sparsity=distance between indices
        //binning,windows,anchors
        uint genomeChrBinNbits, genomeChrBinNbases, chrBinN, *chrBin;
        uint winBinChrNbits, winBinNbits, winAnchorDistNbins, winFlankNbins, winBinN;
        uint winAnchorMultimapNmax; //max number of alignments for anchors

        uint genomeSAindexNbases; //length of the SA pre-index strings
        uint *genomeSAindexStart;//starts of the L-mer indices in the SAindex, 1<=L<=genomeSAindexNbases
        
        
        //read parameters
        vector <string> readFilesIn, readFilesInTmp;
        uint readNmates;
        string readMatesLengthsIn;
        vector <string> readFilesCommand;
        
        vector <uint> clip5pNbases, clip3pNbases, clip3pAfterAdapterNbases;    
        vector <double> clip3pAdapterMMp;
        vector <string> clip3pAdapterSeq;
        char *clip3pAdapterSeqNum[MAX_N_MATES];//adapter sequence - numerical
        bool readMatesEqualLengths; //whether or not the read mates have the same length, true if onyl one mate
        
        //align parameters
        uint alignSJoverhangMin,alignSJDBoverhangMin,alignSplicedMateMapLmin; //min SJ donor/acceptor length
        double alignSplicedMateMapLminOverLmate;
        uint alignWindowsPerReadNmax; //max number of alignment windows per read
        uint alignTranscriptsPerWindowNmax; //maximum number of transcripts recorded per window
        uint alignTranscriptsPerReadNmax;   //max number of alignments per read     
        uint alignIntronMin;//min length to call a gap an intron
        uint alignIntronMax;//max length to call 
        uint alignMatesGapMax;//max gap between the mates (if paired-end)
        
        //seed parameters
        uint seedMultimapNmax; //max number of multiple alignments per piece          
        uint seedSearchLmax; //max length of the seed
        uint seedPerReadNmax; //max number of pieces per Read
        uint seedPerWindowNmax; //max number of aligns per window
        uint seedNoneLociPerWindow; //max number of aligns from one piece per window
        uint seedSearchStartLmax;
        double seedSearchStartLmaxOverLread; //length of split start points

        //chunk parameters
        uint chunkInSizeBytes,chunkInSizeBytesArray,chunkOutSAMsizeBytes, chunkOutSJsize;
        
        //output
        string outFileNamePrefix, outStd;
        string outSAMmode, outSAMstrandField, outSAMattributes, outSAMunmapped;
        string outReadsUnmapped;
        int outQSconversionAdd;
        string outFileTmp;
        
        //output filtering
        uint outFilterMatchNmin,outFilterMismatchNmax,outFilterMultimapNmax;//min number of matches
        double outFilterScoreMinOverLread, outFilterMatchNminOverLread;//normalzied to read length
        intScore outFilterScoreMin,outFilterMultimapScoreRange;//min score to output
        double outFilterMismatchNoverLmax; //max proportion of all MM within all bases
        string outFilterIntronMotifs;
        string outFilterType; //type of filtering 
        int outFilterBySJoutStage; //indicates the stage of filtering by SJout 
        
        //output filtering SJs
        string outSJfilterReads;
        vector <int32> outSJfilterCountUniqueMin, outSJfilterCountTotalMin;
        vector <int32> outSJfilterOverhangMin;
        vector <int32> outSJfilterDistToOtherSJmin; //min allowed distance to other SJ's donor/acceptor
        vector <int32> outSJfilterIntronMaxVsReadN;
        
        //storage limits
        uint limitGenomeGenerateRAM;
        uint limitIObufferSize; //max size of the in/out buffer, bytes
        
        // penalties
        intScore scoreGap, scoreGapNoncan, scoreGapGCAG, scoreGapATAC, scoreDelBase, scoreDelOpen, scoreInsBase, scoreInsOpen; 
        intScore scoreStitchSJshift;//Max negative score when
        double scoreGenomicLengthLog2scale;        

        //old variables: CLEAN-up needed
        char outputBED[MAX_OUTPUT_FLAG]; //output flags
        
        //SW search
        uint swMode, swWinCoverageMinP;
        //SW penalties
        uint swPeoutFilterMatchNmin, swPenMismatch, swPenGapOpen, swPenGapExtend;
        uint swHsize;        
        
        int annotScoreScale;//overall multiplication factor for the annotation
        string annotSignalFile;//binary file with annotation signal
    
        //SJ database parameters
        string sjdbFileChrStartEnd;
        string sjdbGTFfile, sjdbGTFchrPrefix, sjdbGTFfeatureExon, sjdbGTFtagExonParentTranscript;
        uint sjdbOverhang,sjdbLength; //length of the donor/acceptor, length of the sj "chromosome" =2*sjdbOverhang+1 including spacer
        int sjdbScore;        
        
        uint sjChrStart,sjdbN; //first sj-db chr
        uint sjGstart; //start of the sj-db genome sequence
        uint *sjDstart,*sjAstart,*sjStr, *sjdbStart, *sjdbEnd; //sjdb loci
        uint8 *sjdbMotif; //motifs of annotated junctions
        uint8 *sjdbShiftLeft, *sjdbShiftRight; //shifts of junctions
        uint8 *sjdbStrand; //junctions strand, not used yet
        
        uint sjNovelN, *sjNovelStart, *sjNovelEnd; //novel junctions collapased and filtered
        
        //chimeric
        uint chimSegmentMin, chimJunctionOverhangMin; //min chimeric donor/acceptor length
        uint maxChimReadGap; //max read gap for stitching chimeric windows
        int chimScoreMin,chimScoreDropMax,chimScoreSeparation, chimScoreJunctionNonGTAG; //min chimeric score
        
        //splitting
        char Qsplit;
        uint maxNsplit, minLsplit, minLmap;
        
        //limits
    
        
    ////////////////////// CLEAN-UP needed
    InOutStreams *inOut; //main input output streams

    uint Lread;
            
    //Genome parameters
    uint nGenome, nSA, nSAbyte, nChrReal;//genome length, SA length, # of chromosomes, vector of chromosome start loci
    uint nSAi; //size of the SAindex
    vector <string> chrName;
    map <string,uint> chrNameIndex;
    unsigned char GstrandBit, SAiMarkNbit, SAiMarkAbsentBit; //SA index bit for strand information
    uint GstrandMask, SAiMarkAbsentMask, SAiMarkAbsentMaskC, SAiMarkNmask, SAiMarkNmaskC;//maske to remove strand bit from SA index, to remove mark from SAi index
    
   

    Parameters();
    void chrInfoLoad(); //find nChr and chrStart from genome
    void chrBinFill();//file chrBin array
    int readParsFromFile(ifstream*, ofstream*, int); //read parameters from one file
    int readPars(); // read parameters from all files
    int scanOneLine (string &lineIn, int inputLevel, int inputLevelRequested);
    void scanAllLines (istream &streamIn, int inputLevel, int inputLevelRequested);
    void inputParameters (int argInN, char* argIn[]); //input parameters: default, from files, from command line    
};
#endif  // Parameters.h
