/**
 * This file, alder_file_stat.c, is part of alder-adapter.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-adapter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-adapter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-adapter.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/stat.h>
#include "alder_fastq_stat.h"

//  0 : exist
// -1 : not exist
int alder_file_exist(const char *fn)
{
    struct stat buf;
    int status = stat(fn, &buf);
    return status;
}
