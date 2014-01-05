/**
 * This file, alder_wordtable_tokenset.c, is part of alder-wordtable.
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
#include <stdlib.h>
#include <assert.h>
#include "alder_cmacro.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_tokenset.h"

/* This function returns the size of \Sigma or 4 for DNA characters.
 */
size_t
alder_wordtable_tokenset_size()
{
    return ALDER_WORDTABLE_TOKENSIZE;
}

/* This function returns a complementary token in integer of a given token.
 * A <-> T or 0 <-> 3
 * G <-> C or 2 <-> 1
 */
int
alder_wordtable_tokenset_reverse(int token)
{
    switch (token) {
        case 0:
            return 3;
            break;
        case 1:
            return 2;
            break;
        case 2:
            return 1;
            break;
        case 3:
            return 0;
            break;
        default:
            return ALDER_WORDTABLE_TOKENSIZE;
            break;
    }
}

/* This function returns the token corresponding to a character.
 * A or a ->  0
 * C or c ->  1
 * G or g ->  2
 * T or t ->  3
 * others -> -1
 */
int
alder_wordtable_tokenset_token_of(char c)
{
    switch (c) {
        case 'A':
        case 'a':
            return 0;
            break;
        case 'C':
        case 'c':
            return 1;
            break;
        case 'G':
        case 'g':
            return 2;
            break;
        case 'T':
        case 't':
            return 3;
            break;
        default:
            return -1;
            break;
    }
}

/* This function returns the character corresponding to an integer token.
 * 0 -> A
 * 1 -> C
 * 2 -> G
 * 3 -> T
 * others -> $
 */
char
alder_wordtable_tokenset_char_of(int token)
{
    switch (token) {
        case 0:
            return 'A';
            break;
        case 1:
            return 'C';
            break;
        case 2:
            return 'G';
            break;
        case 3:
            return 'T';
            break;
        default:
            return '$';
            break;
    }
}
