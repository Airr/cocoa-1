/**
 * This file, main.c, is part of alder-uf.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-uf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-uf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-uf.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"
#include "alder_uf.h"
#include "alder_uf_option.h" 

int main(int argc, char * argv[])
{
    int isExitWithHelp = 0;
    alder_uf_option_t option;
    struct gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    isExitWithHelp = alder_uf_option_init(&option, &args_info);
    if (isExitWithHelp == 1) {
        my_cmdline_parser_free(&args_info);
        alder_uf_option_free(&option);
        exit(1);
    }

    alder_uf_t *uf;
    /* Test: creats and destroys a union-find type variable. */
//    uf = alder_uf_create();
//    alder_uf_destroy(uf);

    /* Test: makes a new set with an element. */
    uf = alder_uf_create();
    alder_uf_makeset(uf, 1);
    alder_uf_makeset(uf, 2);
    alder_uf_makeset(uf, 3);
    alder_uf_makeset(uf, 4);
    alder_uf_makeset(uf, 5);
    alder_uf_makeset(uf, 6);
    alder_uf_makeset(uf, 7);
    
    alder_uf_makeset(uf, 11);
    alder_uf_makeset(uf, 12);
    alder_uf_makeset(uf, 13);
    alder_uf_makeset(uf, 14);
    alder_uf_makeset(uf, 15);
    alder_uf_makeset(uf, 16);
    alder_uf_makeset(uf, 17);
    alder_uf_union(uf, 11, 12);
    alder_uf_union(uf, 11, 13);
    alder_uf_union(uf, 11, 14);
    alder_uf_union(uf, 11, 15);
    alder_uf_union(uf, 11, 16);
    alder_uf_union(uf, 11, 17);
    
    alder_uf_union(uf, 1, 2);
    alder_uf_union(uf, 3, 4);
    
    uf_node_t *uf_node = alder_uf_enumerate(uf, 1);
    alder_uf_printSet(uf, stdout);
    uf_node = alder_uf_enumerate(uf, 3);
    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 4);
    uf_node = alder_uf_enumerate(uf, 2);
    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 5, 6);
    alder_uf_union(uf, 5, 7);
    uf_node = alder_uf_enumerate(uf, 5);
    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 5);
    uf_node = alder_uf_enumerate(uf, 2);
    alder_uf_printSet(uf, stdout);
    
    alder_uf_union(uf, 2, 11);
    uf_node = alder_uf_enumerate(uf, 2);
    alder_uf_printSet(uf, stdout);
    
    alder_uf_destroy(uf);

    /* Test: takes the union of two sets. */
//    uf = alder_uf_create();
//    alder_uf_makeset(uf, 1);
//    alder_uf_makeset(uf, 2);
//    alder_uf_union(uf, 1, 2);
//    alder_uf_destroy(uf);

    /* Test: finds the set that contains an element.  */
//    uf = alder_uf_create();
//    alder_uf_makeset(uf, 1);
//    int i = alder_uf_find(uf, 1);
//    alder_uf_destroy(uf);




    
    my_cmdline_parser_free(&args_info);
    alder_uf_option_free(&option);
    return 0;
}

