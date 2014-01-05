/**
 * This file, alder_file_availableopenfiles.c, is part of alder-file.
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
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <string.h>
#include "alder_file_availableopenfiles.h"

/* This function 
 *
 * returns 
 * 0 on success
 * 1 if an error occurs.
 
 Check sysctl function.
 KERN_MAXFILES
 The maximum number of files that may be open in the system.
 
 KERN_MAXFILESPERPROC
 The maximum number of files that may be open for a single process.  This limit only applies to
 processes with an effective uid of nonzero at the time of the open request.  Files that have
 already been opened are not affected if the limit or the effective uid is changed.
 */
int alder_file_availableopenfiles(int64_t *n_file)
{
    int numMaxFiles = 0;
    int mib[4];
    size_t len = sizeof(numMaxFiles);
    
    /* set the mib for hw.ncpu */
    mib[0] = CTL_KERN;
    mib[1] = KERN_MAXFILESPERPROC;  // alternatively, try HW_NCPU;
    
    /* get the number of CPUs from the system */
    int s = sysctl(mib, 2, &numMaxFiles, &len, NULL, 0);
    if( s == -1 )
    {
        fprintf(stderr, "Error - retrieving max open files failed : %s\n",
                strerror(errno));
        return 1;
    }
    
    *n_file = numMaxFiles;
    return 0;
}