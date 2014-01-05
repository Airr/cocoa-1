/**
 * This file, alder_file_sysctl.c, is part of alder-file.
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
#include <sys/types.h>
#include <sys/sysctl.h>
#include "alder_file_sysctl.h"

/* This function finds the number of CPU cores.
 * ref. http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
 * 
 * returns
 * the number of CPUs
 */
int alder_file_sysctl()
{
    int numCPU;
    int mib[4];
    size_t len = sizeof(numCPU);
    
    /* set the mib for hw.ncpu */
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
    
    /* get the number of CPUs from the system */
    sysctl(mib, 2, &numCPU, &len, NULL, 0);
    
    if( numCPU < 1 )
    {
        mib[1] = HW_NCPU;
        sysctl( mib, 2, &numCPU, &len, NULL, 0 );
        
        if( numCPU < 1 )
        {
            numCPU = 1;
        }
    }
    
    return numCPU;
}