/**
 * This file, alder_dna.c, is part of alder-dna.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-dna is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-dna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-dna.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_dna.h"

/* This function converts a number to a character.
 * returns a char value given an int value.
 * 0 -> A
 * 1 -> C
 * 2 -> T
 * 3 -> G
 */
inline char alder_dna_int2char(int i)
{
    switch (i) {
        case 0:
            return 'A';
            break;
        case 1:
            return 'C';
            break;
        case 2:
            return 'T';
            break;
        case 3:
            return 'G';
            break;
        default:
            return 'X';
            break;
    }
}

inline int alder_dna_char2int(char c)
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
        case 'T':
        case 't':
            return 2;
            break;
        case 'G':
        case 'g':
            return 3;
            break;
        default:
            return 4;
            break;
    }
}