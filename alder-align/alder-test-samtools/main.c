//
//  main.c
//  alder-test-samtools
//
//  Created by Sang Chul Choi on 8/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include "sam.h"
#include "sam_header.h"
#include "../alder-align/samtools/samtools.h"

// Functions that could be useful.
// print_header_line, sam_header_line_parse, sam_header_parse2, sam_header_parse
// bam_header_t->dict is HeaderLine. So, we could use this to print out or
// create header.
//
// sam file open
// samopen samclose samread samwrite
//
// Conclusion
// ----------
// It is not easy to read the code of samtools to use to create sam/bam files.
// We could write a code to write a sam file, use samtools to read it and write
// it back to sam or bam file. This could be an easy way to write a sam/bam file
// that is compatible with the sam specification.
//
// bam.c
// bam_verbose = 0 to make samtools quiet.

int main(int argc, const char * argv[])
{
    // Samfile
//	samfile_t *fp;
//    tamFile ft;
//    ft = (tamFile)calloc(1, sizeof(struct __tamFile_t));
//    ft->str = (kstring_t*)calloc(1, sizeof(kstring_t));
//    fp->x.tamr = ft;
//    fp->header = header;
//    const char *s = "@HD\tVN:1.5\tSO:coordinate\n@SQ\tSN:ref\tLN:45";
    
#if 0
    bam_header_t *header = bam_header_init();
    const char *s = "@HD\tVN:1.5\tSO:coordinate\n@SQ\tSN:1\tLN:45\n@SQ\tSN:2\tLN:15";
    size_t lenS = strlen(s);
    header->text = malloc((lenS+1)*sizeof(char));
    strcpy(header->text, s);
    header->l_text = (uint32_t)strlen(header->text);
    sam_header_parse(header);
    for (int32_t i = 0; i < header->n_targets; ++i) {
        fprintf(stdout, "@SQ\tSN:%s\tLN:%d\n", header->target_name[i], header->target_len[i]);
    }
    char *sp = sam_header_write(header->dict);
    fprintf(stdout, "%s\n", sp);
    free(sp);
    bam_header_destroy(header);
#endif
    const char *fn = "data/test.cam";
    FILE *fp = fopen(fn, "w");
    fprintf(fp, "@HD\tVN:1.4\tSO:unsorted\n");
    for (size_t i = 0; i < 3; i++) {
        fprintf(fp, "@SQ\tSN:%ld\tLN:%ld\n", i+1, (i+1)*1000000);
    }
    fprintf(fp, "r001\t163\t1\t7\t30\t8M2I4M1D3M\t=\t37\t39\tTTAGATAAAGGATACTG\t*\n");
    fclose(fp);
    
    const char *fn2 = "data/test.sam";
    samfile_t *in = samopen(fn, "r", NULL);
    
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
    samfile_t *out = samopen(fn2, "wh", in->header);
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
    
    
//	samclose(fp);
	return 0;
}
