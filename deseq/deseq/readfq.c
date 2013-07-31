//
//  readfq.c
//  deseq
//
//  Created by Sang Chul Choi on 7/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <zlib.h>
#include <stdio.h>
#include "kseq.h"

// MacOSX Only
#include <fcntl.h>

KSEQ_INIT(gzFile, gzread)

int peach_readfq(const char* path)
{
	gzFile fp;
	kseq_t *seq;
    int fileDescriptor;
	int n = 0, slen = 0, qlen = 0;
    
    fileDescriptor = open("/tmp/deseq.PEh6hk/sample1.fastq.gz", O_RDONLY);
	fp = gzdopen(fileDescriptor, "r");
	seq = kseq_init(fp);
	while (kseq_read(seq) >= 0)
    {
        ++n, slen += seq->seq.l, qlen += seq->qual.l;
    }
	printf("%d\t%d\t%d\n", n, slen, qlen);
	kseq_destroy(seq);
	gzclose(fp);
	return 0;
}

// Copy a FASTQ file.
int peachFastqCopy(const char* path)
{
	gzFile fp1;
    gzFile fp2;
	kseq_t *seq;
    int fileDescriptor1;
    int fileDescriptor2;
	int n = 0, slen = 0, qlen = 0;
    
    fileDescriptor1 = open("/tmp/deseq.PEh6hk/sample1.fastq.gz", O_RDONLY);
    fileDescriptor2 = open("/tmp/deseq.PEh6hk/sample1.prinseq.fastq.gz", O_WRONLY|O_CREAT);
	fp1 = gzdopen(fileDescriptor1, "r");
    fp2 = gzdopen(fileDescriptor2, "w");
	seq = kseq_init(fp1);
	while (kseq_read(seq) >= 0)
    {
        //name, comment, seq, qual
        gzprintf(fp2, "@%s %s\n", seq->name.s, seq->comment.s);
        gzprintf(fp2, "%s\n", seq->seq.s);
        gzprintf(fp2, "+\n");
        gzprintf(fp2, "%s\n", seq->qual.s);
//        ++n, slen += seq->seq.l, qlen += seq->qual.l;
    }
	printf("%d\t%d\t%d\n", n, slen, qlen);
	kseq_destroy(seq);
	gzclose(fp1);
    gzclose(fp2);
	return 0;
}

// Remove left and right sides.
int peachFastqPreprocess(const char* path)
{
	gzFile fp1;
    gzFile fp2;
	kseq_t *seq;
    int fileDescriptor1;
    int fileDescriptor2;
	int n = 0, slen = 0, qlen = 0;
    char *seqs = NULL;
    char *quals = NULL;
    
    seqs = (char *) malloc(1000*sizeof(char));
    quals = (char *) malloc(1000*sizeof(char));
    fileDescriptor1 = open("/tmp/deseq.PEh6hk/sample1.fastq.gz", O_RDONLY);
    fileDescriptor2 = open("/tmp/deseq.PEh6hk/sample1.prinseq.fastq.gz", O_WRONLY|O_CREAT);
	fp1 = gzdopen(fileDescriptor1, "r");
    fp2 = gzdopen(fileDescriptor2, "w");
	seq = kseq_init(fp1);
	while (kseq_read(seq) >= 0)
    {
        //name, comment, seq, qual
        strncpy(seqs, seq->seq.s + 20, 60);
        strncpy(quals, seq->qual.s + 20, 60);
        gzprintf(fp2, "@%s %s\n", seq->name.s, seq->comment.s);
        gzprintf(fp2, "%s\n", seqs);
        gzprintf(fp2, "+\n");
        gzprintf(fp2, "%s\n", quals);
        //        ++n, slen += seq->seq.l, qlen += seq->qual.l;
    }
	printf("%d\t%d\t%d\n", n, slen, qlen);
	kseq_destroy(seq);
	gzclose(fp1);
    gzclose(fp2);
    free(seqs);
    free(quals);
	return 0;
}
