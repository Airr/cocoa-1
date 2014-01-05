/**
 * This file, alder_kmer_report.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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
#include "alder_hashtable_mcas.h"
#include "alder_kmer_report.h"

int alder_kmer_report(const char *fn,
                      int isSummary,
                      const char *query)
{
    int s = 0;
    if (query == NULL) {
        alder_hashtable_mcas_load(fn, isSummary);
    } else {
        alder_hashtable_mcas_query(fn, query);
    }
    
    return s;
}