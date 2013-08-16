#include "IncludeDefine.h"
#include "Parameters.h"
#include "SequenceFuns.h"
#include "Genome.h"
#include "ReadAlignChunk.h"
#include "ReadAlign.h"
#include "Stats.h"
#include "genomeGenerate.h"
#include "outputSJ.h"
#include "ThreadControl.h"
#include "GlobalVariables.cpp"
#include "TimeFunctions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "ErrorWarning.h"

int main(int argInN, char* argIn[]) {
//     system("echo `date` ..... Starting STAR run > Log.timing.out");
    
    time(&g_statsAll.timeStart);
   

//     g_statsGlobal = new StatsAll;
//     g_statsGlobal->resetNall();       
    
    Parameters *P = new Parameters; //all parameters
       
    P->inputParameters(argInN, argIn);
    
    *(P->inOut->logStdOut) << timeMonthDayTime(g_statsAll.timeStart) << " ..... Started STAR run\n" <<flush;        

    //generate genome
    if (P->runMode=="genomeGenerate") {
        genomeGenerate(P);
        system (("rm -rf " + P->outFileTmp).c_str());        
        P->inOut->logMain << "DONE: Genome generation, EXITING\n" << flush;
//         system("echo `date` ..... Finished genome generation >> Log.timing.out");
        exit(0);
    } else if (P->runMode!="alignReads") {
        P->inOut->logMain << "EXITING because of INPUT ERROR: unknown value of input parameter runMode=" <<P->runMode<<endl<<flush;
        exit(1);
    };
    
    Genome mainGenome;
    mainGenome.genomeLoad(P);
    //calculate genome-related parameters
    P->winBinN = P->nGenome/(1LLU << P->winBinNbits)+1;

/////////////////////////////////////////////////////////////////////////////////////////////////START
    
    if (P->outSAMmode != "None") {//open SAM file and write header
        *P->inOut->outSAM << "@PG\tID:STAR\tPN:STAR\tVN:" << SVN_VERSION_COMPILED <<"\tCL:" << P->commandLineFull <<  "\tcl:" << P->commandLine <<endl;
        for (uint ii=0;ii<P->nChrReal;ii++) {
            *P->inOut->outSAM << "@SQ\tSN:"<< P->chrName.at(ii) <<"\tLN:"<<P->chrLength[ii]<<endl;
        };
    };
    
    if (P->chimSegmentMin>0) {
        P->inOut->outChimJunction.open((P->outFileNamePrefix + "Chimeric.out.junction").c_str());
        P->inOut->outChimSAM.open((P->outFileNamePrefix + "Chimeric.out.sam").c_str());
        P->inOut->outChimSAM << "@PG\tID:STAR\tPN:STAR\tVN:" << SVN_VERSION_COMPILED <<"\tCL:" << P->commandLineFull <<  "\tcl:" << P->commandLine <<endl;
        
        for (uint ii=0;ii<P->nChrReal;ii++) {
            P->inOut->outChimSAM << "@SQ\tSN:"<< P->chrName.at(ii) <<"\tLN:"<<P->chrLength[ii]<<endl;
        };        
        pthread_mutex_init(&g_threadChunks.mutexOutChimSAM, NULL);   
        pthread_mutex_init(&g_threadChunks.mutexOutChimJunction, NULL);
    };
         
    // P->inOut->logMain << "mlock value="<<mlockall(MCL_CURRENT|MCL_FUTURE) <<"\n"<<flush;
    
//     omp_set_num_threads(P->runThreadN);
    
    ReadAlignChunk *RAchunk[P->runThreadN];
    for (int ii=0;ii<P->runThreadN;ii++) {
        RAchunk[ii]=new ReadAlignChunk(P, mainGenome, ii);
        RAchunk[ii]->RA->iRead=0;
    };
    
    if (P->runThreadN>1) {
        g_threadChunks.threadArray=new pthread_t[P->runThreadN];
        pthread_mutex_init(&g_threadChunks.mutexInRead, NULL);
        pthread_mutex_init(&g_threadChunks.mutexOutSAM, NULL);
        pthread_mutex_init(&g_threadChunks.mutexOutUnmappedFastx, NULL);
        pthread_mutex_init(&g_threadChunks.mutexOutFilterBySJout, NULL);
        pthread_mutex_init(&g_threadChunks.mutexStats, NULL);
    };
    
    
    ///////////////////////////////////////////////////////////////////
    g_statsAll.progressReportHeader(P->inOut->logProgress);    
//     system("echo `date` ..... Starting mapping >> Log.timing.out");
    time(&g_statsAll.timeStartMap);
    *P->inOut->logStdOut << timeMonthDayTime(g_statsAll.timeStartMap) << " ..... Started mapping\n" <<flush;
    
    g_statsAll.timeLastReport=g_statsAll.timeStartMap;
    
    for (int ithread=1;ithread<P->runThreadN;ithread++) {//spawn threads
        pthread_create(&g_threadChunks.threadArray[ithread], NULL, &g_threadChunks.threadRAprocessChunks, (void *) RAchunk[ithread]);
    };
    
    RAchunk[0]->processChunks(); //start main thread
    
    for (int ithread=1;ithread<P->runThreadN;ithread++) {//wait for all threads to complete
        int threadJoinStatus = pthread_join(g_threadChunks.threadArray[ithread], NULL);
        if (threadJoinStatus) {//something went wrong with one of threads
                ostringstream errOut;
                errOut << "EXITING because of FATAL ERROR: phtread error while joining thread # " << ithread <<", error code: "<<threadJoinStatus ;
                exitWithError(errOut.str(),std::cerr, P->inOut->logMain, 1, *P);
        };
    };    
    
    if (P->outFilterBySJoutStage==1) {//completed stage 1, go to stage 2
        outputSJ(RAchunk,P);//collapse novel junctions
        
        P->outFilterBySJoutStage=2;
        
        for (int ithread=1;ithread<P->runThreadN;ithread++) {//spawn threads
            pthread_create(&g_threadChunks.threadArray[ithread], NULL, &g_threadChunks.threadRAprocessChunks, (void *) RAchunk[ithread]);
        };

        RAchunk[0]->processChunks(); //start main thread

        for (int ithread=1;ithread<P->runThreadN;ithread++) {//wait for all threads to complete
            int threadJoinStatus = pthread_join(g_threadChunks.threadArray[ithread], NULL);
            if (threadJoinStatus) {//something went wrong with one of threads
                ostringstream errOut;
                errOut << "EXITING because of FATAL ERROR: phtread error while joining thread # " << ithread <<", error code: "<<threadJoinStatus ;
                exitWithError(errOut.str(),std::cerr, P->inOut->logMain, 1, *P);
            };
        };  
    };
    
    //aggregate output (junctions, signal, etc)
    //collapse splice junctions from different threads/chunks, and output them
    outputSJ(RAchunk,P);
    
    g_statsAll.progressReport(P->inOut->logProgress);
    P->inOut->logProgress  << "ALL DONE!\n"<<flush;
    P->inOut->logFinal.open((P->outFileNamePrefix + "Log.final.out").c_str());
    g_statsAll.reportFinal(P->inOut->logFinal,P);
    *P->inOut->logStdOut << timeMonthDayTime(g_statsAll.timeFinish) << " ..... Finished successfully\n" <<flush;
    
    P->inOut->logMain  << "ALL DONE!\n"<<flush;
    delete P->inOut; //to close files
    
    if (P->readFilesCommand.at(0)!="-"){//remove fifo files
        for (uint ii=0;ii<P->readNmates;ii++) {
            remove(P->readFilesInTmp.at(ii).c_str());
        };
    };
    
//     if (P->outFilterBySJoutStage>0) {
//         for (uint ii=0;ii<P->readNmates;ii++) {
//             remove(P->outFilterBySJoutFileNames.at(ii).c_str());
//         };        
//     };
    
    system (("rm -rf " + P->outFileTmp).c_str());

    return 0;    
};
