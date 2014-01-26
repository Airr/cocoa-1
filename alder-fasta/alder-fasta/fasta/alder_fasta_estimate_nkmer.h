/**
 * This file, alder_fastq_estimate_nkmer.h, is part of alder-fasta.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_fasta_alder_fastq_estimate_nkmer_h
#define alder_fasta_alder_fastq_estimate_nkmer_h

#include <stdint.h>

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

int alder_fasta_estimate_nkmer(uint64_t *v,
                               int kmer_size,
                               const char *fn);

__END_DECLS


#endif /* alder_fasta_alder_fastq_estimate_nkmer_h */
