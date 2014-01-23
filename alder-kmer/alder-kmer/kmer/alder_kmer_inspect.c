/**
 * This file, alder_kmer_inspect.c, is part of alder-kmer.
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
#include "alder_file_io.h"
#include "alder_kmer_inspect.h"

/**
 *  This function inspects the computer system to optimize alder-kmer's runs
 *  on it.
 *
 *  @param infile  input files
 *  @param outdir  output directory
 *  @param outfile output file prefix
 */
void alder_kmer_inspect(struct bstrList *infile,
                        const char *outdir,
                        const char *outfile)
{
    int s;
    // Read files test.
    bstring bfn = bformat("%s/%s.ins", outdir, outfile);
//    size_t s = alder_file_io_best_write_buffer_size(bdata(bfn));
//    printf("Write buffer size in log2: %zu\n", s);
    
    size_t write_log2size = 0;
    size_t read_log2size = 0;
    
    s = alder_file_io_best_buffer_size(bdata(bfn),
                                       &write_log2size,
                                       &read_log2size);
    
    if (write_log2size < read_log2size) {
        read_log2size = write_log2size;
    }
    printf("Buffer size in log2: %zu (read), %zu (write)\n",
           read_log2size, write_log2size);
    
    bdestroy(bfn);
}

