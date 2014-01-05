/**
 * This file, alder_type_gidenstam.h, is part of alder-type.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-type is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-type is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-type.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_type_alder_type_gidenstam_h
#define alder_type_alder_type_gidenstam_h

#include <stdint.h>
#include <stdbool.h>

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

#define NR_LINKS_NODE 10

typedef uint64_t wrd_t;

typedef struct alder_type_gidenstam_entry_struct {
    wrd_t *address;
    wrd_t old;
    wrd_t new;
    wrd_t *mch;
} alder_type_gidenstam_entry_t;

typedef struct alder_type_gidenstam_descriptor_struct {
    int mm_ref;
    bool mm_trace;
    bool mm_del;
    int size;
    alder_type_gidenstam_entry_t *words;
    struct alder_type_gidenstam_descriptor_struct **link;
} alder_type_gidenstam_descriptor_t;


__END_DECLS


#endif /* alder_type_alder_type_gidenstam_h */
