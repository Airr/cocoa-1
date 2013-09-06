/**
 * This file, alder_logger.h, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_align_alder_logger_h
#define alder_align_alder_logger_h

#include <nglogc/log.h>

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

#define MAIN_LOGGER 0x0011
#define ERROR_LOGGER 0x0001
#define ERR_TEST 0x00000001
#define EXAMPLE_ERR 0x00000001

int alder_logger_initialize (const char *mainLoggerFilename, logc_logLevel_t l);
void alder_logger_finalize ();
int alder_logger_error_initialize(logc_logLevel_t l);
void alder_logger_error_finalize();

__END_DECLS

#endif
