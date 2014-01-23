/**
 * This file, alder_file_io.h, is part of alder-file.
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

#ifndef alder_file_alder_file_io_h
#define alder_file_alder_file_io_h

#include <stdint.h>

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

void alder_file_io_test();
void alder_file_io_test01();
void alder_file_io_test02();
void alder_file_io_test03(const char *fn);

int alder_file_io_best_buffer_size(const char *fn,
                                   size_t *best_write_buffer_size,
                                   size_t *best_read_buffer_size);

__END_DECLS


#endif /* alder_file_alder_file_io_h */
