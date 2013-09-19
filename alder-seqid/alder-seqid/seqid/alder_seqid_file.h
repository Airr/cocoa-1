/**
 * This file, alder_seqid_file.h, is part of alder-seqid.
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

#ifndef alder_seqid_alder_seqid_file_h
#define alder_seqid_alder_seqid_file_h

#include "bstrlib.h"

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

/**
 * I need a list of file names in the order of pair-end and single-end.
 * Files can be FASTA, or FASTQ. It could be something else as long as 
 * files contain sequence name and sequence data. I allow file formats 
 * including text, gzip, bzip, zip, tar, 7-zip, ... maybe by using
 * libarchive. For now, I will just use text and gzip till I know how to 
 * use libarchive.
 *
 * filename - file names in the order of pair-end and single-end data parts
 * type - file types. 1 (text), 2 (gzip).
 * format - file format. 1 (FASTA), 2 (FASTQ).
 * pair - pair ID.
 */
typedef struct {
    struct bstrList *filename;
    int             *type;
    int             *format;
    int             *pair;
} alder_seqid_file_t;

alder_seqid_file_t * alder_seqid_file_init(struct bstrList *fn);
void alder_seqid_file_fprintf(FILE * stream, alder_seqid_file_t *asf);
void alder_seqid_file_free(alder_seqid_file_t *asf);
                           
__END_DECLS


#endif /* alder_seqid_alder_seqid_file_h */
