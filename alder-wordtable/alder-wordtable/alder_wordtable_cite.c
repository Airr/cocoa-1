/**
 * This file, alder_wordtable_cite.c, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_wordtable_cite.h" 

static const char *alder_wordtable_cite_description =
"Rodland. 2013 BMC Bioinformatics 14:313.\n"
"  Compact representation of k-mer de Bruijn graphs for genome read assembly.";

void alder_wordtable_cite()
{
    printf("%s\n", alder_wordtable_cite_description);
}

