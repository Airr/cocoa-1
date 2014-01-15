/**
 * This file, alder_kmer_simulate.c, is part of alder-kmer.
 *
 * Copyright 2013 by Sang Chul Choi
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

static void random_bseq(uint8_t *s, size_t n, bstring bseq, size_t offset);

static void
alder_kmer_simulate_select(alder_hashtable_mcas_t **cht,
                           alder_encode_number_t **ss,
                           uint64_t *i_ni,
                           uint64_t *i_np,
                           alder_encode_number_t *s1,
                           alder_encode_number_t *s2,
                           alder_hashtable_mcas_t **ht,
                           int number_iteration,
                           int number_partition);

static void
alder_kmer_simulate_select_woHashtable(alder_encode_number_t **ss,
                                       uint64_t *i_ni,
                                       uint64_t *i_np,
                                       alder_encode_number_t *s1,
                                       alder_encode_number_t *s2,
                                       int number_iteration,
                                       int number_partition);

static alder_kmer_simulate_partition_t *
alder_kmer_simulate_partition_create(const char *outfile, const char *outdir,
                                     int kmer_size, int ni, int np);

static void
alder_kmer_simulate_partition_destroy(alder_kmer_simulate_partition_t *o);

static void
alder_kmer_simulate_partition_write(alder_kmer_simulate_partition_t *o,
                                    alder_encode_number_t *n,
                                    int i_ni, int i_np);

static void
alder_kmer_simulate_partition_flush(alder_kmer_simulate_partition_t *o,
                                    int i_ni, int i_np);

static void
alder_kmer_simulate_partition_write2(alder_kmer_simulate_partition_t *o,
                                     alder_encode_number2_t *n1,
                                     int i_ni, int i_np);

static void random_0123(uint8_t *s, size_t n);
static int reverse_complementary_0123(uint8_t *s, size_t n);


/* This function generates data for checking alder-kmer.
 *
 * 1. Fill the tables.
 * 2. Construct a sequence list.
 * 3. Use the list to add more sequences, and put them in sequence files.
 * 4. Repeat the procedure above until there is a slot whose count reach max.
 *
 * max is 256.
 *
 * memory variables
 * 1. opart - all of the partition files in all iterations.
 * 2. s1, s2 - encoded kmer
 * 3. random_kmer - initial kmer
 * 4. ht - array of hash tables.
 * 5. seqs - sequences generated to fill up the hash tables.
 *
 */
int alder_kmer_simulate(const char *outfile,
                        const char *outdir,
                        alder_format_type_t format,
                        int number_file,
                        int number_iteration,
                        int number_partition,
                        int kmer_size,
                        int sequence_length,
                        int hashtable_maxcount,
                        int hashtable_size)
{
    int s = 0;
    int number_hash_table = number_partition * number_iteration;
    
    ///////////////////////////////////////////////////////////////////////////
    // Open parition files.
    alder_kmer_simulate_partition_t * opart =
    alder_kmer_simulate_partition_create(outfile, outdir, kmer_size,
                                         number_iteration, number_partition);
    if (opart == NULL) {
        return ALDER_STATUS_ERROR;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Hash tables, two kmer sequences.
    alder_encode_number_t * s1 = NULL;
    alder_encode_number_t * s2 = NULL;
    alder_encode_number_t * s3 = NULL; // for res_key in mcas function.
    alder_hashtable_mcas_t **ht = NULL;
    uint8_t *random_kmer = NULL;
    s1 = alder_encode_number_create_for_kmer(kmer_size);
    s2 = alder_encode_number_create_for_kmer(kmer_size);
    s3 = alder_encode_number_create_for_kmer(kmer_size);
    random_kmer = malloc(sizeof(*random_kmer) * kmer_size);
    ht = malloc(sizeof(*ht) * number_hash_table);
    if (s1 == NULL || s2 == NULL || s3 == NULL ||
        random_kmer == NULL || ht == NULL) {
        XFREE(ht);
        XFREE(random_kmer);
        alder_encode_number_destroy(s3);
        alder_encode_number_destroy(s2);
        alder_encode_number_destroy(s1);
        alder_kmer_simulate_partition_destroy(opart);
        return ALDER_STATUS_ERROR;
    }
    memset(random_kmer,0,sizeof(*random_kmer) * kmer_size);
   
    ///////////////////////////////////////////////////////////////////////////
    // Hash tables.
    memset(ht, 0, sizeof(*ht) * number_hash_table);
    for (int i = 0; i < number_hash_table; i++) {
        ht[i] = alder_hashtable_mcas_create(kmer_size,
                                            hashtable_size);
        if (ht[i] == NULL) break;
    }
    if (ht[number_hash_table - 1] == NULL) {
        for (int i = 0; i < number_hash_table; i++) {
            if (ht[i] != NULL) alder_hashtable_mcas_destroy(ht[i]);
        }
        XFREE(ht);
        XFREE(random_kmer);
        alder_encode_number_destroy(s2);
        alder_encode_number_destroy(s1);
        alder_kmer_simulate_partition_destroy(opart);
        return ALDER_STATUS_ERROR;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // Main part of the simulation
    ///////////////////////////////////////////////////////////////////////////
    struct bstrList *seqs = bstrVectorCreate(1);
    bstring bseq = bfromcstr("");
    // Create a kmer sequence.
    random_0123(random_kmer, kmer_size);
    alder_encode_number_kmer(s1, random_kmer);
    reverse_complementary_0123(random_kmer, kmer_size);
    alder_encode_number_kmer(s2, random_kmer);
    alder_encode_number_to_bstringWithOffset(s1, bseq, 1);
#if !defined(NDEBUG)
    bstring bseq_debug = bfromcstr("");
    alder_encode_number_to_bstring(s1, bseq_debug);
    alder_log5("s1: %s", bdata(bseq_debug));
    alder_encode_number_to_bstring(s2, bseq_debug);
    alder_log5("s2: %s", bdata(bseq_debug));
    alder_log5("bseq: %s", bdata(bseq));
#endif
    
    int remaining_slot = 0;
    for (int i = 0; i < number_hash_table; i++) {
        remaining_slot += alder_hashtable_mcaspseudo_number_open_slot(ht[i]);
    }
    alder_log5("Number of hash tables: %d", number_hash_table);
    alder_log5("Empty slots are %d", remaining_slot);
    int maximum_slot = 0;
    while (remaining_slot > 0 && maximum_slot < hashtable_maxcount/2) {
        alder_encode_number_kmer_shift_left(s1);
        alder_encode_number_kmer_shift_right(s2);
#if !defined(NDEBUG)
        alder_encode_number_to_bstring(s1, bseq_debug);
        alder_log5("s1 (left-shift with G): %s", bdata(bseq_debug));
        alder_encode_number_to_bstring(s2, bseq_debug);
        alder_log5("s2(right-shift with G): %s", bdata(bseq_debug));
#endif
        int n_possible_letter = 0;
        int possible_letter[4];
        for (int i = 0; i < 4; i++) {
            int b1 = i;
            int b2 = (b1 + 2) % 4;
            alder_encode_number_kmer_replace_left_with(s1, b1);
            alder_encode_number_kmer_replace_right_with(s2, b2);
#if !defined(NDEBUG)
            alder_encode_number_to_bstring(s1, bseq_debug);
            alder_log5("s1 (left-shift with %c): %s",
                       alder_dna_int2char(b1),
                       bdata(bseq_debug));
            alder_encode_number_to_bstring(s2, bseq_debug);
            alder_log5("s2(right-shift with %c): %s",
                       alder_dna_int2char(b2),
                       bdata(bseq_debug));
#endif
            
            alder_hashtable_mcas_t *cht = NULL;
            alder_encode_number_t *ss = NULL;
            uint64_t i_ni = 0;
            uint64_t i_np = 0;
            alder_kmer_simulate_select(&cht, &ss, &i_ni, &i_np, s1, s2, ht,
                                       number_iteration, number_partition);
            
            int can_added = alder_hashtable_mcaspseudo_can_increment(cht,
                                                                     ss->n,
                                                                     s3->n);
            if (can_added == 1) {
                possible_letter[n_possible_letter] = i;
                n_possible_letter++;
            }
        }
#if !defined(NDEBUG)
        alder_log5("Possible letters are (%d)", n_possible_letter);
        for (int i = 0; i < n_possible_letter; i++) {
            alder_log5("%c", alder_dna_int2char(possible_letter[i]));
        }
#endif
        
        if (n_possible_letter > 0 &&
            blength(bseq) < sequence_length) {
            // Add the letter to the sequence.
            int i = (int)Equilikely(0, n_possible_letter - 1);
            int b1 = possible_letter[i];
            int b2 = (b1 + 2) % 4;
            alder_encode_number_kmer_replace_left_with(s1, b1);
            alder_encode_number_kmer_replace_right_with(s2, b2);
            
            char c = alder_dna_int2char(b1);
            bconchar(bseq, c);
            alder_log5("Sequence: %s", bdata(bseq));
            
            alder_hashtable_mcas_t *cht = NULL;
            alder_encode_number_t *ss = NULL;
            uint64_t i_ni = 0;
            uint64_t i_np = 0;
            alder_kmer_simulate_select(&cht, &ss, &i_ni, &i_np, s1, s2, ht,
                                       number_iteration, number_partition);
            int added = alder_hashtable_mcaspseudo_increment(cht,
                                                             ss->n,
                                                             s3->n);
            if (added <= 0) {
                fprintf(stderr, "Fatal - failed to add a kmer!\n");
                goto cleanup;
            }
            assert(added > 0);
            alder_kmer_simulate_partition_write(opart, ss, (int)i_ni, (int)i_np);
        } else {
            // Write the sequence in a file.
            if (blength(bseq) >= kmer_size) {
                bstrVectorAddBstring(seqs, bseq);
            } else {
                assert(0);
            }
            // Start a new sequence.
            bseq = bfromcstr("");
            
            random_0123(random_kmer, kmer_size);
            alder_encode_number_kmer(s1, random_kmer);
            reverse_complementary_0123(random_kmer, kmer_size);
            alder_encode_number_kmer(s2, random_kmer);
            alder_encode_number_to_bstringWithOffset(s1, bseq, 1);
        }
#if !defined(NDEBUG)
        for (int i = 0; i < number_hash_table; i++) {
            alder_hashtable_mcas_t *cht = ht[i];
            for (int j = 0; j < alder_hashtable_mcas_size(cht); j++) {
                bstring bseqTable = alder_hashtable_mcas_key(cht, j);
                alder_log5("[%02d][%02d] %llu %llu %s", i, j,
                           alder_hashtable_mcas_value(cht, j),
                           alder_hashtable_mcas_key0(cht, j),
                           bdata(bseqTable));
                bdestroy(bseqTable);
            }
        }
#endif
        remaining_slot = 0;
        for (int i = 0; i < number_hash_table; i++) {
            remaining_slot += alder_hashtable_mcaspseudo_number_open_slot(ht[i]);
        }
        alder_log4("slot: %d", remaining_slot);
        // Now, all of the tables are filled.
        maximum_slot = 0;
        for (int i = 0; i < number_hash_table; i++) {
            int n = alder_hashtable_mcaspseudo_maximum_count(ht[i]);
            if (maximum_slot < n) {
                maximum_slot = n;
            }
        }
        alder_log5("max slot: %d", maximum_slot);
    }
    if (blength(bseq) >= kmer_size) {
        bstrVectorAddBstring(seqs, bseq);
    }
    assert(seqs->qty > 0);
    for (int i = 0; i < seqs->qty; i++) {
        alder_log5("seq[%d]: %s", i, bdata(seqs->entry[i]));
    }
    // Now, all of the tables are filled.
    maximum_slot = 0;
    for (int i = 0; i < number_hash_table; i++) {
        int n = alder_hashtable_mcaspseudo_maximum_count(ht[i]);
        if (maximum_slot < n) {
            maximum_slot = n;
        }
    }
    int seqs_original_qty = seqs->qty;
    
    while (maximum_slot < hashtable_maxcount) {
        // Choose a sequence and the position.
        int i_seq = (int)Equilikely(0, seqs_original_qty - 1);
        bstring bseqInfile = seqs->entry[i_seq];
        bseq = bstrcpy(seqs->entry[i_seq]);
        int n_kmer = blength(bseq) - kmer_size + 1;
        int p_seq = (int)Equilikely(0, n_kmer - 1);
        // sequence
        random_bseq(random_kmer, kmer_size, bseq, p_seq);
        alder_encode_number_kmer(s1, random_kmer);
        reverse_complementary_0123(random_kmer, kmer_size);
        alder_encode_number_kmer(s2, random_kmer);
        alder_encode_number_to_bstringWithOffset(s1, bseq, 0);
        alder_log5("seq: %d at %d, bseq: %s", i_seq, p_seq, bdata(bseq));
        
        // Add the current kmer first.
        alder_hashtable_mcas_t *cht = NULL;
        alder_encode_number_t *ss = NULL;
        uint64_t i_ni = 0;
        uint64_t i_np = 0;
        alder_kmer_simulate_select(&cht, &ss, &i_ni, &i_np, s1, s2, ht,
                                   number_iteration, number_partition);
        int added = alder_hashtable_mcaspseudo_increment(cht, ss->n, s3->n);
        assert(added > 0);
        alder_kmer_simulate_partition_write(opart, ss, (int)i_ni, (int)i_np);

        // Add kmers to hash tables.
        for (int i = p_seq; i < n_kmer - 1 && added < hashtable_maxcount; i++) {
            // Add the letter to the sequence.
            int b1 = alder_dna_char2int(bseqInfile->data[i + kmer_size]);
            int b2 = (b1 + 2) % 4;
            alder_encode_number_kmer_shift_left_with(s1, b1);
            alder_encode_number_kmer_shift_right_with(s2, b2);
            
            char c = alder_dna_int2char(b1);
            bconchar(bseq, c);
            alder_log5("Sequence: %s", bdata(bseq));
            
            alder_kmer_simulate_select(&cht, &ss, &i_ni, &i_np, s1, s2, ht,
                                       number_iteration, number_partition);
            added = alder_hashtable_mcaspseudo_increment(cht, ss->n, s3->n);
            assert(added > 0);
            alder_kmer_simulate_partition_write(opart, ss, (int)i_ni, (int)i_np);
        }
        if (blength(bseq) >= kmer_size) {
            bstrVectorAddBstring(seqs, bseq);
        } else {
            bdestroy(bseq);
        }
        
#if !defined(NDEBUG)
        for (int i = 0; i < number_hash_table; i++) {
            alder_hashtable_mcas_t *cht = ht[i];
            for (int j = 0; j < alder_hashtable_mcas_size(cht); j++) {
                alder_log5("[%02d][%02d] %llu %llu", i, j,
                           alder_hashtable_mcas_value(cht, j),
                           alder_hashtable_mcas_key0(cht, j));
            }
        }
#endif
        for (int i = 0; i < seqs->qty; i++) {
            alder_log5("seq[%d]: %s", i, bdata(seqs->entry[i]));
        }
        
        maximum_slot = 0;
        for (int i = 0; i < number_hash_table; i++) {
            int n = alder_hashtable_mcaspseudo_maximum_count(ht[i]);
            if (maximum_slot < n) {
                maximum_slot = n;
            }
        }
        alder_log4("max slot: %d", maximum_slot);
    }
    ///////////////////////////////////////////////////////////////////////////
    // Flush partition files.
    for (int i_ni = 0; i_ni < number_iteration; i_ni++) {
        for (int i_np = 0; i_np < number_partition; i_np++) {
            alder_kmer_simulate_partition_flush(opart, i_ni, i_np);
        }
    }
    /* END: Sequence files and partition files */
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Print files
    bstring bfilename = bformat("%s/%s", outdir, outfile);
    alder_fileseq_write_bstrListToFilename(bdata(bfilename), seqs, format,
                                           number_file);
    bdestroy(bfilename);
    
    bstrVectorDelete(seqs);
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Print the hash tables.
    bstring bht = bformat("%s/%s.tbl", outdir, outfile);
    FILE *fp = fopen(bdata(bht), "wb");
    alder_hashtable_mcas_printHeaderToFILE(fp,
                                           kmer_size,
                                           hashtable_size,
                                           number_iteration,
                                           number_partition);
    for (int i_ni = 0; i_ni < number_iteration; i_ni++) {
        for (int i_np = 0; i_np < number_partition; i_np++) {
            assert(0);
//            alder_hashtable_mcas_print_with_FILE(ht[i_np + i_ni * number_partition], fp);
        }
    }
    
    XFCLOSE(fp);
    
cleanup:
    /* Cleanup */
#if !defined(NDEBUG)
    bdestroy(bseq_debug);
#endif
    
    for (int i = 0; i < number_hash_table; i++) {
        if (ht[i] != NULL) {
            alder_hashtable_mcas_destroy(ht[i]);
        }
    }
    XFREE(ht);
    XFREE(random_kmer);
    alder_encode_number_destroy(s3);
    alder_encode_number_destroy(s2);
    alder_encode_number_destroy(s1);
    alder_kmer_simulate_partition_destroy(opart);
    return s;
}

/* This function generates sequence and partition files, but not a hash table
 * file. This is for checking the partitioning step. 
 *
 * 1. Each
 */
int alder_kmer_simulate_woHashtable(long version,
                                    const char *outfile,
                                    const char *outdir,
                                    alder_format_type_t format,
                                    int number_file,
                                    int number_iteration,
                                    int number_partition,
                                    int kmer_size,
                                    int sequence_length,
                                    size_t maxkmer)
{
    if (version > 1) {
        version = 2;
    }
    int s = 0;
    
    ///////////////////////////////////////////////////////////////////////////
    // Create a hash table.
    alder_hashtable_mcas_t *ht = alder_hashtable_mcas_create(kmer_size,
                                                             maxkmer * 2);
    
    
    ///////////////////////////////////////////////////////////////////////////
    // Open parition files.
    alder_kmer_simulate_partition_t * opart =
    alder_kmer_simulate_partition_create(outfile, outdir, kmer_size,
                                         number_iteration, number_partition);
    if (opart == NULL) return ALDER_STATUS_ERROR;
    
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
    for (int i = 0; i < number_file; i++) {
        
        
        bstring bfilename;
        if (format == ALDER_FORMAT_FASTA) {
            bfilename = bformat("%s/%s-%02d.fa", outdir, outfile, i);
        } else if (format == ALDER_FORMAT_FASTQ) {
            bfilename = bformat("%s/%s-%02d.fq", outdir, outfile, i);
        } else {
            bfilename = bformat("%s/%s-%02d.seq", outdir, outfile, i);
        }
        FILE *fpseq = fopen(bdata(bfilename), "w");
        
        int i_sequence_length = 0;
        size_t n_kmer = 0;
        while (n_kmer < maxkmer) {
            if (n_kmer % 100000 == 1) {
                alder_progress_step((i * maxkmer) + n_kmer,
                                    number_file * maxkmer,
                                    0);
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
            alder_kmer_simulate_select_woHashtable(&ss, &i_ni, &i_np, s1, s2,
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
            
            if (version == 1) {
                alder_kmer_simulate_partition_write(opart, ss,
                                                    (int)i_ni, (int)i_np);
            } else if (version == 2) {
                alder_kmer_simulate_partition_write2(opart, s2s,
                                                     (int)i_ni, (int)i_np);
            } else {
                assert(0);
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
        
        if (version == 1) {
            for (int i_ni = 0; i_ni < number_iteration; i_ni++) {
                for (int i_np = 0; i_np < number_partition; i_np++) {
                    alder_kmer_simulate_partition_flush(opart, i_ni, i_np);
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
        
        XFCLOSE(fpseq);
        bdestroy(bfilename);
    }
    alder_progress_end(0);

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



void alder_kmer_simulate_select(alder_hashtable_mcas_t **cht,
                                alder_encode_number_t **ss,
                                uint64_t *i_ni,
                                uint64_t *i_np,
                                alder_encode_number_t *s1,
                                alder_encode_number_t *s2,
                                alder_hashtable_mcas_t **ht,
                                int number_iteration,
                                int number_partition)
{
    uint64_t hash_s1 = alder_encode_number_hash(s1);
    uint64_t hash_s2 = alder_encode_number_hash(s2);
    uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
    *ss = (hash_s1 < hash_s2) ? s1 : s2;
    
    *i_ni = hash_ss % number_iteration;
    *i_np = hash_ss / number_iteration % number_partition;
    uint64_t selected_ht = *i_np + *i_ni * number_partition;
    alder_log5("hash1: %llu", hash_s1);
    alder_log5("hash2: %llu", hash_s2);
    alder_log5("hashs: %llu", hash_ss);
    alder_log5("ni: %llu", *i_ni);
    alder_log5("np: %llu", *i_np);
    alder_log5("ht: %llu", selected_ht);
    
    *cht = ht[selected_ht];
}

void alder_kmer_simulate_select_woHashtable(alder_encode_number_t **ss,
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


#pragma mark alder_kmer_simulate_partition_t

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

/* See the struct alder_kmer_simulate_partition_t.
 * It is pretty boring to look inside struct create, and destroy functions.
 */
void alder_kmer_simulate_partition_destroy(alder_kmer_simulate_partition_t *o)
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

void alder_kmer_simulate_partition_write(alder_kmer_simulate_partition_t *o,
                                         alder_encode_number_t *n1,
                                         int i_ni, int i_np)
{
    int k = i_ni * o->n_np + i_np;
    FILE *fp = o->fp[k];
    alder_encode_number_t *n2 = o->n[k];
    int nk = o->nk[k];
    alder_encode_number_packwrite(fp, n1, n2, &nk);
    o->nk[k] = nk;
}

void alder_kmer_simulate_partition_write2(alder_kmer_simulate_partition_t *o,
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

/* This function flushes the content in the buffer.
 */
void alder_kmer_simulate_partition_flush(alder_kmer_simulate_partition_t *o,
                                         int i_ni, int i_np)
{
    int k = i_ni * o->n_np + i_np;
    FILE *fp = o->fp[k];
    alder_encode_number_t *n2 = o->n[k];
    int nk = o->nk[k];
    alder_encode_number_packflush(fp, n2, &nk);
    assert(nk == 0);
    o->nk[k] = nk;
}

/* This function converts bstring bseq to an array of int.
 */
static void random_bseq(uint8_t *s, size_t n, bstring bseq, size_t offset)
{
    for (size_t i = 0; i < n; i++) {
        s[i] = alder_dna_char2int(bseq->data[offset + i]);
    }
}

/* This function makes an array of uint8_t with randomly chosen numbers
 * among 4 numbers from 0 to 3.
 */
static void random_0123(uint8_t *s, size_t n)
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
static int reverse_complementary_0123(uint8_t *s, size_t n)
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

