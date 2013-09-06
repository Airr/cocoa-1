/**
 * This file, alder_adapter_cut_file2.h, is part of alder-adapter.
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

#ifndef alder_adapter_alder_adapter_cut_file2_h
#define alder_adapter_alder_adapter_cut_file2_h

#include "alder_fastq_stat.h"
#include "alder_adapter_option.h"

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

int alder_adapter_cut_file2(const char *fnin, const char *fnout,
                            const char *fnin2, const char *fnout2,
                            const char *adapter_seq, const char *adapter_seq2,
                            const alder_adapter_option_t *option,
                            alder_fastq_stat_t *stat);

__END_DECLS


#endif /* alder_adapter_alder_adapter_cut_file2_h */
