/**
 * This file, alder_lcfg_read.c, is part of alder-kmer.
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
#include "bstrlib.h"
#include "lcfg_static.h"
#include "alder_lcfg_read.h"

struct lcfg*
alder_lcfg_base_read_open(const char *fn)
{
    struct lcfg *cfg = lcfg_new(fn);
    if (cfg == NULL) {
        fprintf(stderr, "lcfg error: failed to open %s\n", fn);
        return NULL;
    }
    
    if( lcfg_parse(cfg) != lcfg_status_ok ) {
        fprintf(stderr, "lcfg error: %s\n", lcfg_error_get(cfg));
        lcfg_delete(cfg);
        return NULL;
    }

    return cfg;
}

int
alder_lcfg_base_read_uint64(const char *fn,  const char *key, uint64_t *v)
{
    struct lcfg *cfg = alder_lcfg_base_read_open(fn);
    
    enum lcfg_status s;
    size_t len;
    void *data;
    s = lcfg_value_get(cfg, key, &data, &len);
    if (s != lcfg_status_ok) {
        return ALDER_STATUS_ERROR;
    }
    bstring bv = bfromcstralloc((int)len, "");
    for (int i = 0; i < len; i++) {
        bconchar(bv, *((const char *)(data + i)));
    }
    *v = (uint64_t)strtoll((const char*)bv->data, NULL, 10);
    bdestroy(bv);
    
    lcfg_delete(cfg);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_read_int(const char *fn,  const char *key, int *v)
{
    struct lcfg *cfg = alder_lcfg_base_read_open(fn);
    
    enum lcfg_status s;
    size_t len;
    void *data;
    s = lcfg_value_get(cfg, key, &data, &len);
    if (s != lcfg_status_ok) {
        return ALDER_STATUS_ERROR;
    }
    bstring bv = bfromcstralloc((int)len, "");
    for (int i = 0; i < len; i++) {
        bconchar(bv, *((const char *)(data + i)));
    }
    *v = (int)strtol((const char*)bv->data, NULL, 10);
    bdestroy(bv);
    
    lcfg_delete(cfg);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_read_size_t(const char *fn,  const char *key, size_t *v)
{
    struct lcfg *cfg = alder_lcfg_base_read_open(fn);
    
    enum lcfg_status s;
    size_t len;
    void *data;
    s = lcfg_value_get(cfg, key, &data, &len);
    if (s != lcfg_status_ok) {
        return ALDER_STATUS_ERROR;
    }
    bstring bv = bfromcstralloc((int)len, "");
    for (int i = 0; i < len; i++) {
        bconchar(bv, *((const char *)(data + i)));
    }
    *v = (size_t)strtoll((const char*)bv->data, NULL, 10);
    bdestroy(bv);
    
    lcfg_delete(cfg);
    return ALDER_STATUS_SUCCESS;
}

int
alder_lcfg_base_read_char(const char *fn,  const char *key, char **v, int vlen)
{
    struct lcfg *cfg = alder_lcfg_base_read_open(fn);
    
    enum lcfg_status s;
    size_t len;
    void *data;
    s = lcfg_value_get(cfg, key, &data, &len);
    if (s != lcfg_status_ok) {
        return ALDER_STATUS_ERROR;
    }
    bstring bv = bfromcstralloc((int)len, "");
    for (int i = 0; i < len; i++) {
        bconchar(bv, *((const char *)(data + i)));
    }
    int slen = (int)len < vlen - 1 ? (int)len : vlen - 1;
    strncpy(*v, (char*)bv->data, slen);
    (*v)[slen] = '\0';
    bdestroy(bv);
    
    lcfg_delete(cfg);
    return ALDER_STATUS_SUCCESS;
}


