/**
 * This file, alder_lcfg_kmer.c, is part of alder-lcfg.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "alder_cmacro.h"
#include "bstrlib.h"
#include "lcfg_static.h"
#include "alder_lcfg_write.h"
#include "alder_lcfg_read.h"
#include "alder_lcfg_kmer.h"

int
alder_lcfg_kmer_test()
{
    const char *fn = "kmer.cfg";
    
    alder_lcfg_kmer_writeKmersize(fn, 5);
    
    int kmersize;
    int s = alder_lcfg_kmer_readKmersize(fn, &kmersize);
    if (s != ALDER_STATUS_SUCCESS) {
        fprintf(stderr, "lcfg error: failed to find the key of kmersize.\n");
    } else {
        fprintf(stdout, "kmersize: %d\n", kmersize);
    }
    
    return 0;
}

struct lcfg*
alder_lcfg_kmer_openForRead(const char *fn)
{
    struct lcfg *cfg = lcfg_new(fn);
    if (cfg == NULL) {
        fprintf(stderr, "lcfg error: failed to open %s\n", fn);
        return NULL;
    }
    
    if( lcfg_parse(cfg) != lcfg_status_ok ) {
        fprintf(stderr, "lcfg error: %s\n", lcfg_error_get(cfg));
        return NULL;
        
    }
    return cfg;
}

void
alder_lcfg_kmer_closeForRead(struct lcfg *o)
{
    lcfg_delete(o);
}

int
alder_lcfg_kmer_readKmersize(const char *fn, int *v)
{
    return alder_lcfg_base_read_int(fn, "kmersize", v);
}


int
alder_lcfg_kmer_readTotalkmer(const char *fn, uint64_t *v)
{
    return alder_lcfg_base_read_uint64(fn, "totalkmer", v);
}

int
alder_lcfg_kmer_readNi(const char *fn, uint64_t *v)
{
    return alder_lcfg_base_read_uint64(fn, "ni", v);
}

int
alder_lcfg_kmer_readNp(const char *fn, uint64_t *v)
{
    return alder_lcfg_base_read_uint64(fn, "np", v);
}

int
alder_lcfg_kmer_readParfilesize(const char *fn, uint64_t *v)
{
    return alder_lcfg_base_read_uint64(fn, "parfilesize", v);
}

int
alder_lcfg_kmer_readNh(const char *fn, size_t *v)
{
    return alder_lcfg_base_read_size_t(fn, "nh", v);
}

int
alder_lcfg_kmer_readTablefilename(const char *fn, char **v, int vlen)
{
    return alder_lcfg_base_read_char(fn, "tabfilename", v, vlen);
}

int
alder_lcfg_kmer_readTablefilesize(const char *fn, size_t *v)
{
    return alder_lcfg_base_read_size_t(fn, "tabfilesize", v);
}

int
alder_lcfg_kmer_readKmercount(const char *fn, size_t *v)
{
    return alder_lcfg_base_read_size_t(fn, "kmercount", v);
}










int
alder_lcfg_kmer_writeKmersize(const char *fn, int v)
{
    return alder_lcfg_base_write_int(fn, "kmersize", v);
}

int
alder_lcfg_kmer_writeTotalkmer(const char *fn, uint64_t v)
{
    return alder_lcfg_base_write_uint64(fn, "totalkmer", v);
}

int
alder_lcfg_kmer_writeNi(const char *fn, uint64_t v)
{
    return alder_lcfg_base_write_uint64(fn, "ni", v);
}

int
alder_lcfg_kmer_writeNp(const char *fn, uint64_t v)
{
    return alder_lcfg_base_write_uint64(fn, "np", v);
}

int
alder_lcfg_kmer_writeParfilesize(const char *fn, uint64_t v)
{
    return alder_lcfg_base_write_uint64(fn, "parfilesize", v);
}

int
alder_lcfg_kmer_writeNh(const char *fn, size_t v)
{
    return alder_lcfg_base_write_size_t(fn, "nh", v);
}

int
alder_lcfg_kmer_writeTablefilename(const char *fn, const char *v)
{
    return alder_lcfg_base_write_char(fn, "tabfilename", v);
}

int
alder_lcfg_kmer_writeTablefilesize(const char *fn, size_t v)
{
    return alder_lcfg_base_write_size_t(fn, "tabfilesize", v);
}

int
alder_lcfg_kmer_writeKmercount(const char *fn, size_t v)
{
    return alder_lcfg_base_write_size_t(fn, "kmercount", v);
}

int
alder_lcfg_kmer_openForWrite(const char *fn)
{
    FILE *fp = fopen(fn, "w");
    XFCLOSE(fp);
    return 0;
}
