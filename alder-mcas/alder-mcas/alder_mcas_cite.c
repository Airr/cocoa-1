/**
 * This file, alder_mcas_cite.c, is part of alder-mcas.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-mcas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-mcas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-mcas.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_mcas_cite.h" 

static const char *alder_mcas_cite_description = "Author et al 2013.";

void alder_mcas_cite()
{
    printf("%s\n", alder_mcas_cite_description);
}

