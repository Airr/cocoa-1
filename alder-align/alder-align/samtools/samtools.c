//
//  samtools.c
//  alder-align
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "../samtools/sam.h"
#include "../fasta/alder_fasta.h"
//#include "../palign/alder_align_alignment.h"
#include "gsl_vector_sam.h"
#include "../bstring/bstraux.h"
#include "samtools.h"

    // First, write a header.
    //    bam_header_t *header = bam_header_init();
    //    const char *s = "@HD\tVN:1.5\tSO:coordinate\n@SQ\tSN:1\tLN:45\n@SQ\tSN:2\tLN:15\n";
    //    size_t lenS = strlen(s);
    //    header->text = malloc((lenS+1)*sizeof(char));
    //    strcpy(header->text, s);
    //    header->l_text = (uint32_t)strlen(header->text);
    //    sam_header_parse(header);
    //    char *sp = sam_header_write(header->dict);
    //    fp = fopen(fn2, "w");
    //    fprintf(fp, "%s\n", sp);
    //    fclose(fp);
    //    free(sp);
    
    //    samfile_t *out = samopen(fn2, "wh", header);

FILE *alder_sam_open(const char *fn, const fasta_t *fh)
{
    FILE *fp = fopen(fn, "w");
    if (fp == NULL) {
        fprintf(stderr, "error: alder_sam_open failed");
        return NULL;
    }
    fprintf(fp, "@HD\tVN:1.4\tSO:unsorted\n");
    for (size_t i = 0; i < fh->numberOfSequence; i++) {
        int64_t len;
        if (i == 0) {
            len = fh->index[0];
        } else if (i == fh->numberOfSequence - 1) {
            len = fh->numberOfBase - fh->index[i];
        } else {
            len = fh->index[i] - fh->index[i-1];
        }
        fprintf(fp, "@SQ\tSN:%zd\tLN:%lld", i, len);
    }
    fprintf(fp, "@PG\tID:1\tPN:alder\n");
    return fp;
}

int alder_sam_print(FILE *fp, const alder_sam_t *a)
{
    fprintf(fp, "%s\t%d\t%s\t%d\t%d\t%s\t%s\t%d\t%d\t%s\t%s\n",
            a->qname, a->flag, a->rname, a->pos, a->mapq, a->cigar, a->rnext,
            a->pnext, a->tlen, a->seq, a->qual);
}

void alder_sam_close(FILE *fp)
{
    fclose(fp);
}

int alder_sam_check(const char *fn)
{
    bstring bstrFn = bfromcstr(fn);
    bstring bstrExtension = bTail(bstrFn, 4);
    bstring bstrExtensionCam = bfromcstr(".cam");
    bstring bstrExtensionSam = bfromcstr(".sam");
    bstring bstrFnSam;
    if (bstrcmp(bstrExtension, bstrExtensionCam)) {
        bstrFnSam = bstrcpy(bstrFn);
        bassignmidstr(bstrFnSam, bstrFn, 0, blength(bstrFn)-4);
        bconcat(bstrFnSam, bstrExtensionSam);
    } else {
        bdestroy(bstrFn);
        bdestroy(bstrExtension);
        bdestroy(bstrExtensionCam);
        bdestroy(bstrExtensionSam);
        fprintf(stderr, "error: the input must be cam\n");
        return 1;
    }
    samfile_t *in = samopen(fn, "r", NULL);
    samfile_t *out = samopen(bdata(bstrFnSam), "wh", in->header);
    
    bam1_t *b = bam_init1();
    int r;
    int64_t count = 0;
    while ((r = samread(in, b)) >= 0) { // read one alignment from `in'
        if (!process_aln(in->header, b)) {
            samwrite(out, b); // write the alignment to `out'
            count++;
        }
    }
    if (r < -1) {
        fprintf(stderr, "[main_samview] truncated file.\n");
    }
    //    bam_header_destroy(header);
    bam_destroy1(b);
    samclose(in);
    samclose(out);
                         
    bdestroy(bstrFn);
    bdestroy(bstrFnSam);
    bdestroy(bstrExtension);
    bdestroy(bstrExtensionCam);
    bdestroy(bstrExtensionSam);
    return 0;
}

