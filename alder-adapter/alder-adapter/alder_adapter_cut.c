//
//  Author: Marcel Martin
//  See the at the end of this file for the License of cutadapt.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FIXME: s1 must be uppercase.
//        s2 must be uppercase.
//        what is min_overlap? 3 default (--overlap)
//
// 
//
//        if read has wild card or N, then ALLOW_WILDCARD_SEQ2 is added to wildcard (degenerate?)
//        if adapter has wild card, then ALLOW_WILDCARD_SEQ1 is added to wildcard (degenerate?)
//
// 1. Exact match first (suffix array?)
//
//
//
// s1: adatper sequence (convert it to uppercase)
// m : length of s1
// s2: read sequence (assume it is uppercase)
// n : length of s2
// error_rate:
// flags: 14, 11, 8, or 15 (see FLAG below)
// degenerate:
int alder_adapter_cut(const char *s1,
                      const int m,
                      const char *s2,
                      const int n,
                      const double error_rate,
                      const int flags,
                      const int degenerate,
                      int *start1_,
                      int *best_i_,
                      int *start2_,
                      int *best_j_,
                      int *best_matches_,
                      int *best_cost_)
{
    const int DELETION_COST = 1;
    const int INSERTION_COST = 1;
    const int MISMATCH_COST = 1;
    const int MATCH_COST = 0;

    // BACK = START_WITHIN_SEQ2 | STOP_WITHIN_SEQ2 | STOP_WITHIN_SEQ1 (14)
    // FRONT = START_WITHIN_SEQ2 | STOP_WITHIN_SEQ2 | START_WITHIN_SEQ1 (11)
    // PREFIX = STOP_WITHIN_SEQ2 (8)
    // ANYWHERE = SEMIGLOBAL (15)
    enum FLAG {
        START_WITHIN_SEQ1 = 1,
        START_WITHIN_SEQ2 = 2,
        STOP_WITHIN_SEQ1 = 4,
        STOP_WITHIN_SEQ2 = 8,
        SEMIGLOBAL = 15,
        ALLOW_WILDCARD_SEQ1 = 1,
        ALLOW_WILDCARD_SEQ2 = 2,
    };
        /*
     DP Matrix:
     s2 (j)
     ----------> n
     |
     s1 (i) |
     |
     V
     m
     */
    
    // structure for a DP matrix entry
    typedef struct {
        int cost;
        int matches; // no. of matches in this alignment
        int origin; // where the alignment originated: negative for positions within seq1, positive for pos. within seq2
    } Entry;
    
    // only a single column of the DP matrix is stored
    Entry* column;
    column = (Entry*)malloc((m+1)*sizeof(Entry));
    if (column == NULL)
        return 1;
    
    int i, j, best_i, best_j, best_cost, best_matches, best_origin;
    
    //initialize first column
    for (i = 0; i <= m; ++i) {
        column[i].matches = 0;
        column[i].cost = (flags & START_WITHIN_SEQ1) ? 0 : i * DELETION_COST;
        column[i].origin = (flags & START_WITHIN_SEQ1) ? -i : 0;
    }
    
    best_i = m;
    best_j = 0;
    best_cost = column[m].cost;
    best_matches = 0;
    best_origin = column[m].origin;
    
    // maximum no. of errors
    int k = error_rate * m;
    int last = k + 1;
    if (flags & START_WITHIN_SEQ1) {
        last = m;
    }
    // iterate over columns
    for (j = 1; j <= n; ++j) {
        // remember first entry
        Entry tmp_entry = column[0];
        
        // fill in first entry in this column TODO move out of loop
        if (flags & START_WITHIN_SEQ2) {
            column[0].cost = 0;
            column[0].origin = j;
            column[0].matches = 0;
        } else {
            column[0].cost = j * INSERTION_COST;
            column[0].origin = 0;
            column[0].matches = 0;
        }
        for (i = 1; i <= last; ++i) {
            int match = (s1[i-1] == s2[j-1])
            || ((degenerate & ALLOW_WILDCARD_SEQ1) && (s1[i-1] == 'N'))
            || ((degenerate & ALLOW_WILDCARD_SEQ2) && (s2[j-1] == 'N'));
            int cost_diag = tmp_entry.cost + (match ? MATCH_COST : MISMATCH_COST);
            int cost_deletion = column[i].cost + DELETION_COST;
            int cost_insertion = column[i-1].cost + INSERTION_COST;
            
            int origin, cost, matches;
            if (cost_diag <= cost_deletion && cost_diag <= cost_insertion) {
                // MATCH or MISMATCH
                cost = cost_diag;
                origin = tmp_entry.origin;
                matches = tmp_entry.matches + match;
            } else if (cost_insertion <= cost_deletion) {
                // INSERTION
                cost = cost_insertion;
                origin = column[i-1].origin;
                matches = column[i-1].matches;
            } else {
                // DELETION
                cost = cost_deletion;
                origin = column[i].origin;
                matches = column[i].matches;
            }
            
            // remember current cell for next iteration
            tmp_entry = column[i];
            
            column[i].cost = cost;
            column[i].origin = origin;
            column[i].matches = matches;
            
        }
        while (column[last].cost > k) {
            last--;
        }
        if (last < m) {
            last++;
        } else {
            // found
            // if requested, find best match in last row
            if (flags & STOP_WITHIN_SEQ2) {
                // length of the aligned part of string1
//                int length = m + min(column[m].origin, 0);
                int length = column[m].origin < 0 ? m + column[m].origin : m;
                int cost = column[m].cost;
                int matches = column[m].matches;
                if (cost <= length * error_rate
                    && (matches > best_matches
                        || (matches == best_matches && cost < best_cost))) {
                    // update
                    best_matches = matches;
                    best_cost = cost;
                    best_origin = column[m].origin;
                    best_i = m;
                    best_j = j;
                }
            }
            
        }
        // column finished
    }
    
    if (flags & STOP_WITHIN_SEQ1) {
        // search in last column // TODO last?
        for (i = 0; i <= m; ++i) {
//            int length = i + min(column[i].origin, 0);
            int length = column[i].origin < 0 ? i + column[m].origin : i;
            int cost = column[i].cost;
            int matches = column[i].matches;
            if (cost <= length * error_rate
                && (matches > best_matches
                    || (matches == best_matches && cost < best_cost))) {
                // update best
                best_matches = matches;
                best_cost = cost;
                best_origin = column[i].origin;
                best_i = i;
                best_j = n;
            }
        }
    }
    
    free(column);
    int start1, start2;
    if (best_origin >= 0) {
        start1 = 0;
        start2 = best_origin;
    } else {
        start1 = -best_origin;
        start2 = 0;
    }
    
    
    *start1_ = start1;
    *best_i_ = best_i;
    *start2_ = start2;
    *best_j_ = best_j;
    *best_matches_ = best_matches;
    *best_cost_ = best_cost;
    
    if (best_matches > 3
        && (best_j == n || best_matches == m)) {
        // No code.
//        seq->seq.s[start2] = '\0';
//        seq->qual.s[start2] = '\0';
    } else {
        *start2_ = n;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// License - cutadapt
///////////////////////////////////////////////////////////////////////////////
#if 0
cutadapt
========

cutadapt removes adapter sequences from DNA high-throughput
sequencing data. This is usually necessary when the read length of the
machine is longer than the molecule that is sequenced, such as in
microRNA data.

cutadapt is implemented in Python, with an extension module,
written in C, that implements the alignment algorithm.


Project homepage
================

See http://code.google.com/p/cutadapt/ . Please use the Google code issue
tracker for bug reports and feature requests.


License
=======

(This is the MIT license.)

Copyright (c) 2010-2012 Marcel Martin <marcel.martin@tu-dortmund.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
#endif
