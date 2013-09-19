/**
 * This file, alder_fastq_length.h, is part of alder-seqid.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-seqid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-seqid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-seqid.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_seqid_alder_fastq_length_h
#define alder_seqid_alder_fastq_length_h

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

int alder_fastq_length(const char* fn, int64_t *n1, int64_t *n2);
int alder_fastq_length_gzip(const char* fn, int64_t *n1, int64_t *n2);
int alder_fastq_length_file(const char* fn, int64_t *n1, int64_t *n2);

__END_DECLS


#endif /* alder_seqid_alder_fastq_length_h */
