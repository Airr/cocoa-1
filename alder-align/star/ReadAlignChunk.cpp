#include "ReadAlignChunk.h"
#include <pthread.h>

ReadAlignChunk::ReadAlignChunk(Parameters* Pin, Genome &genomeIn, int iChunk) : P(Pin) {//initialize chunk
    
    RA = new ReadAlign(P,genomeIn);//new local copy of RA for each chunk
    
    chunkIn=new char* [P->readNmates];
    readInStream=new istringstream* [P->readNmates];
//     readInStream=new istringstream* [P->readNmates];
    for (uint ii=0;ii<P->readNmates;ii++) {
       chunkIn[ii]=new char[P->chunkInSizeBytesArray];//reserve more space to finish loading one read
       memset(chunkIn[ii],'\n',P->chunkInSizeBytesArray);
       readInStream[ii] = new istringstream;
       readInStream[ii]->rdbuf()->pubsetbuf(chunkIn[ii],P->chunkInSizeBytesArray);
       RA->readInStream[ii]=readInStream[ii];
    };
    
    chunkOutSAM=new char [P->chunkOutSAMsizeBytes];
    chunkOutSAMstream=new ostringstream;
    chunkOutSAMstream->rdbuf()->pubsetbuf(chunkOutSAM,P->chunkOutSAMsizeBytes);
    RA->outSAMstream=chunkOutSAMstream;
    RA->outSAMstream->seekp(0,ios::beg);
    
    chunkOutSJ=new OutSJ (P->chunkOutSJsize, P);
    chunkOutSJ1=new OutSJ (P->chunkOutSJsize, P);

    RA->chunkOutSJ=chunkOutSJ;
    RA->chunkOutSJ1=chunkOutSJ1;
    
    if (P->chimSegmentMin>0) {
        chunkFstreamOpen(P->outFileTmp + "/Chimeric.out.sam.thread", iChunk, RA->chunkOutChimSAM);
        chunkFstreamOpen(P->outFileTmp + "/Chimeric.out.junction.thread", iChunk, RA->chunkOutChimJunction);   
    };
    if (P->outReadsUnmapped=="Fastx" ) {    
        chunkFstreamOpen(P->outFileTmp + "/Unmapped.out.mate1.thread",iChunk, RA->chunkOutUnmappedReadsStream[0]);
        if (P->readNmates==2) chunkFstreamOpen(P->outFileTmp + "/Unmapped.out.mate2.thread",iChunk, RA->chunkOutUnmappedReadsStream[1]);
    };
    if (P->outFilterType=="BySJout") {
        chunkFstreamOpen(P->outFileTmp + "/FilterBySJoutFiles.mate1.thread",iChunk, RA->chunkOutFilterBySJoutFiles[0]);
        if (P->readNmates==2) chunkFstreamOpen(P->outFileTmp + "/FilterBySJoutFiles.mate2.thread",iChunk, RA->chunkOutFilterBySJoutFiles[1]);
    };
};

///////////////
void ReadAlignChunk::chunkFstreamOpen(string filePrefix, int iChunk, fstream &fstreamOut) {//open fstreams for chunks
    ostringstream fName1;
    fName1 << filePrefix << iChunk;
    remove(fName1.str().c_str());
    fstreamOut.open(fName1.str().c_str(), ios::in | ios::out | ios::app); //TODO check that file could be opened
};

void ReadAlignChunk::chunkFstreamCat (fstream &chunkOut, ofstream &allOut, bool mutexFlag, pthread_mutex_t &mutexVal){
    chunkOut.flush();
    chunkOut.seekg(0,ios::beg);
    if (mutexFlag) pthread_mutex_lock(&mutexVal);
    allOut << chunkOut.rdbuf();
    if (mutexFlag) pthread_mutex_unlock(&mutexVal);
    chunkOut.clear();
    chunkOut.seekp(0,ios::beg); //set put pointer at the beginning
};
        
