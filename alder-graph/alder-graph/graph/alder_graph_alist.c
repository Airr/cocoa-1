/**
 * This file, alder_alder_graph_alist_alist.c, is part of alder-graph.
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

/* See http://pine.cs.yale.edu/pinewiki/C/Graphs */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bstrlib.h"
#include "alder_cmacro.h"
#include "alder_graph_alist.h"

/* basic directed graph type */
/* the implementation uses adjacency lists
 * represented as variable-length arrays */

/* these arrays may or may not be sorted: if one gets long enough
 * and you call alder_graph_alist_has_edge on its source, it will be */

struct alder_graph_alist_struct {
    int n;              /* number of vertices             */
    int m;              /* number of edges                */
    struct successors {
        int d;          /* number of successors           */
        int len;        /* number of slots in array       */
        bstring s;      /* data at a node                 */
        char is_sorted; /* true if list is already sorted */
        int list[1];    /* actual list of successors      */
    } *alist[1];
};

/* create a new graph with n vertices labeled 0..n-1 and no edges */
alder_graph_alist_t
alder_graph_alist_create(int n)
{
    alder_graph_alist_t g;
    int i;
    
    g = malloc(sizeof(struct alder_graph_alist_struct) +
               sizeof(struct successors *) * (n-1));
    assert(g);
    
    g->n = n;
    g->m = 0;
    
    for(i = 0; i < n; i++) {
        g->alist[i] = malloc(sizeof(struct successors));
        assert(g->alist[i]);
        
        g->alist[i]->d = 0;
        g->alist[i]->len = 1;
        g->alist[i]->is_sorted= 1;
    }
    
    return g;
}

/* free all space used by graph */
void
alder_graph_alist_destroy(alder_graph_alist_t g)
{
    int i;
    
    for(i = 0; i < g->n; i++) free(g->alist[i]);
    free(g);
}

void
alder_graph_alist_set_s(alder_graph_alist_t g, int u, bstring s)
{
    g->alist[u]->s = s;
}

/* add an edge to an existing graph */
void
alder_graph_alist_add_edge(alder_graph_alist_t g, int u, int v)
{
    assert(u >= 0);
    assert(u < g->n);
    assert(v >= 0);
    assert(v < g->n);
    
    /* do we need to grow the list? */
    while(g->alist[u]->d >= g->alist[u]->len) {
        g->alist[u]->len *= 2;
        g->alist[u] =
        realloc(g->alist[u],
                sizeof(struct successors) + sizeof(int) * (g->alist[u]->len - 1));
    }
    
    /* now add the new sink */
    g->alist[u]->list[g->alist[u]->d++] = v;
    g->alist[u]->is_sorted = 0;
    
    /* bump edge count */
    g->m++;
}

/* return the number of vertices in the graph */
int
alder_graph_alist_vertex_count(alder_graph_alist_t g)
{
    return g->n;
}

/* return the number of vertices in the graph */
int
alder_graph_alist_edge_count(alder_graph_alist_t g)
{
    return g->m;
}

/* return the out-degree of a vertex */
int
alder_graph_alist_out_degree(alder_graph_alist_t g, int source)
{
    assert(source >= 0);
    assert(source < g->n);
    
    return g->alist[source]->d;
}

/* when we are willing to call bsearch */
#define BSEARCH_THRESHOLD (10)

static int
intcmp(const void *a, const void *b)
{
    return *((const int *) a) - *((const int *) b);
}

/* return 1 if edge (source, sink) exists), 0 otherwise */
int
alder_graph_alist_has_edge(alder_graph_alist_t g, int source, int sink)
{
    int i;
    
    assert(source >= 0);
    assert(source < g->n);
    assert(sink >= 0);
    assert(sink < g->n);
    
    if(alder_graph_alist_out_degree(g, source) >= BSEARCH_THRESHOLD) {
        /* make sure it is sorted */
        if(! g->alist[source]->is_sorted) {
            qsort(g->alist[source]->list,
                  g->alist[source]->d,
                  sizeof(int),
                  intcmp);
        }
        
        /* call bsearch to do binary search for us */
        return
        bsearch(&sink,
                g->alist[source]->list,
                g->alist[source]->d,
                sizeof(int),
                intcmp)
        != 0;
    } else {
        /* just do a simple linear search */
        /* we could call lfind for this, but why bother? */
        for(i = 0; i < g->alist[source]->d; i++) {
            if(g->alist[source]->list[i] == sink) return 1;
        }
        /* else */
        return 0;
    }
}

/* invoke f on all edges (u,v) with source u */
/* supplying data as final parameter to f */
void
alder_graph_alist_foreach(alder_graph_alist_t g, int source,
              void (*f)(alder_graph_alist_t g, int source, int sink, void *data),
              void *data)
{
    int i;
    
    assert(source >= 0);
    assert(source < g->n);
    
    for(i = 0; i < g->alist[source]->d; i++) {
        f(g, source, g->alist[source]->list[i], data);
    }
}


#define TEST_SIZE (37)

/* never call this */
static void
match_sink(alder_graph_alist_t g, int source, int sink, void *data)
{
    assert(data && sink == *((int *) data));
}

int
alder_graph_alist_test()
{
    alder_graph_alist_t g;
    int i;
    int j;
    
    g = alder_graph_alist_create(TEST_SIZE);
    
    assert(alder_graph_alist_vertex_count(g) == TEST_SIZE);
    
    /* check it's empty */
    for(i = 0; i < TEST_SIZE; i++) {
        for(j = 0; j < TEST_SIZE; j++) {
            assert(alder_graph_alist_has_edge(g, i, j) == 0);
        }
    }
    
    /* check it's empty again */
    for(i = 0; i < TEST_SIZE; i++) {
        assert(alder_graph_alist_out_degree(g, i) == 0);
        alder_graph_alist_foreach(g, i, match_sink, 0);
    }
    
    /* check edge count */
    assert(alder_graph_alist_edge_count(g) == 0);
    
    /* fill in the diagonal */
    for(i = 0; i < TEST_SIZE; i++) {
        alder_graph_alist_add_edge(g, i, i);
    }
    
    /* check */
    assert(alder_graph_alist_edge_count(g) == TEST_SIZE);
    
    for(i = 0; i < TEST_SIZE; i++) {
        for(j = 0; j < TEST_SIZE; j++) {
            assert(alder_graph_alist_has_edge(g, i, j) == (i == j));
        }
    }
    
    for(i = 0; i < TEST_SIZE; i++) {
        assert(alder_graph_alist_out_degree(g, i) == 1);
        alder_graph_alist_foreach(g, i, match_sink, &i);
    }
    
    
    /* fill in all the entries */
    for(i = 0; i < TEST_SIZE; i++) {
        for(j = 0; j < TEST_SIZE; j++) {
            if(i != j) alder_graph_alist_add_edge(g, i, j);
        }
    }
    
    /* test they are all there */
    assert(alder_graph_alist_edge_count(g) == TEST_SIZE * TEST_SIZE);
    
    for(i = 0; i < TEST_SIZE; i++) {
        assert(alder_graph_alist_out_degree(g, i) == TEST_SIZE);
        for(j = 0; j < TEST_SIZE; j++) {
            assert(alder_graph_alist_has_edge(g, i, j) == 1);
        }
    }
    
    /* free it */
    alder_graph_alist_destroy(g);
    
    return 0;
}

