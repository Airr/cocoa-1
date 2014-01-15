/**
 * This file, alder_file_isstdin.c, is part of alder-file.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-file.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "alder_cmacro.h"
#include "alder_file_isstdin.h"

/* This function tests if there is any standard input to the program.
 * returns
 * 0 if no standard input, otherwise non-zero.
 */
/**
 *  This function checks if there is any standard input.
 *  WARN: You must use fildes to read the standard input since calling this
 *        function.
 *
 *  @return YES or NO.
 */
int alder_file_isstdin()
{
    int timeout = 100; // wait 100ms
    struct pollfd fd;
    fd.fd = 0;
    fd.events = POLLIN;
    fd.revents = 0;
    int s = poll(&fd, 1, timeout);
    if (s > 0 && ((fd.revents & POLLIN) != 0))  {
        // got some data
        s = ALDER_YES;
    } else {
        // check for error
        s = ALDER_NO;
    }
    
    return s;
}

/**
 *  This function checks if there is any standard input. 
 *  WARN: You must use FILE* to read the standard input since calling this
 *        function.
 *
 *  @return YES or NO.
 */
int alder_file_isstdin_FILE()
{
    int ret = ALDER_YES;
    int c = getchar();
    if (ferror(stdin)) {
        ret = ALDER_NO;
    } else {
        ungetc(c, stdin);
        ret = ALDER_YES;
    }
    return ret;
}
