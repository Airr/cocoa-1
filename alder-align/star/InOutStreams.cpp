#include "InOutStreams.h"
#include "GlobalVariables.h"

InOutStreams::InOutStreams() {
    logStdOut=NULL;
    outSAM=NULL;
};

InOutStreams::~InOutStreams() {
    
    if (logStdOut!=NULL) logStdOut->flush();
    if (outSAM!=NULL) outSAM->flush();
    
    logStdOutFile.flush();
    outSAMfile.flush();
    outChimSAM.flush();
    outChimJunction.flush();    
    logProgress.flush();
    logMain.flush();
    logFinal.flush();
    
    logStdOutFile.close();
    outSAMfile.close();
    outChimSAM.close();
    outChimJunction.close();    
    logProgress.close();
    logMain.close();
    logFinal.close();
};
   
