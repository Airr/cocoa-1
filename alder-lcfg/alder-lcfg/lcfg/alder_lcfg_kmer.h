/**
 * This file, alder_lcfg_kmer.h, is part of alder-lcfg.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-lcfg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-lcfg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-lcfg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_lcfg_alder_lcfg_kmer_h
#define alder_lcfg_alder_lcfg_kmer_h

#include <stdint.h>
#include "lcfg_static.h"

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

int
alder_lcfg_kmer_test();

struct lcfg*
alder_lcfg_kmer_openForRead(const char *fn);

void
alder_lcfg_kmer_closeForRead(struct lcfg *o);



int
alder_lcfg_kmer_readKmersize(const char *fn, int *v);

int
alder_lcfg_kmer_readTotalkmer(const char *fn, uint64_t *v);

int
alder_lcfg_kmer_readNi(const char *fn, uint64_t *v);

int
alder_lcfg_kmer_readNp(const char *fn, uint64_t *v);

int
alder_lcfg_kmer_readParfilesize(const char *fn, uint64_t *v);

int
alder_lcfg_kmer_readNh(const char *fn, size_t *v);

int
alder_lcfg_kmer_readTablefilename(const char *fn, char **v, int vlen);

int
alder_lcfg_kmer_readTablefilesize(const char *fn, size_t *v);

int
alder_lcfg_kmer_readKmercount(const char *fn, size_t *v);




int
alder_lcfg_kmer_writeKmersize(const char *fn, int v);

int
alder_lcfg_kmer_writeTotalkmer(const char *fn, uint64_t v);

int
alder_lcfg_kmer_writeNi(const char *fn, uint64_t v);

int
alder_lcfg_kmer_writeNp(const char *fn, uint64_t v);

int
alder_lcfg_kmer_writeParfilesize(const char *fn, uint64_t v);

int
alder_lcfg_kmer_writeNh(const char *fn, size_t v);

int
alder_lcfg_kmer_writeTablefilename(const char *fn, const char *v);

int
alder_lcfg_kmer_writeTablefilesize(const char *fn, size_t v);

int
alder_lcfg_kmer_writeKmercount(const char *fn, size_t v);

int
alder_lcfg_kmer_openForWrite(const char *fn);

__END_DECLS


#endif /* alder_lcfg_alder_lcfg_kmer_h */
