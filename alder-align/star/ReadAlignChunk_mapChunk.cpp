#include "ReadAlignChunk.h"
#include "GlobalVariables.h"
#include "ThreadControl.h"

void ReadAlignChunk::mapChunk() {//map one chunk. Input reads stream has to be setup in RA->readInStream[ii]
    RA->statsRA.resetN();       
    
    for (uint ii=0;ii<P->readNmates;ii++) {//clear eof and rewind the input streams
        RA->readInStream[ii]->clear();
        RA->readInStream[ii]->seekg(0,ios::beg);
    };
    
    int readStatus=0;
    while (readStatus==0) {//main cycle over all reads

        readStatus=RA->oneRead(); //map one read

        if (readStatus==0) RA->iRead++;

        //write SAM aligns to chunk buffer
        uint chunkOutSAMtotal=(uint) RA->outSAMstream->tellp();
        if ( chunkOutSAMtotal > P->chunkOutSAMsizeBytes ) {//this should not happen, is probably a bug
            P->inOut->logMain <<"EXITING because of fatal error: buffer size for SAM output is too small\n";
            P->inOut->logMain <<"Solution: increase input parmameter chunkOutSAMsizeBytes"<< endl<<flush;
            exit(1);
        } else if ( chunkOutSAMtotal + 100000 > P->chunkOutSAMsizeBytes || (readStatus==-1 && noReadsLeft) ) {//write buffer to disk because it's almost full, or all reads are mapped
            if (P->runThreadN>1) pthread_mutex_lock(&g_threadChunks.mutexOutSAM);                
            P->inOut->outSAM->write(chunkOutSAM,chunkOutSAMtotal);
            if (P->runThreadN>1) pthread_mutex_unlock(&g_threadChunks.mutexOutSAM);
            RA->outSAMstream->seekp(0,ios::beg);               
        };

        //write SJs to chunk buffer
        if ( chunkOutSJ->N + 10 > P->chunkOutSJsize ) {//this means the number of collapsed junctions is larger than the chunks size
            P->inOut->logMain <<"EXITING because of fatal error: buffer size for SJ output is too small\n";
            P->inOut->logMain <<"Solution: increase input parmameter chunkOutSJsizeBytes"<< endl<<flush;
            exit(1);
        } else if ( chunkOutSJ->N + 1000 > P->chunkOutSJsize || (readStatus==-1 && noReadsLeft) ) {//write buffer to disk because it's almost full, or all reads are mapped
            chunkOutSJ->collapseSJ();
        };            

        //write SJs to chunk buffer
        if ( chunkOutSJ1->N + 10 > P->chunkOutSJsize ) {//this means the number of collapsed junctions is larger than the chunks size
            P->inOut->logMain <<"EXITING because of fatal error: buffer size for SJ output is too small\n";
            P->inOut->logMain <<"Solution: increase input parmameter chunkOutSJsizeBytes"<< endl<<flush;
            exit(1);
        } else if ( chunkOutSJ1->N + 1000 > P->chunkOutSJsize || (readStatus==-1 && noReadsLeft) ) {//write buffer to disk because it's almost full, or all reads are mapped
            chunkOutSJ1->collapseSJ();
        };             

    }; //reads cycle

    //add stats, write progress if needed
    if (P->runThreadN>1) pthread_mutex_lock(&g_threadChunks.mutexStats);
    g_statsAll.addStats(RA->statsRA);
    g_statsAll.progressReport(P->inOut->logProgress);
    if (P->runThreadN>1) pthread_mutex_unlock(&g_threadChunks.mutexStats); 
};            