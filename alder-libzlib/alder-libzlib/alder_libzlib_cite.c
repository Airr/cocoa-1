/**
 * This file, alder_libzlib_cite.c, is part of alder-libzlib.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-libzlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-libzlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-libzlib.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_libzlib_cite.h" 

static const char *alder_libzlib_cite_description = "Author et al 2013.";

void alder_libzlib_cite()
{
    printf("%s\n", alder_libzlib_cite_description);
}

