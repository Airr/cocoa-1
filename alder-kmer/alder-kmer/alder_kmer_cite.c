/**
 * This file, alder_kmer_cite.c, is part of alder-kmer.
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
#include "alder_kmer_cite.h" 

static const char *alder_kmer_cite_description =
"Rizk, Lavenier and Chikhi. 2013 Bioinformatics 29:652-653.\n"
"  DSK: k-mer counting with very low memory usage.\n"
"Marcais and Kingsford. 2011 Bioinformatics 27:764-770.\n"
"  A fast, lock-free approach for efficient parallel counting of occurrences of k-mers.";

/**
 *  This function prints the references of techniques used in alder-kmer.
 */
void alder_kmer_cite()
{
    printf("%s\n", alder_kmer_cite_description);
}

