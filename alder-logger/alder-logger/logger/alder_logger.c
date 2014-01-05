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
#include <string.h>
#include <errno.h>
#include "nglogc/log.h"
#include "alder_logger.h"

/* I can turn off the logger by setting the log level to SILENT.
 * Then, only the message of logc_log with SILENT level would be
 * written out to the log. I can see more and more log message
 * by raising the log level using logc_changeLogLevel function.
 * For example, with BASIC level turned on, log messages with 
 * two levels (SILENT and BASIC) would be printed out. In the 
 * level of FINEST with the call of logc_changeLogLevel, 
 * all of log message would be printed out.
 */
static void alder_logger_test_6message(uint16_t logger, const char *l)
{
    logc_log(logger, LOG_SILENT,  "%s: silent", l);
    logc_log(logger, LOG_BASIC,   "%s: basic", l);
    logc_log(logger, LOG_WARNING, "%s: warning", l);
    logc_log(logger, LOG_INFO,    "%s: info", l);
    logc_log(logger, LOG_FINE,    "%s: fine", l);
    logc_log(logger, LOG_FINEST,  "%s: finest", l);
}

/* Test function: a log file is not created if no log writer did with
 * higher than or equal to the given log level.
 */
int alder_logger_test01()
{
    alder_logger_initialize("01.log", LOG_SILENT);
    logc_log(MAIN_LOGGER, LOG_BASIC, "silent: basic - should not be written");
    alder_logger_finalize();
    return 0;
}

int alder_logger_test02()
{
    alder_logger_initialize("02.log", LOG_BASIC);
    alder_log("This is message %s %d", "aaa", 2);
    alder_logger_finalize();
    return 0;
}

int alder_logger_test03()
{
    alder_logger_initialize("03.log", LOG_BASIC);
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_SILENT);
    alder_logger_test_6message(MAIN_LOGGER, "silent");
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_BASIC);
    alder_logger_test_6message(MAIN_LOGGER, "basic");
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_WARNING);
    alder_logger_test_6message(MAIN_LOGGER, "warning");
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_INFO);
    alder_logger_test_6message(MAIN_LOGGER, "info");
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_FINE);
    alder_logger_test_6message(MAIN_LOGGER, "fine");
    
    logc_changeLogLevel(MAIN_LOGGER, LOG_FINEST);
    alder_logger_test_6message(MAIN_LOGGER, "fineset");
    
    alder_logger_finalize();
    return 0;
}

int alder_logger_test04()
{
    alder_logger_error_initialize(LOG_SILENT);
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_SILENT);
    alder_logger_test_6message(ERROR_LOGGER, "silent");
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_BASIC);
    alder_logger_test_6message(ERROR_LOGGER, "basic");
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_WARNING);
    alder_logger_test_6message(ERROR_LOGGER, "warning");
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_INFO);
    alder_logger_test_6message(ERROR_LOGGER, "info");
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_FINE);
    alder_logger_test_6message(ERROR_LOGGER, "fine");
    
    logc_changeLogLevel(ERROR_LOGGER, LOG_FINEST);
    alder_logger_test_6message(ERROR_LOGGER, "fineset");
    
    alder_logger_error_finalize();
    return 0;
}

//#define LOGC_ENABLE_LOW_LEVEL 
//#define LOGC_HAVE_FLF

int alder_logger_test05()
{
    /* trace the function call */
//    logc_logEnter(MAIN_LOGGER, "runLogs");
    /* log an error message with the log level LOG_BASIC*/
    logc_logErrorBasic(MAIN_LOGGER, 0, "This is a LOG_BASIC error message");
    /* log an error message with the log level LOG_WARNING */
    logc_logErrorWarning(MAIN_LOGGER, 0, "This is a LOG_WARNING error message");
    /* log an error message with the log level LOG_INFO */
    logc_logErrorInfo(MAIN_LOGGER, 0, "This is a LOG_INFO error message");
    /* log an error message with the log level LOG_FINE */
    logc_logErrorFine(MAIN_LOGGER, 0, "This is a LOG_FINE error message");
    /* log an error message with the log level LOG_FINEST */
    logc_logErrorFinest(MAIN_LOGGER, 0, "This is a LOG_FINEST error message\n");
    
    /* log an log message with the log level LOG_BASIC */
    logc_logBasic(MAIN_LOGGER, "This is a LOG_BASIC log message");
    /* log an log message with the log level LOG_WARNING */
    logc_logWarning(MAIN_LOGGER, "This is a LOG_WARNING log message");
    /* log an log message with the log level LOG_INFO */
    logc_logInfo(MAIN_LOGGER, "This is a LOG_INFO log message");
    /* log an log message with the log level LOG_FINE */
    logc_logFine(MAIN_LOGGER, "This is a LOG_FINE log message");
    /* log an log message with the log level LOG_FINEST */
    logc_logFinest(MAIN_LOGGER, "This is a LOG_FINEST log message");
    /* trace the function call */
//    logc_logLeave(MAIN_LOGGER, "runLogs");
    return 0;
}

int alder_logger_test()
{
    alder_logger_test01();
    alder_logger_test02();
    alder_logger_test03();
//    alder_logger_test04();
    
    alder_logger_initialize("05.log", LOG_FINE);
    logc_setLogFormat(MAIN_LOGGER, ERR_TAG, CLEAN);
    alder_logger_test05();
    alder_logger_finalize();
    
//    alder_logger_test06();
   
    return 0;
}

/* This function initializes the logger.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_logger_error_initialize(logc_logLevel_t l)
{
    logc_error_t r = logc_registerLogger(ERROR_LOGGER, STDERROUT, l);
    if (r != LOG_ERR_OK) {
        fprintf(stderr, "Error - cannot register an error logger.\n");
        return 1;
    }
    logc_setLogFormat(ERROR_LOGGER, ERR_TAG, CLEAN);
//    logc_setLogFormat(ERROR_LOGGER, ERR_TAG_TIMESTAMP, CLEAN);
    return 0;
}

/* This function closes the error logger.
 */
void alder_logger_error_finalize()
{
    logc_removeLogger(ERROR_LOGGER);
}

/* This function initializes the logger.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_logger_initialize (const char *mainLoggerFilename,
                             logc_logLevel_t l)
{
//    int s = remove(mainLoggerFilename);
//    if (s != 0) {
//        fprintf(stderr, "Error - cannot open file %s: %s\n",
//                mainLoggerFilename, strerror(errno));
//        return 1;
//    }
    
    int s = 0;
//    logc_registerLogger(MAIN_LOGGER, FILEOUT, l);
//    logc_error_t status = logc_setLogfile(MAIN_LOGGER, mainLoggerFilename);
//    if (status != LOG_ERR_OK) {
//        fprintf(stderr, "Error - no log is defined\n");
//        s = 1;
//    }
    logc_registerLogger(MAIN_LOGGER, STDOUT, l);
//    logc_registerLogger(MAIN_LOGGER, STDERROUT, l);

    logc_setLogFormat(MAIN_LOGGER, ERR_TAG, CLEAN);
//    logc_setLogFormat(MAIN_LOGGER, ERR_TAG_TIMESTAMP, TIMESTAMP);
    return s;
}

/* This function finalizes the logger.
 */
void alder_logger_finalize ()
{
    logc_removeLogger(MAIN_LOGGER);
}