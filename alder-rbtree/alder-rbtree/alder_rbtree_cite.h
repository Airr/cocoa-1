/**
 * This file, alder_rbtree_cite.h, is part of alder-rbtree.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-rbtree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-rbtree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-rbtree.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_rbtree_alder_rbtree_cite_h
#define alder_rbtree_alder_rbtree_cite_h

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

void alder_rbtree_cite();

__END_DECLS


#endif /* alder_rbtree_alder_rbtree_cite_h */
