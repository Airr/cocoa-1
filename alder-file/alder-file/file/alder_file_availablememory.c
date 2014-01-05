/**
 * This file, alder_file_availablememory.c, is part of alder-file.
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

#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include "alder_file_availablememory.h"

int alder_file_memory_test()
{
    int64_t free_memory;
    int64_t used_memory;
    int r = alder_file_availablememory2(&free_memory, &used_memory);
    if (r == 0) {
        fprintf(stdout, "Free memory: %llu (MB)\n", free_memory >> 20);
        fprintf(stdout, "Used memory: %llu (MB)\n", used_memory >> 20);
    } else {
        fprintf(stderr, "Error: failed to get memory info\n");
    }
    return 0;
}

/* This function finds the physical memory in bytes.
 */
int alder_file_availablememory(int64_t *physical_memory)
{
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    size_t length = sizeof(int64_t);
    sysctl(mib, 2, physical_memory, &length, NULL, 0);
    return 0;
}

#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>

/* This function returns values of currently used and free memory in bytes.
 * It looks like 
 * http://sg80bab.blogspot.com/2007/03/is-my-mac-using-too-much-memory.html
 */
int alder_file_availablememory2(int64_t *free_memory, int64_t *used_memory)
{
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics_data_t vm_stats;
    
    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO,
                                        (host_info_t)&vm_stats, &count))
    {
//        *free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;
        *free_memory = (((int64_t)vm_stats.free_count +
                         (int64_t)vm_stats.inactive_count) * (int64_t)page_size);
        
        *used_memory = ((int64_t)vm_stats.active_count +
                       (int64_t)vm_stats.inactive_count +
                       (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
        return 0;
    } else {
        return 1;
    }
}
