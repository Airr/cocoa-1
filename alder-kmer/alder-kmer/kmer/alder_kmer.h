/**
 * This file, alder_kmer.h, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef alder_kmer_alder_kmer_h
#define alder_kmer_alder_kmer_h

#include <stdint.h>
#include "alder_kmer_report.h"
#include "alder_kmer_thread.h"
#include "alder_kmer_simulate.h"
#include "alder_kmer_count.h"
#include "alder_kmer_decode.h"
#include "alder_kmer_list.h"
#include "alder_kmer_match.h"
#include "alder_kmer_binary.h"
#include "alder_kmer_uncompress.h"
#include "alder_kmer_partition.h"
#include "alder_kmer_inspect.h"
#include "alder_kmer_table.h"
#include "alder_kmer_assess.h"
#include "alder_kmer_common.h"

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

enum {
    ALDER_KMER_SUCCESS = 0,
    ALDER_KMER_ERROR = 1,
    ALDER_KMER_ERROR_MEMORY = 2,
    ALDER_KMER_ERROR_FILE = 3,
    ALDER_KMER_ERROR_FILENOTEXIST = 4
};

int
alder_kmer_test();

__END_DECLS

#endif /* alder_kmer_alder_kmer_h */
