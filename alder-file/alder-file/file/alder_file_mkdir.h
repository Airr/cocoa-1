/**
 * This file, alder_file_mkdir.h, is part of alder-file.
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

#ifndef alder_file_alder_file_mkdir_h
#define alder_file_alder_file_mkdir_h

#include <sys/types.h>
#include <sys/stat.h>
#define ALDER_FILE_DEFAULT_MODE S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH
// From `man 2 chmod`
//#define S_IRWXU 0000700    /* RWX mask for owner */
//#define S_IRUSR 0000400    /* R for owner */
//#define S_IWUSR 0000200    /* W for owner */
//#define S_IXUSR 0000100    /* X for owner */
//
//#define S_IRWXG 0000070    /* RWX mask for group */
//#define S_IRGRP 0000040    /* R for group */
//#define S_IWGRP 0000020    /* W for group */
//#define S_IXGRP 0000010    /* X for group */
//
//#define S_IRWXO 0000007    /* RWX mask for other */
//#define S_IROTH 0000004    /* R for other */
//#define S_IWOTH 0000002    /* W for other */
//#define S_IXOTH 0000001    /* X for other */

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

int alder_file_mkdir(const char *path);

__END_DECLS


#endif /* alder_file_alder_file_mkdir_h */
