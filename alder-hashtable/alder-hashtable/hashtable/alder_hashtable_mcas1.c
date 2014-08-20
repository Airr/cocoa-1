/**
 * This file, alder_hashtable_mcas1.c, is part of alder-kmer.
 *
 * Copyright 2013, 2014 by Sang Chul Choi
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

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "alder_cmacro.h"
#include "alder_logger.h"
#include "libdivide.h"
#include "alder_hash.h"
#include "alder_rng.h"
#include "alder_mcas_wf.h"
#include "alder_encode.h"
#include "libdivide.h"
#include "alder_hashtable_mcas1.h"
