/**
 * This file, alder_hey_cite.c, is part of alder-hey.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-hey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-hey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-hey.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_hey_cite.h" 

static const char *alder_hey_cite_description = "Author et al 2013.";

void alder_hey_cite()
{
    printf("%s\n", alder_hey_cite_description);
}

