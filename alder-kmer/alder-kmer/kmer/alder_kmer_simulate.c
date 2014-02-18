/**
 * This file, alder_kmer_simulate.c, is part of alder-kmer.
 *
 * Copyright 2013,2014 by Sang Chul Choi
 *
 * alder-kmer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-kmer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-kmer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "p99.h"
#include "bstrmore.h"
#include "alder_dna.h"
#include "alder_progress.h"
#include "alder_logger.h"
#include "alder_cmacro.h"
#include "alder_file.h"
#include "alder_fileseq.h"
#include "bstrlib.h"
#include "alder_rng.h"
#include "alder_bit.h"
#include "alder_kmer.h"
#include "alder_hashtable.h"
#include "alder_encode_number.h"
#include "alder_kmer_simulate.h"

typedef struct alder_kmer_simulate_partition_struct
alder_kmer_simulate_partition_t;

typedef enum {
    ALDER_KMER_SIMULATE_FORMAT_FASTA,
    ALDER_KMER_SIMULATE_FORMAT_FASTQ,
    ALDER_KMER_SIMULATE_FORMAT_SEQUENCE
} alder_kmer_simulate_format_t;

struct alder_kmer_simulate_partition_struct {
    int n_ni;
    int n_np;
    int nip[2];
    FILE **fp;
    int *nk;
    alder_encode_number_t **n;
};

/* See the struct alder_kmer_simulate_partition_t.
 * It is pretty boring to look inside struct create, and destroy functions.
 */
static void
alder_kmer_simulate_partition_destroy(alder_kmer_simulate_partition_t *o)
{
    if (o != NULL) {
        if (o->n != NULL) {
            P99_FORALL(o->nip,i,j) {
                size_t k = i * o->n_np + j;
                alder_encode_number_destroy(o->n[k]);
            }
            XFREE(o->n);
        }
        
        XFREE(o->nk);
        
        if (o->fp != NULL) {
            P99_FORALL(o->nip,i,j) {
                size_t k = i * o->n_np + j;
                XFCLOSE(o->fp[k]);
            }
            XFREE(o->fp);
        }
        XFREE(o);
    }
}

/* See the struct alder_kmer_simulate_partition_t.
 * It is pretty boring to look inside struct create, and destroy functions.
 */
alder_kmer_simulate_partition_t *
alder_kmer_simulate_partition_create(const char *outfile, const char *outdir,
                                     int kmer_size, int ni, int np)
{
    alder_kmer_simulate_partition_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    /* init */
    o->n_ni = ni;
    o->n_np = np;
    o->nip[0] = ni;
    o->nip[1] = np;
    o->fp = NULL;
    o->nk = NULL;
    o->n = NULL;
    /* memory */
    o->fp = malloc(sizeof(*o->fp)*np*ni);
    if (o->fp == NULL) {
        alder_kmer_simulate_partition_destroy(o);
        return NULL;
    }
    memset(o->fp, 0, sizeof(*o->fp)*np*ni);
    for (int i = 0; i < o->n_ni; i++) {
        for (int j = 0; j < o->n_np; j++) {
            bstring bfile = bformat("%s/%s_%d-%d.par", outdir, outfile, i, j);
            if (bfile == NULL) {
                alder_kmer_simulate_partition_destroy(o);
                return NULL;
            }
            o->fp[i * o->n_np + j] = fopen(bdata(bfile), "w");
            if (o->fp[i * o->n_np + j] == NULL) {
                alder_kmer_simulate_partition_destroy(o);
                return NULL;
            }
            bdestroy(bfile);
        }
    }
    o->nk = malloc(sizeof(*o->nk)*np*ni);
    if (o->nk == NULL) {
        alder_kmer_simulate_partition_destroy(o);
        return NULL;
    }
    memset(o->nk, 0, sizeof(*o->nk)*np*ni);
    o->n = malloc(sizeof(*o->n)*np*ni);
    if (o->n == NULL) {
        alder_kmer_simulate_partition_destroy(o);
        return NULL;
    }
    memset(o->n, 0, sizeof(*o->n)*np*ni);
    for (int i = 0; i < o->n_ni; i++) {
        for (int j = 0; j < o->n_np; j++) {
            int k = i * o->n_np + j;
            o->n[k] = alder_encode_number_create_for_kmer(kmer_size);
            if (o->n[k] == NULL) {
                alder_kmer_simulate_partition_destroy(o);
                return NULL;
            }
        }
    }
    return o;
}

static void
alder_kmer_simulate_select(alder_encode_number_t **ss,
                           uint64_t *i_ni,
                           uint64_t *i_np,
                           alder_encode_number_t *s1,
                           alder_encode_number_t *s2,
                           int number_iteration,
                           int number_partition)
{
    uint64_t hash_s1 = alder_encode_number_hash(s1);
    uint64_t hash_s2 = alder_encode_number_hash(s2);
    uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
    *ss = (hash_s1 < hash_s2) ? s1 : s2;
    
    *i_ni = hash_ss % number_iteration;
    *i_np = hash_ss / number_iteration % number_partition;
    alder_log5("hash1: %llu", hash_s1);
    alder_log5("hash2: %llu", hash_s2);
    alder_log5("hashs: %llu", hash_ss);
    alder_log5("ni: %llu", *i_ni);
    alder_log5("np: %llu", *i_np);
}

static void
alder_kmer_simulate_partition_write2(alder_kmer_simulate_partition_t *o,
                                     alder_encode_number2_t *n1,
                                     int i_ni, int i_np)
{
    int k = i_ni * o->n_np + i_np;
    FILE *fp = o->fp[k];
    size_t ib = n1->b / 8;
    size_t jb = n1->b % 8;
    
    /* Encode the Kmer ss. */
    for (size_t i = 0; i < ib; i++) {
        for (size_t j = 0; j < 8; j++) {
            fwrite(&n1->n[i].key8[j], 1, 1, fp);
        }
    }
    for (size_t j = 0; j < jb; j++) {
        fwrite(&n1->n[ib].key8[j], 1, 1, fp);
    }
}

/* This function makes an array of uint8_t with randomly chosen numbers
 * among 4 numbers from 0 to 3.
 */
static void
random_0123(uint8_t *s, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        s[i] = (uint8_t)Equilikely(0, 3);
    }
}

/* This function makes an array of uint8_t with numbers that correspond to
 * its reverse complementary.
 * 
 * A C T G -> 0 1 2 3
 * 
 * A <-> T  0 <-> 2
 * G <-> C  1 <-> 3
 * x = (x + 2) % 4
 *
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
static int
reverse_complementary_0123(uint8_t *s, size_t n)
{
    uint8_t *s2 = malloc(sizeof(*s2) * n);
    if (s2 == NULL) {
        return 1;
    }
    memcpy(s2, s, sizeof(*s) * n);
    for (size_t i = 0; i < n; i++) {
        s[i] = (s2[n-i-1] + 2) % 4;
    }
    
    XFREE(s2);
    return 0;
}

/**
 *  This function generates sequence and partition files, but not a hash table.
 *
 *  @param version              version
 *  @param outfile_given        outfile_given
 *  @param with_parfile_flag    crate parfile as well
 *  @param outfile              outfile prefix
 *  @param outdir               outdire
 *  @param format               format
 *  @param number_file          number of file
 *  @param number_iteration     ni
 *  @param number_partition     np
 *  @param kmer_size            K
 *  @param sequence_length      length of a sequence
 *  @param maxkmer              max number of kmer
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_simulate(long version,
                        unsigned int outfile_given,
                        const char *outfile,
                        const char *outdir,
                        alder_format_type_t format,
                        int with_parfile_flag,
                        int number_file,
                        int number_iteration,
                        int number_partition,
                        int kmer_size,
                        int sequence_length,
                        size_t maxkmer,
                        int progress_flag,
                        int progressToError_flag)
{
    int s = 0;
    
    ///////////////////////////////////////////////////////////////////////////
    // Create a hash table; this is not for creating a table file.
    // I'd check if there is a kmer with multiple occurences.
    alder_hashtable_mcas_t *ht = alder_hashtable_mcas_create(kmer_size,
                                                             maxkmer * 2);
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Create par files.
    alder_kmer_simulate_partition_t * opart = NULL;
    if (with_parfile_flag == 1) {
        opart = alder_kmer_simulate_partition_create(outfile, outdir,
                                                     kmer_size,
                                                     number_iteration,
                                                     number_partition);
        if (opart == NULL) return ALDER_STATUS_ERROR;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Two kmer sequences.
    alder_encode_number_t * s1 = NULL;
    alder_encode_number_t * s2 = NULL;
    alder_encode_number_t * s3 = NULL; // for res_key in mcas function.
    alder_encode_number2_t * s21 = NULL;
    alder_encode_number2_t * s22 = NULL;
    uint8_t *random_kmer = NULL;
    s1 = alder_encode_number_create_for_kmer(kmer_size);
    s2 = alder_encode_number_create_for_kmer(kmer_size);
    s3 = alder_encode_number_create_for_kmer(kmer_size);
    s21 = alder_encode_number2_createWithKmer(kmer_size);
    s22 = alder_encode_number2_createWithKmer(kmer_size);
    random_kmer = malloc(sizeof(*random_kmer) * kmer_size);
    if (s1 == NULL || s2 == NULL || s3 == NULL ||
        s21 == NULL || s22 == NULL ||
        random_kmer == NULL) {
        XFREE(random_kmer);
        alder_encode_number2_destroy(s22);
        alder_encode_number2_destroy(s21);
        alder_encode_number_destroy(s3);
        alder_encode_number_destroy(s2);
        alder_encode_number_destroy(s1);
        alder_kmer_simulate_partition_destroy(opart);
        return ALDER_STATUS_ERROR;
    }
    memset(random_kmer,0,sizeof(*random_kmer) * kmer_size);
   
    int i_seq = 0;
    if (outfile_given == 0 && number_file == 1) {
        progress_flag = 0;
    }
    alder_progress_start(10000);
    for (int i = 0; i < number_file; i++) {
        alder_hashtable_mcas_reset(ht);
        
        bstring bfilename;
        if (format == ALDER_FORMAT_FASTA) {
            bfilename = bformat("%s/%s-%02d.fa", outdir, outfile, i);
        } else if (format == ALDER_FORMAT_FASTQ) {
            bfilename = bformat("%s/%s-%02d.fq", outdir, outfile, i);
        } else {
            bfilename = bformat("%s/%s-%02d.seq", outdir, outfile, i);
        }
        
        FILE *fpseq = NULL;
        if (number_file == 1 && outfile_given == 0) {
            fpseq = stdout;
        } else {
            fpseq = fopen(bdata(bfilename), "w");
        }
        
        /* Main whlie-loop to create sequences */
        int i_sequence_length = 0;
        size_t n_kmer = 0;
        while (n_kmer < maxkmer) {
            if (progress_flag == 1) {
                alder_progress_step((i * maxkmer) + n_kmer,
                                    number_file * maxkmer,
                                    progressToError_flag);
            }
            n_kmer++;
            
            // Start a new sequence by creating a kmer.
            if (i_sequence_length == 0) {
                i_seq++;
                i_sequence_length = kmer_size - 1;
                random_0123(random_kmer, kmer_size);
                /* version 1 */
                alder_encode_number_kmer(s1, random_kmer);
                reverse_complementary_0123(random_kmer, kmer_size);
                alder_encode_number_kmer(s2, random_kmer);
                reverse_complementary_0123(random_kmer, kmer_size);
                /* version 2 */
                alder_encode_number2_kmer(s21, random_kmer);
                reverse_complementary_0123(random_kmer, kmer_size);
                alder_encode_number2_kmer(s22, random_kmer);
                reverse_complementary_0123(random_kmer, kmer_size);
                
                /* Write the first k-1 letters. */
                if (format == ALDER_FORMAT_FASTA) {
                    fprintf(fpseq, ">%d\n", i_seq);
                } else if (format == ALDER_FORMAT_FASTQ) {
                    fprintf(fpseq, "@%d\n", i_seq);
                }
                for (int j = 1; j < kmer_size; j++) {
                    fputc(alder_dna_int2char(random_kmer[j]), fpseq);
                }
            }
            
            // Create a next kmer by shifting a letter.
            // Write the kmer to one of the partition files.
            int b1 = (int)Equilikely(0,3);
            int b2 = (b1 + 2) % 4;
            /* version 1 */
            alder_encode_number_kmer_shift_left_with(s1, b1);
            alder_encode_number_kmer_shift_right_with(s2, b2);
            alder_encode_number_t *ss = NULL;
            uint64_t i_ni = 0;
            uint64_t i_np = 0;
            alder_kmer_simulate_select(&ss, &i_ni, &i_np, s1, s2,
                                                   number_iteration,
                                                   number_partition);
            alder_hashtable_mcas_increment(ht, ss->n, s3->n, false);
            /* version 2 */
            alder_encode_number2_shiftLeftWith(s21, b1);
            alder_encode_number2_shiftRightWith(s22, b2);
            alder_encode_number2_t *s2s = NULL;
            i_ni = 0;
            i_np = 0;
            alder_hashtable_mcas_select2(&s2s, &i_ni, &i_np, s21, s22,
                                         number_iteration,
                                         number_partition);
            
            if (opart != NULL) {
                alder_kmer_simulate_partition_write2(opart, s2s,
                                                     (int)i_ni, (int)i_np);
            }
            
            // End the current sequence, and start a new sequence.
            i_sequence_length++;
            fputc(alder_dna_int2char(b1), fpseq);
            if (i_sequence_length == sequence_length) {
                // Tail of the sequence
                if (format == ALDER_FORMAT_FASTQ) {
                    fprintf(fpseq, "\n+\n");
                    for (int i_sequence_pos = 0;
                         i_sequence_pos < i_sequence_length;
                         i_sequence_pos++) {
                        fputc('?', fpseq);
                    }
                }
                fputc('\n', fpseq);
                i_sequence_length = 0;
            } else {
                // write the last letter to a file.
                if (format == ALDER_FORMAT_FASTA && i_sequence_length % 60 == 0) {
                    fputc('\n', fpseq);
                }
            }
        }
        
        
        // Tail of the sequence
        if (i_sequence_length > 0) {
            if (format == ALDER_FORMAT_FASTQ) {
                fprintf(fpseq, "\n+\n");
                for (int i_sequence_pos = 0;
                     i_sequence_pos < i_sequence_length;
                     i_sequence_pos++) {
                    fputc('?', fpseq);
                }
            }
            fputc('\n', fpseq);
        }
        
        if (number_file == 1 && outfile_given == 0) {
            // No code.
        } else {
            XFCLOSE(fpseq);
        }
        bdestroy(bfilename);
    }
    
    if (progress_flag == 1) {
        alder_progress_end(progressToError_flag);
    }

    int max_count = alder_hashtable_mcaspseudo_maximum_count(ht);
    if (max_count == 1) {
        s = ALDER_STATUS_SUCCESS;
    } else {
        s = ALDER_STATUS_ERROR;
    }
    
    XFREE(random_kmer);
    alder_encode_number_destroy(s3);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(s1);
    alder_kmer_simulate_partition_destroy(opart);
    alder_hashtable_mcas_destroy(ht);
    return s;
}

void write_set_to_seqfile(FILE *fpseq, alder_format_type_t format,
                          int sequence_length,
                          uint8_t *random_kmer, int kmer_size, size_t maxkmer,
                          int error_rate,
                          int progress_flag, int progressToError_flag)
{
    
    double y = (double)error_rate / 100.0;
    
    int i_seq = 0;
    int i_sequence_length = 0;
    size_t n_kmer = 0;
    while (n_kmer < maxkmer) {
        if (progress_flag == 1) {
//            alder_progress_step((i * maxkmer) + n_kmer,
//                                number_file * maxkmer,
//                                progressToError_flag);
        }
        
        // Start a new sequence by creating a kmer.
        if (i_sequence_length == 0) {
            i_seq++;
            
            /* Write the first k-1 letters. */
            if (format == ALDER_FORMAT_FASTA) {
                fprintf(fpseq, ">%d\n", i_seq);
            } else if (format == ALDER_FORMAT_FASTQ) {
                fprintf(fpseq, "@%d\n", i_seq);
            }
        }
        
        i_sequence_length++;
        if (kmer_size <= i_sequence_length) {
            n_kmer++;
        }
        
        int b1 = (int)Equilikely(0,3);
        double x = (double)rand() / (double)RAND_MAX;
        
        if (x < y) {
//            int b2 = (int)Equilikely(0, 2);
            int b2 = rand() % 3;
            b1 = (b1 + b2 + 1) % 4;
        }
        fputc(alder_dna_int2char(b1), fpseq);
        
        // End the current sequence, and start a new sequence.
        if (i_sequence_length == sequence_length) {
            // Tail of the sequence
            if (format == ALDER_FORMAT_FASTQ) {
                fprintf(fpseq, "\n+\n");
                for (int i_sequence_pos = 0;
                     i_sequence_pos < i_sequence_length;
                     i_sequence_pos++) {
                    fputc('?', fpseq);
                }
            }
            fputc('\n', fpseq);
            i_sequence_length = 0;
        } else {
            // write the last letter to a file.
            if (format == ALDER_FORMAT_FASTA && i_sequence_length % 60 == 0) {
                fputc('\n', fpseq);
            }
        }
    }
    
    // Tail of the sequence
    if (i_sequence_length > 0) {
        if (format == ALDER_FORMAT_FASTQ) {
            fprintf(fpseq, "\n+\n");
            for (int i_sequence_pos = 0;
                 i_sequence_pos < i_sequence_length;
                 i_sequence_pos++) {
                fputc('?', fpseq);
            }
        }
        fputc('\n', fpseq);
    }
}

/**
 *  This function generates a set of sequences with errors. A set of sequences
 *  are generated first, and they are saved in a file with errors in DNAs. 
 *  For the first n_iteration_random iterations, random data sets are
 *  generated. After that point, a single data set is generated and errors
 *  are introduced. A data set with errors can be repeated error_duplicate 
 *  times. Error rate is per base. This would produce a data set with mostly
 *  single instances in the front, and later kmers with multiple counts but
 *  some of them are error_duplicate repeated as errors.
 *
 *  @param version              version
 *  @param outfile_given        outfile_given
 *  @param outfile              outfile name
 *  @param outdir               outfile directory
 *  @param format               format
 *  @param error_iteartion      number of iteartions until introducing errors
 *  @param number_file          number of files
 *  @param error_rate           error rate in one iteration
 *  @param error_duplicate      same error in multiple iterations
 *  @param kmer_size            kmer size
 *  @param sequence_length      sequence length
 *  @param maxkmer              max kmer (determining the size of data)
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_simulate2(long version,
                         unsigned int outfile_given,
                         const char *outfile,
                         const char *outdir,
                         alder_format_type_t format,
                         int number_file,
                         long seed,
                         int error_rate,
                         int error_initial,
                         int error_iteration,
                         int error_duplicate,
                         int kmer_size,
                         int sequence_length,
                         size_t maxkmer,
                         int progress_flag,
                         int progressToError_flag)
{
    int s = 0;
    srand((unsigned int)seed);
    
    uint8_t *random_kmer = NULL;
    random_kmer = malloc(sizeof(*random_kmer) * kmer_size);
    if (random_kmer == NULL) {
        return ALDER_STATUS_ERROR;
    }
    memset(random_kmer,0,sizeof(*random_kmer) * kmer_size);
   
//    int i_seq = 0;
    if (outfile_given == 0 && number_file == 1) {
        progress_flag = 0;
    }
    alder_progress_start(10000);
    for (int i = 0; i < number_file; i++) {
        
        bstring bfilename;
        if (format == ALDER_FORMAT_FASTA) {
            bfilename = bformat("%s/%s-%02d.fa", outdir, outfile, i);
        } else if (format == ALDER_FORMAT_FASTQ) {
            bfilename = bformat("%s/%s-%02d.fq", outdir, outfile, i);
        } else {
            bfilename = bformat("%s/%s-%02d.seq", outdir, outfile, i);
        }
        
        FILE *fpseq = NULL;
        if (number_file == 1 && outfile_given == 0) {
            fpseq = stdout;
        } else {
            fpseq = fopen(bdata(bfilename), "w");
        }
        
        long seed_new;
        if (seed == -1) {
            seed_new = ((unsigned long) time((time_t *) NULL)) + i;
        } else {
            seed_new = seed + i;
        }
        
        /* Main whlie-loop to create sequences */
        PlantSeeds(seed_new);
        for (int i_initial = 0; i_initial < error_initial; i_initial++) {
            write_set_to_seqfile(fpseq, format, sequence_length,
                                 random_kmer, kmer_size, maxkmer,
                                 0,
                                 progress_flag, progressToError_flag);
        }
    
        int i_iteration = 0;
        while (i_iteration < error_iteration) {
            int x_duplicate = (int)Equilikely(1, error_duplicate);
//            printf("x_duplicate: %d\n", x_duplicate);
            if (i_iteration + x_duplicate < error_iteration) {
                i_iteration += x_duplicate;
            } else {
                x_duplicate = error_iteration - i_iteration;
                i_iteration += x_duplicate;
            }
            
            for (int i_duplicate = 0; i_duplicate < x_duplicate; i_duplicate++) {
                PlantSeeds(seed_new + 1);
                srand((unsigned int)seed_new + i_iteration);
                write_set_to_seqfile(fpseq, format, sequence_length,
                                     random_kmer, kmer_size, maxkmer,
                                     error_rate,
                                     progress_flag, progressToError_flag);
            }
        }
        
        if (number_file == 1 && outfile_given == 0) {
            // No code.
        } else {
            XFCLOSE(fpseq);
        }
        bdestroy(bfilename);
    }
    
    if (progress_flag == 1) {
//        alder_progress_end(progressToError_flag);
    }

    XFREE(random_kmer);
    return s;
}