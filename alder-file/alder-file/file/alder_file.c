/**
 * This file, alder_file.c, is part of alder-file.
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

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "alder_file.h"

void alder_file(const char *fn)
{
    int s = alder_file_creatable(fn);
    if (s == 0) {
        printf("  is creatable.\n");
    } else if (s == -1) {
        printf("  is not creatable.\n");
    } else {
        assert(0);
    }
    
    struct stat fileStat;
    if(stat(fn, &fileStat) < 0) {
        printf("No information for %s\n", fn);
        return;
    }
    
    printf("Information for %s\n", fn);
    printf("---------------------------\n");
    printf("File Size: \t\t%lld bytes\n", fileStat.st_size);
    printf("Number of Links: \t%d\n", fileStat.st_nlink);
    printf("File inode: \t\t%lld\n", fileStat.st_ino);
    
    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
    
    printf("The file %s a symbolic link\n", (S_ISLNK(fileStat.st_mode)) ? "is" : "is not");
    
    s = alder_file_exist(fn);
    if (s == 0) {
        printf("  exist.\n");
    } else if (s == -1) {
        printf("  does not exist.\n");
    } else {
        assert(0);
    }
    
    s = alder_file_writable(fn);
    if (s == 0) {
        printf("  writable.\n");
    } else if (s < 0) {
        printf("  is not writable.\n");
    } else {
        assert(0);
    }
    
    
    
}

