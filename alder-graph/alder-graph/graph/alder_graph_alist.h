/**
 * This file, alder_alder_graph_alist_alist.h, is part of alder-graph.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-graph is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-graph is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-graph.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef alder_alder_graph_alist_alder_alder_graph_alist_alist_h
#define alder_alder_graph_alist_alder_alder_graph_alist_alist_h

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

int
alder_graph_alist_test();

typedef struct alder_graph_alist_struct *alder_graph_alist_t;

alder_graph_alist_t
alder_graph_alist_create(int n);

void alder_graph_alist_destroy(alder_graph_alist_t);

void alder_graph_alist_set_s(alder_graph_alist_t g, int u, bstring s);
void alder_graph_alist_add_edge(alder_graph_alist_t, int source, int sink);

int alder_graph_alist_vertex_count(alder_graph_alist_t);
int alder_graph_alist_edge_count(alder_graph_alist_t);
int alder_graph_alist_out_degree(alder_graph_alist_t, int source);
int alder_graph_alist_has_edge(alder_graph_alist_t, int source, int sink);

void alder_graph_alist_foreach
(alder_graph_alist_t g, int source,
 void (*f)(alder_graph_alist_t g, int source, int sink, void *data),
 void *data);

__END_DECLS


#endif /* alder_alder_graph_alist_alder_alder_graph_alist_alist_h */
