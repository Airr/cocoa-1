/**
 * This file, alder_thread.h, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef alder_thread_alder_thread_h
#define alder_thread_alder_thread_h

#include "alder_thread_worker.h"
#include "alder_thread_argument.h"
#include "alder_thread_join.h"
#include "alder_thread_stack.h"
#include "alder_thread_readwriter.h"
#include "alder_thread_casn.h"
#include "alder_thread_mcas.h"
#include "alder_thread_sundell.h"

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


__END_DECLS

#endif /* alder_thread_alder_thread_h */
