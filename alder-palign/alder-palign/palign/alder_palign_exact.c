/**
 * This file, alder_palign_exact.c, is part of alder-align.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-align is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-align is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-align.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "alder_fasta.h"
#include "gsl_vector_sam.h"
#include "alder_palign_exact.h"

int alder_palign_exact_execute_with_anchor(alder_sam_t *sam,
                                           alder_vector_palign_t *palign,
                                           const size_t palignIndex,
                                           const alder_fasta_t *fS,
                                           const char *queryName)
{
    const size_t numberOfAnchor = palign->data[palignIndex].anchor->size;
    assert(numberOfAnchor == 1);
    const int64_t startReference = palign->data[palignIndex].s;
    const char *seqRead = bdata(palign->data[palignIndex].query);
    int anchorLength =(int)(palign->data[palignIndex].qanchor->data[0].e - palign->data[palignIndex].qanchor->data[0].s);
    
    // This used to be the start of the function in the alder_palign_dialign.
    ///////////////////////////////////////////////////////////////////////////
    // We complete the alignment with sam.
    ///////////////////////////////////////////////////////////////////////////
//    alder_cigar_queue_t *qu = alder_cigar_queue_create();
//    alder_cigar_item_t m = { .n = 0, .t = 0 };
//    alder_cigar_enqueue(m, qu);
    
    // query sequence name.
    bstring bQuery = bfromcstr(queryName);
    sam->qname = bstr2cstr(bQuery, '\0');
    bdestroy(bQuery);
    sam->flag = 0;
    // reference sequence name.
    // 1. Find the reference name in 
    sam->rname = alder_fasta_name_by_saindex(fS, startReference);
    
    // 1-based
    sam->pos = (uint32_t)alder_fasta_position_by_saindex(fS, startReference) + 1;
    sam->mapq = 37;
    bstring bCigar = bformat("%dM",anchorLength);
    sam->cigar = bstr2cstr(bCigar, '\0');
    bdestroy(bCigar);
    
    // The followings are for pair-end data.
    bstring bRnext = bfromcstr("*");
    sam->rnext = bstr2cstr(bRnext, '\0');
    bdestroy(bRnext);
    sam->pnext = 0;
    sam->tlen = 0;
    
    //    sam->seq; must have been set earlier.
    bstring bSeq = bfromcstr(seqRead);
    sam->seq = bstr2cstr(bSeq, '\0');
    bdestroy(bSeq);
    
    bstring bQual = bfromcstr("*");
    sam->qual = bstr2cstr(bQual, '\0');
    bdestroy(bQual);
    
    bstring bOptional = bfromcstr("HO:i:1");
    sam->optional = bstr2cstr(bOptional, '\0');
    bdestroy(bOptional);
    
    // Use bcstrfree() to delete
    // qname, rname, rnext, qual, optional, cigar
    ///////////////////////////////////////////////////////////////////////////
    
    return 0;
}