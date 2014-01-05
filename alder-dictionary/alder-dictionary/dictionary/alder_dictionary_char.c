/**
 * This file, alder_dictionary_char.c, is part of alder-dictionary.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-dictionary is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-dictionary is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-dictionary.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include "alder_dictionary.h"
#include "alder_dictionary_char.h"

static int
compare_char(const void *c1, const void *c2)
{
    const char *a1 = c1;
    const char *a2 = c2;
    if (*a1 < *a2) {
        return -1;
    } else if (*a1 > *a2) {
        return +1;
    } else {
        return 0;
    }
}

static void
print_char (FILE *fp, const void *c)
{
    const char *a = c;
    fprintf(fp, "%c", *a);
}

void test_dictionary_char()
{
    char alphabet[27] = {
        'a','b','c','d','e','f','g','h','i','j','k','l','m',
        'n','o','p','q','r','s','t','u','v','w','x','y','z'
    };
    alder_dictionary_t *dict = alder_dictionary_create(compare_char, print_char);
    int s = alder_dictionary_insert (dict, &alphabet[0]);
    assert(s == ALDER_DICTIONARY_SUCCESS);
    s = alder_dictionary_insert (dict, &alphabet[3]);
    s = alder_dictionary_insert (dict, &alphabet[7]);
    s = alder_dictionary_insert (dict, &alphabet[2]);
    s = alder_dictionary_insert (dict, &alphabet[13]);
    s = alder_dictionary_insert (dict, &alphabet[10]);
    s = alder_dictionary_insert (dict, &alphabet[9]);
    s = alder_dictionary_insert (dict, &alphabet[6]);
    s = alder_dictionary_insert (dict, &alphabet[25]);
//    assert(s == ALDER_DICTIONARY_ERROR_DUPLICATE);
    alder_dictionary_print(dict, stdout);
    
    alder_dictionary_destroy(dict);
}

