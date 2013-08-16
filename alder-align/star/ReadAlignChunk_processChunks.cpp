#include "ReadAlignChunk.h"
#include "GlobalVariables.h"
#include "ThreadControl.h"
#include "ErrorWarning.h"

void ReadAlignChunk::processChunks() {//read-map-write chunks  
    noReadsLeft=false; //true if there no more reads left in the file
    
    while (!noReadsLeft) {//continue until the input EOF
            //////////////read a chunk from input files and store in memory
        if (P->outFilterBySJoutStage<2) {//read chunks from input file

            if (P->runThreadN>1) pthread_mutex_lock(&g_threadChunks.mutexInRead);

            uint chunkInSizeBytesTotal[2]={0,0};
            while (chunkInSizeBytesTotal[0] < P->chunkInSizeBytes && chunkInSizeBytesTotal[1] < P->chunkInSizeBytes && !P->inOut->readIn[0].eof() && !P->inOut->readIn[1].eof()) {
                char nextChar=P->inOut->readIn[0].peek();
                if (nextChar=='@') {//fastq, not multi-line
                    for (uint imate=0; imate<P->readNmates; imate++) {                    
                        for (uint imate=0; imate<P->readNmates; imate++) {
                            for (int iline=0;iline<4;iline++) {
                                P->inOut->readIn[imate].getline(chunkIn[imate] + chunkInSizeBytesTotal[imate], MAX_READ_LENGTH+1 );
                                chunkInSizeBytesTotal[imate] += P->inOut->readIn[imate].gcount();   
                                chunkIn[imate][chunkInSizeBytesTotal[imate]-1]='\n';                        
                            };
                        };
                    };
                } else if (nextChar=='>') {//fasta, can be multiline, which is converted to single line
                    for (uint imate=0; imate<P->readNmates; imate++) {
                        //read read name
                        P->inOut->readIn[imate].getline(chunkIn[imate] + chunkInSizeBytesTotal[imate], MAX_READ_LENGTH+1 );
                        if (P->inOut->readIn[imate].gcount()<2) break; //no more input
                        chunkInSizeBytesTotal[imate] += P->inOut->readIn[imate].gcount();                           
                        chunkIn[imate][chunkInSizeBytesTotal[imate]-1]='\n';                         
                        nextChar=P->inOut->readIn[imate].peek();                        
                        while (nextChar!='@' && nextChar!='>' && nextChar!=' ' && nextChar!='\n' && !P->inOut->readIn[0].eof()) {//read multi-line fasta
                            P->inOut->readIn[imate].getline(chunkIn[imate] + chunkInSizeBytesTotal[imate], MAX_READ_LENGTH+1 );
                            if (P->inOut->readIn[imate].gcount()<2) break; //no more input
                            chunkInSizeBytesTotal[imate] += P->inOut->readIn[imate].gcount()-1;   
                            nextChar=P->inOut->readIn[imate].peek();
                        };
                        chunkIn[imate][chunkInSizeBytesTotal[imate]]='\n'; 
                        chunkInSizeBytesTotal[imate] ++;   
                    };                
                } else if (nextChar==' ' || nextChar=='\n' || P->inOut->readIn[0].eof()) {//end of stream
                    break;
                } else {//error
                    ostringstream errOut;
                    errOut << "EXITING because of FATAL ERROR in input reads: unknown file format: the read ID should start with @ or > \n";
                    exitWithError(errOut.str(),std::cerr, P->inOut->logMain, EXIT_CODE_INPUT_FILES, *P);                      
                };
            };
            //TODO: check here that both mates are zero or non-zero
            if (chunkInSizeBytesTotal[0]==0) {
                noReadsLeft=true; //true if there no more reads left in the file
            } else {
                noReadsLeft=false; 
            };

            for (uint imate=0; imate<P->readNmates; imate++) chunkIn[imate][chunkInSizeBytesTotal[imate]]='\n';//extra empty line at the end of the chunks

            if (P->runThreadN>1) pthread_mutex_unlock(&g_threadChunks.mutexInRead);
            
        } else {//read from one file per thread
            noReadsLeft=true;
            for (uint imate=0; imate<P->readNmates; imate++) {
                RA->chunkOutFilterBySJoutFiles[imate].flush();
                RA->chunkOutFilterBySJoutFiles[imate].seekg(0,ios::beg);
                RA->readInStream[imate]=& RA->chunkOutFilterBySJoutFiles[imate];
            };
        };
        
        mapChunk();
                
    };//cycle over input chunks
    
    //the thread is finsihed mapping reads, concatenate the temp files into output files
    if (P->chimSegmentMin>0) {    
        chunkFstreamCat (RA->chunkOutChimSAM, P->inOut->outChimSAM, P->runThreadN>1, g_threadChunks.mutexOutChimSAM);
        chunkFstreamCat (RA->chunkOutChimJunction, P->inOut->outChimJunction, P->runThreadN>1, g_threadChunks.mutexOutChimJunction);
    };
    if (P->outReadsUnmapped=="Fastx" ) {
        for (uint ii=0;ii<P->readNmates;ii++) {    
            chunkFstreamCat (RA->chunkOutUnmappedReadsStream[ii],P->inOut->outUnmappedReadsStream[ii], P->runThreadN>1, g_threadChunks.mutexOutUnmappedFastx);
        };
    };
};

