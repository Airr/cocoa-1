/**
 * This file, alder_snippet-forloop.c, is part of alder-snippet.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-snippet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-snippet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-snippet.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_snippet_forloop.h"

int
alder_snippet_forloop()
{
    int x = 0;
    for (int i = 2; i-- > 0; ) {
        x++;
        printf("[%d] ", i);
    }
    printf(" - x = %d\n", x);
    assert(x == 2);
    
    for (int i = 1; ++i < 5;) {
        printf("[%d] ", i);
    }
    printf("\n");
    for (int i = 1 + 1; i < 5; i++) {
        printf("[%d] ", i);
    }
    printf("\n");
    return 0;
}
