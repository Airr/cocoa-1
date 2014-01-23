/**
 * This file, alder_fileseq_type.h, is part of alder-fileseq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fileseq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fileseq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fileseq.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_fileseq_alder_fileseq_type_h
#define alder_fileseq_alder_fileseq_type_h

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

#define ALDER_FILETYPE_BSEQ    0x10000000

#define ALDER_FILETYPE_ISFASTA 0x11111111
#define ALDER_FILETYPE_ISFASTQ 0x22222222
#define ALDER_FILETYPE_ISSEQ   0x44444444

#define ALDER_FILETYPE_FASTA   0x00000001
#define ALDER_FILETYPE_FASTQ   0x00000002
#define ALDER_FILETYPE_SEQ     0x00000004
#define ALDER_FILETYPE_NOZ     0x00000007

#define ALDER_FILETYPE_GZFASTA 0x00000010
#define ALDER_FILETYPE_GZFASTQ 0x00000020
#define ALDER_FILETYPE_GZSEQ   0x00000040
#define ALDER_FILETYPE_GZ      0x00000070

#define ALDER_FILETYPE_BZFASTA 0x00000100
#define ALDER_FILETYPE_BZFASTQ 0x00000200
#define ALDER_FILETYPE_BZSEQ   0x00000400
#define ALDER_FILETYPE_BZ      0x00000700

#define ALDER_FILETYPE_LZFASTA 0x00001000
#define ALDER_FILETYPE_LZFASTQ 0x00002000
#define ALDER_FILETYPE_LZSEQ   0x00004000
#define ALDER_FILETYPE_LZ      0x00007000

__BEGIN_DECLS

int alder_fileseq_type(const char *fn);

__END_DECLS


#endif /* alder_fileseq_alder_fileseq_type_h */
