//
//  alder_logger.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
//#include <gsl/gsl_errno.h>
#include <nglogc/log.h>
//#include <nglogc/logger.h>
#include "alder_logger.h"

// 0 : success
int alder_error_logger_initialize(logc_logLevel_t l)
{
    logc_error_t r = logc_registerLogger(ERROR_LOGGER, STDERROUT, l);
    if (r != LOG_ERR_OK) {
        fprintf(stderr, "error : cannot register an error logger.");
    }
    return r;
}

void  alder_error_logger_finalize()
{
    logc_removeLogger(ERROR_LOGGER);
    
}

int alder_logger_initialize (const char *mainLoggerFilename,
                             logc_logLevel_t l)
{
    remove(mainLoggerFilename);
    
    logc_registerLogger(MAIN_LOGGER, FILEOUT, l); // for basic
//    logc_registerLogger(MAIN_LOGGER, FILEOUT, LOG_FINEST); // for debug
    logc_error_t status = logc_setLogfile(MAIN_LOGGER, mainLoggerFilename);
    
    if (status != LOG_ERR_OK) {
        fprintf(stderr, "error: no log is defined\n");
    }
//    logc_log(MAIN_LOGGER, LOG_INFO, "");
//    logc_setLogFormat(MAIN_LOGGER, TIMESTAMP_ERR_TAG, TIMESTAMP);
    return 0;
}

int alder_logger_finalize ()
{
    logc_removeLogger(MAIN_LOGGER);
    return 0;
}