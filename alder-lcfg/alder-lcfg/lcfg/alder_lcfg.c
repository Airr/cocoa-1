/**
 * This file, alder_lcfg.c, is part of alder-lcfg.
 *
 * Copyright 2014 by Sang Chul Choi
 *
 * alder-lcfg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-lcfg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-lcfg.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <ctype.h>

#include "lcfg_static.h"

enum lcfg_status example_visitor(const char *key, void *data, size_t len, void *user_data) {
    int i;
    char c;
    
    printf("%s = \"", key);
    for( i = 0; i < len; i++ ) {
        c = *((const char *)(data + i));
        printf("%c", isprint(c) ? c : '.');
    }
    puts("\"");
    
    return lcfg_status_ok;
}


int alder_lcfg_test() {
//    if( argc != 2 ) {
//        printf("usage: %s FILE\n", argv[0]);
//        return -1;
//    }
    
    struct lcfg *c = lcfg_new("/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-lcfg/alder-lcfg/lcfg/test.cfg");
    
    if( lcfg_parse(c) != lcfg_status_ok ) {
        printf("lcfg error: %s\n", lcfg_error_get(c));
    } else {
        lcfg_accept(c, example_visitor, 0);
    }
    
    lcfg_delete(c);
    
    return 0;
}
