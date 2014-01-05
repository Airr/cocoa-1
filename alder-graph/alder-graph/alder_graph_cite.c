/**
 * This file, alder_graph_cite.c, is part of alder-graph.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-graph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-graph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-graph.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "alder_graph_cite.h" 

static const char *alder_graph_cite_description = "Author et al 2013.";

void alder_graph_cite()
{
    printf("%s\n", alder_graph_cite_description);
}

