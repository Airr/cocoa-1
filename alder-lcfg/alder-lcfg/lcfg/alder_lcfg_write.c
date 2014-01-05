/**
 * This file, alder_lcfg_write.c, is part of alder-kmer.
 *
 * Copyright 2014 by Sang Chul Choi
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_lcfg_write.h"

int
alder_lcfg_base_write_uint64(const char *fn,  const char *key, uint64_t v)
{
    FILE *fp = fopen(fn, "a+");
    if (fp == NULL) {
        return ALDER_STATUS_ERROR;
    }
    fprintf(fp, "%s = \"%llu\"\n", key, v);
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_write_int(const char *fn,  const char *key, int v)
{
    FILE *fp = fopen(fn, "a+");
    if (fp == NULL) {
        return ALDER_STATUS_ERROR;
    }
    fprintf(fp, "%s = \"%d\"\n", key, v);
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_write_size_t(const char *fn,  const char *key, size_t v)
{
    FILE *fp = fopen(fn, "a+");
    if (fp == NULL) {
        return ALDER_STATUS_ERROR;
    }
    fprintf(fp, "%s = \"%zu\"\n", key, v);
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_write_char(const char *fn,  const char *key, const char *v)
{
    FILE *fp = fopen(fn, "a+");
    if (fp == NULL) {
        return ALDER_STATUS_ERROR;
    }
    fprintf(fp, "%s = \"%s\"\n", key, v);
    XFCLOSE(fp);
    return ALDER_STATUS_SUCCESS;
}