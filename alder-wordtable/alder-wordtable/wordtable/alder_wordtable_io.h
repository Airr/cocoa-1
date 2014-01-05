/**
 * This file, alder_wordtable_io.h, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_wordtable_alder_wordtable_io_h
#define alder_wordtable_alder_wordtable_io_h

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

#define ALDER_WORDTABLE_IO_CRC_INIT 0xFFFFFFFF

int
alder_wordtable_io_write_header(FILE *fp);
int
alder_wordtable_io_read_header(FILE *fp);

int
alder_wordtable_io_write_IHDR(FILE *fp,
                              size_t kfm_order,
                              size_t kfm_size,
                              size_t kfm_rhosize,
                              size_t kfm_stepsize);

int
alder_wordtable_io_read_IHDR(FILE *fp,
                             size_t *kfm_order,
                             size_t *kfm_size,
                             size_t *kfm_rhosize,
                             size_t *kfm_stepsize);

int
alder_wordtable_io_write_IKFM(FILE *fp, uint64_t lengthChunk, uint32_t *crc);

int
alder_wordtable_io_read_IKFM(FILE *fp, uint64_t *lengthChunk, uint32_t *crc);

int
alder_wordtable_io_write_IRHO(FILE *fp, uint64_t lengthChunk, uint32_t *crc);

int
alder_wordtable_io_read_IRHO(FILE *fp, uint64_t *lengthChunk, uint32_t *crc);

int
alder_wordtable_io_write_IEND(FILE *fp);

int
alder_wordtable_io_read_IEND(FILE *fp);

int
alder_wordtable_io_write_uint8(FILE *fp, uint8_t v, uint32_t *crc);

int
alder_wordtable_io_read_uint8(FILE *fp, uint8_t *v, uint32_t *crc);

int
alder_wordtable_io_write_uint16(FILE *fp, uint16_t v, uint32_t *crc);

int
alder_wordtable_io_read_uint16(FILE *fp, uint16_t *v, uint32_t *crc);

int
alder_wordtable_io_write_uint32(FILE *fp, uint32_t v, uint32_t *crc);

int
alder_wordtable_io_read_uint32(FILE *fp, uint32_t *v, uint32_t *crc);

int
alder_wordtable_io_write_uint64(FILE *fp, uint64_t v, uint32_t *crc);

int
alder_wordtable_io_read_uint64(FILE *fp, uint64_t *v, uint32_t *crc);

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_io_h */
