//
//  alder_logger.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <nglogc/log.h>
//#include <nglogc/logger.h>
#include "alder_logger.h"

int alder_logger_initialize ()
{
    char *mainLoggerFilename = "alder-align.log";
    logc_registerLogger(MAIN_LOGGER, FILEOUT, LOG_BASIC);
//    logc_registerLogger(MAIN_LOGGER, FILEOUT, LOG_FINEST);
    logc_error_t status = logc_setLogfile(MAIN_LOGGER, mainLoggerFilename);
    if (status != LOG_ERR_OK) {
        printf("Error: no log is defined\n");
    }
    logc_log(MAIN_LOGGER, LOG_BASIC, "");
    logc_setLogFormat(MAIN_LOGGER, TIMESTAMP_ERR_TAG, TIMESTAMP);
    return GSL_SUCCESS;
}

int alder_logger_finalize ()
{
    logc_removeLogger(MAIN_LOGGER);
    return GSL_SUCCESS;
}