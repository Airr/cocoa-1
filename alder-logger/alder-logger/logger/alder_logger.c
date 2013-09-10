/**
 * This file, alder_logger.c, is part of alder-logger.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-logger.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "nglogc/log.h"
#include "alder_logger.h"

// 0: success
int alder_logger_error_initialize(logc_logLevel_t l)
{
    logc_error_t r = logc_registerLogger(ERROR_LOGGER, STDERROUT, l);
    if (r != LOG_ERR_OK) {
        fprintf(stderr, "error : cannot register an error logger.");
    }
    return r;
}

void  alder_logger_error_finalize()
{
    logc_removeLogger(ERROR_LOGGER);
}

// 0: success
int alder_logger_initialize (const char *mainLoggerFilename,
                             logc_logLevel_t l)
{
    remove(mainLoggerFilename);
    logc_registerLogger(MAIN_LOGGER, FILEOUT, l);
    logc_error_t status = logc_setLogfile(MAIN_LOGGER, mainLoggerFilename);
    if (status != LOG_ERR_OK) {
        fprintf(stderr, "error: no log is defined\n");
    }
    return status;
}

void alder_logger_finalize ()
{
    logc_removeLogger(MAIN_LOGGER);
}