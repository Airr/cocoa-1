/**
 * This file, alder_logger.h, is part of alder-logger.
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

#ifndef alder_logger_alder_logger_h
#define alder_logger_alder_logger_h

#include "alder_logger_error.h"
#include "nglogc/log.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

#define MAIN_LOGGER  0x0001
#define ERROR_LOGGER 0x0002
#define ERR_TEST     0x00000001
#define EXAMPLE_ERR  0x00000001

/* I could change the level of logger in two aspects.
 * Users can change the log level, if they want, using
 * options available. The default would be SILENT, and 
 * would be BASIC if users turn on the logger. So, users
 * would be able to turn on or off the logger to see basic
 * information. 
 * From the my point of view, I would change the level as
 * I want. I would have the following alder_log macros.
 * The number following the alder_log function name means
 * the log level. 0 would be printed in any way. It would
 * be rare to use this function: alder_log0. Use alder_log
 * with greater numbers as you want to see them in much
 * detail of the program flow.
 */
#if !defined(NDEBUG)

#define alder_log0(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_SILENT, formatStr, ## __VA_ARGS__)
#define alder_log(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_BASIC, formatStr, ## __VA_ARGS__)
#define alder_log2(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_WARNING, formatStr, ## __VA_ARGS__)
#define alder_log3(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_INFO, formatStr, ## __VA_ARGS__)
#define alder_log4(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_FINE, formatStr, ## __VA_ARGS__)
#define alder_log5(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_FINEST, formatStr, ## __VA_ARGS__)

#define alder_loge0(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_SILENT, err, formatStr, ## __VA_ARGS__)
#define alder_loge(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_BASIC, err, formatStr, ## __VA_ARGS__)
#define alder_loge2(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_WARNING, err, formatStr, ## __VA_ARGS__)
#define alder_loge3(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_INFO, err, formatStr, ## __VA_ARGS__)
#define alder_loge4(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_FINE, err, formatStr, ## __VA_ARGS__)
#define alder_loge5(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_FINEST, err, formatStr, ## __VA_ARGS__)

#define alder_loga(desc, array, len) \
logc_logArrayBasic(MAIN_LOGGER, desc, array, len)
#define alder_loga2(desc, array, len) \
logc_logArrayWarning(MAIN_LOGGER, desc, array, len)
#define alder_loga3(desc, array, len) \
logc_logArrayInfo(MAIN_LOGGER, desc, array, len)
#define alder_loga4(desc, array, len) \
logc_logArrayFine(MAIN_LOGGER, desc, array, len)
#define alder_loga5(desc, array, len) \
logc_logArrayFinest(MAIN_LOGGER, desc, array, len)

#else

#define alder_log0(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_SILENT, formatStr, ## __VA_ARGS__)
#define alder_log(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_BASIC, formatStr, ## __VA_ARGS__)
#define alder_log2(formatStr, ...) \
logc_log(MAIN_LOGGER, LOG_WARNING, formatStr, ## __VA_ARGS__)
#define alder_log3(formatStr, ...)
#define alder_log4(formatStr, ...)
#define alder_log5(formatStr, ...)

#define alder_loge0(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_SILENT, err, formatStr, ## __VA_ARGS__)
#define alder_loge(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_BASIC, err, formatStr, ## __VA_ARGS__)
#define alder_loge2(err, formatStr, ...) \
logc_logError(ERROR_LOGGER, LOG_WARNING, err, formatStr, ## __VA_ARGS__)
#define alder_loge3(err, formatStr, ...)
#define alder_loge4(err, formatStr, ...)
#define alder_loge5(err, formatStr, ...)

#define alder_loga(desc, array, len) \
logc_logArrayBasic(MAIN_LOGGER, desc, array, len)
#define alder_loga2(desc, array, len) \
logc_logArrayWarning(MAIN_LOGGER, desc, array, len)
#define alder_loga3(desc, array, len)
#define alder_loga4(desc, array, len)
#define alder_loga5(desc, array, len)

#endif


int alder_logger_initialize (const char *mainLoggerFilename, logc_logLevel_t l);
void alder_logger_finalize ();
int alder_logger_error_initialize(logc_logLevel_t l);
void alder_logger_error_finalize();

int alder_logger_test();
int alder_logger_test01();
int alder_logger_test02();
int alder_logger_test03();
int alder_logger_test04();
int alder_logger_test05();
int alder_logger_test06();

__END_DECLS


#endif /* alder_logger_alder_logger_h */
