/**
 * This file, alder_kmer_thread8.c, is part of alder-kmer.
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

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <zlib.h>
#include "bzlib.h"
#include "bstrlib.h"
#include "rngs.h"
#include "alder_logger.h"
#include "alder_hash.h"
#include "alder_cmacro.h"
#include "alder_progress.h"
#include "alder_encode.h"
#include "alder_file.h"
#include "alder_bit.h"
#include "alder_fileseq.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_chunk.h"
#include "alder_kmer.h"
#include "alder_mcas_wf.h"
#include "alder_hashtable.h"
#include "alder_kmer_thread8.h"

/**
 *  Structure of buffers
 *  --------------------
 *  inbuf: There are [n_subbuf]-many sub buffers. The main inbuf does not have
 *         any header. But, each sub buffer does;
 *         state (1B) partition index (8B), and length (8B).
 *         So, the body start at 17B. The number of sub buffers is determined
 *         by the number of threads. Th size of buffer after this header is
 *         fixed at BUFSIZE + b, where b is the number of bytes for a kmer.
 *         The last byte is not used because I'd save only b-1 bytes.
 */

static int counter_id_counter = 0;

#define NO_READ               2

#define INBUFFER_TYPE         1
#define INBUFFER_CURRENT      2
#define INBUFFER_LENGTH       10
#define INBUFFER_BODY         18

#define SECONDARY_BUFFER_SIZE 1000
#define FILETYPE_FASTA        1
#define FILETYPE_FASTQ        2
#define FILETYPE_SEQ          3

#define ALDER_KMER_TABLE_FULL70 2

static int dna_char2int[128] = {
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,0,4,1,4,4,4,3,4,4,4,4,4,4,4,4,
    4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,4,
    4,0,4,1,4,4,4,3,4,4,4,4,4,4,4,4,
    4,4,4,4,2,4,4,4,4,4,4,4,4,4,4,4
};

static void *counter(void *t);

/**
 *  This function creates a bstring of a partition file name.
 *  The caller is responsible for freeing the bstring. The infile can be NULL.
 *  When partition files are created, the infile partition names need to be
 *  created. This is usually the case the whole procedure is done by count
 *  command. If infile is not NULL, the list of partition files is considered
 *  a list of partition files.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return partition file name or NULL
 */
//static bstring
//filename_partition(alder_kmer_thread8_t *o, uint64_t i_np)
//{
//    /* File name setup */
//    bstring bfpar = NULL;
//    if (o->infile == NULL) {
//        bfpar = bformat("%s/%s-%llu-%llu.par",
//                        bdata(o->boutdir), bdata(o->boutfile),
//                        o->i_ni, i_np);
//        if (bfpar == NULL) return NULL;
//    } else {
//        bfpar = bstrcpy(o->infile->entry[i_np]);
//        if (bfpar == NULL) return NULL;
//    }
//    return bfpar;
//}

/**
 *  This function computes the number of buffer blocks in a par file.
 *  This updates n_blockByReader, which denotes the number of input buffer
 *  blocks for a par file. This number is compared with one in n_blockByCounter
 *  to check whether I have reached a final input buffer block.
 *
 *  @param o    counter
 *  @param i_np partition index
 *
 *  @return SUCCESS or FAIL
 */
//static int
//compute_number_block(alder_kmer_thread8_t *o, uint64_t i_np)
//{
//    o->n_blockByReader[i_np] = 0;
//    bstring bfpar = filename_partition(o, i_np);
//    if (bfpar == NULL) {
//        return ALDER_STATUS_ERROR;
//    }
//    
//    /* Compute the number of buffer blocks in the file. */
//    size_t file_size = 0;
//    alder_file_size(bdata(bfpar), &file_size);
//    o->n_blockByReader[i_np] = (int)ALDER_BYTESIZE_KMER(file_size,
//                                                        o->size_readbuffer);
//    bdestroy(bfpar);
//    return ALDER_STATUS_SUCCESS;
//}

/**
 *  This function frees the memory allocated for the following variables.
 *  1. next_inbuf
 *  2. inbuf
 *  3. n_blockByReader
 *  4. n_blockByCounter
 *  5. ht
 *  6. boutfile
 *  7. boutdir
 *  8. fpin
 *  9. n_t_byte
 * 10. n_i_byte
 * 11. n_i_kmer
 *
 *  @param o counter
 */
static void
alder_kmer_thread8_destroy(alder_kmer_thread8_t *o)
{
    if (o != NULL) {
        /* o->infile is not owned. */
        XFREE(o->n_i_kmer);
        XFREE(o->n_i_byte);
        XFREE(o->n_t_byte);
        XFCLOSE(o->fpin);
        bdestroy(o->boutdir);
        bdestroy(o->boutfile);
        alder_hashtable_mcas2_destroy(o->ht);
        XFREE(o->inbuf);
        XFREE(o->next_inbuf);
        XFREE(o);
    }
}

/**
 *  This function creates a counter.
 *
 *  @param fpout                table file for write
 *  @param n_counter            number of counter threads
 *  @param i_ni                 iteration index
 *  @param kmer_size            K
 *  @param memory_available     M
 *  @param sizeInbuffer         inbuffer size
 *  @param sizeOutbuffer        outbuffer size
 *  @param n_ni                 number of iterations
 *  @param n_np                 number of partitions
 *  @param n_nh                 number of hash elements in a table
 *  @param totalfilesize        [not used]
 *  @param n_byte               number of bytes to process
 *  @param n_total_kmer         total number of kmers to process
 *  @param n_current_kmer       current number of kmers
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to stderr
 *  @param nopack_flag          [not used]
 *  @param infile               infile
 *  @param outdir               out directory
 *  @param outfile              out file name
 *
 *  @return counter
 */
static alder_kmer_thread8_t *
alder_kmer_thread8_create(FILE *fpout,
                          int n_counter,
                          int kmer_size,
                          long memory_available,
                          long sizeInbuffer,
                          long sizeOutbuffer,
                          size_t n_nh,
                          int lower_count,
                          int upper_count,
                          size_t n_byte,
                          size_t n_total_kmer,
                          size_t n_current_kmer,
                          int progress_flag,
                          int progressToError_flag,
                          int format_flag,
                          struct bstrList *infile,
                          const char *outdir,
                          const char *outfile)
{
    alder_log5("Creating a counter ...");
    int s;
    alder_kmer_thread8_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create a counter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    
    o->size_readbuffer = sizeInbuffer;
    o->size_writebuffer = sizeOutbuffer;
    if (!strcmp(bdata(infile->entry[0]), "-")) {
        switch (format_flag) {
            case 0:
                o->reader_type_infile = FILETYPE_FASTA;
                o->type_infile = ALDER_FILETYPE_FASTA;
                break;
            case 1:
                o->reader_type_infile = FILETYPE_FASTQ;
                o->type_infile = ALDER_FILETYPE_FASTQ;
                break;
            case 2:
                o->reader_type_infile = FILETYPE_SEQ;
                o->type_infile = ALDER_FILETYPE_SEQ;
                break;
            default:
                o->reader_type_infile = FILETYPE_FASTQ;
                o->type_infile = ALDER_FILETYPE_FASTQ;
                break;
        }
    }
    
    /* init */
    o->k = kmer_size;
    o->b = alder_encode_number2_bytesize(kmer_size);
    o->n_counter = n_counter;
    
    o->next_lenbuf = 0;
    o->next_inbuf = NULL;
    o->reader_lenbuf = 0;
    o->size_inbuf2 = SECONDARY_BUFFER_SIZE;
    o->inbuf2 = NULL;
    
    o->size_subinbuf = (INBUFFER_BODY + o->size_readbuffer +
                        SECONDARY_BUFFER_SIZE);
    o->size_inbuf = o->size_subinbuf * n_counter;
    o->inbuf = NULL;
    o->ht = NULL;
    o->infile = infile;
    o->fpout = fpout;
    o->boutfile = NULL;
    o->boutdir = NULL;
    o->fpin = NULL;
    o->n_t_byte = NULL;
    o->n_i_byte = NULL;
    o->n_i_kmer = NULL;
    o->n_byte = 0;
    o->n_kmer = 0;
    o->n_hash = 0;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    o->isMultithreaded = (n_counter > 1);
    
    /* memory */
    o->next_inbuf = malloc(sizeof(*o->next_inbuf) * o->b);
    o->inbuf = malloc(sizeof(*o->inbuf) * o->size_inbuf);
    o->boutfile = bfromcstr(outfile);
    o->boutdir = bfromcstr(outdir);
    o->n_t_byte = malloc(sizeof(*o->n_t_byte) * n_counter);
    o->n_i_byte = malloc(sizeof(*o->n_i_byte) * n_counter);
    o->n_i_kmer = malloc(sizeof(*o->n_i_kmer) * n_counter);
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * o->size_inbuf2);
    if (o->inbuf == NULL || o->next_inbuf == NULL ||
        o->boutfile == NULL || o->boutdir == NULL ||
        o->n_i_kmer == NULL || o->n_i_byte == NULL || o->n_t_byte == NULL) {
        alder_kmer_thread8_destroy(o);
        return NULL;
    }
    memset(o->next_inbuf, 0,sizeof(*o->next_inbuf) * o->b);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * o->size_inbuf);
    memset(o->inbuf2, 0, sizeof(*o->inbuf2) * o->size_inbuf2);
    memset(o->n_t_byte, 0, sizeof(*o->n_t_byte) * n_counter);
    memset(o->n_i_byte, 0, sizeof(*o->n_i_byte) * n_counter);
    memset(o->n_i_kmer, 0, sizeof(*o->n_i_kmer) * n_counter);
    assert(o->fpin == NULL);
    
    /* Setup a table. */
    size_t size_table = 0;
    int size_stride = 0;
    size_t size_memory = (size_t)memory_available << 20;
    s = alder_hashtable_mcas2_size_with_available_memory(&size_table,
                                                         &size_stride,
                                                         kmer_size,
                                                         size_memory);
    if (n_nh == 0) {
        n_nh = size_table;
    }
    o->ht = alder_hashtable_mcas2_create(kmer_size,
                                         n_nh,
                                         size_stride,
                                         lower_count,
                                         upper_count); /* bigmem */
    
    return o;
}

#pragma mark main

static pthread_mutex_t mutex_read;

/**
 *  This function implements a 2nd version of counters.
 *
 *  @param fpout                file out
 *  @param n_counter            number of counters
 *  @param i_ni                 iteration index
 *  @param kmer_size            kmer size
 *  @param memory_available     memory
 *  @param sizeInbuffer         input buffer size
 *  @param n_ni                 iterations
 *  @param n_np                 partitions
 *  @param n_nh                 hash table size
 *  @param totalfilesize        total file size
 *  @param n_byte               return number of bytes
 *  @param n_hash               return number of kmers
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to std err
 *  @param nopack_flag          no pack
 *  @param infile               NULL when counting
 *  @param outdir               output directory
 *  @param outfile              output file name
 *
 *  @return SUCCESS or FAIL
 */
int alder_kmer_thread8(FILE *fpout,
                       int n_counter,
                       int i_ni,
                       int kmer_size,
                       long memory_available,
                       long sizeInbuffer,
                       long sizeOutbuffer,
                       uint64_t n_ni,
                       uint64_t n_np,
                       size_t n_nh,
                       int lower_count,
                       int upper_count,
                       size_t *n_byte,
                       size_t *n_kmer,
                       size_t *n_hash,
                       size_t n_total_kmer,
                       size_t *n_current_kmer,
                       int progress_flag,
                       int progressToError_flag,
                       int format_flag,
                       uint8_t *inbuf,
                       size_t size_data,
                       struct bstrList *infile,
                       const char *outdir,
                       const char *outfile)
{
    alder_log5("preparing counting Kmers with version 8...");
    counter_id_counter = 0;
    int s = 0;
    if (n_counter > 1 && kmer_size > 31) {
        mcas_init(n_counter);
    }
    
    alder_kmer_thread8_t *data =
    alder_kmer_thread8_create(fpout,
                              n_counter,
                              kmer_size,
                              memory_available,
                              sizeInbuffer,
                              sizeOutbuffer,
                              n_nh,
                              lower_count,
                              upper_count,
                              *n_byte,
                              n_total_kmer,
                              *n_current_kmer,
                              progress_flag,
                              progressToError_flag,
                              format_flag,
                              infile,
                              outdir,
                              outfile);
    if (data == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "failed to creat counter threads.");
        return ALDER_STATUS_ERROR;
    }
    alder_log5("creating %d threads: one for reader, and one for counter",
               n_counter);
    
    pthread_t *threads = malloc(sizeof(*threads)*n_counter);
    if (data == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create threads for counting");
        XFREE(threads);
        alder_kmer_thread8_destroy(data);
        return ALDER_STATUS_ERROR;
    }
    memset(threads,0,sizeof(*threads)*n_counter);
    
    pthread_attr_t attr;
    s += pthread_attr_init(&attr);
    s += pthread_mutex_init(&mutex_read, NULL);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < n_counter; i++) {
        s += pthread_create(&threads[i], &attr, counter, (void *)data);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "cannot create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    
    /* Wait for all threads to complete */
    alder_log3("main(): waiting for all threads to complete...");
    for (int i = 0; i < n_counter; i++) {
        s += pthread_join(threads[i], NULL);
        if (s != 0) {
            alder_loge(ALDER_ERR_THREAD, "cannot join threads - %s",
                       strerror(errno));
            goto cleanup;
        }
    }
    
cleanup:
    /* Cleanup */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&mutex_read);
    XFREE(threads);
    
    data->n_byte = 0;
    for (int i = 0; i < n_counter; i++) {
        data->n_byte += data->n_i_byte[i];
    }
    data->n_kmer = 0;
    for (int i = 0; i < n_counter; i++) {
        data->n_kmer += data->n_i_kmer[i];
    }
    alder_log2("counter()[%d] Kmers: %zu", i_ni, data->n_kmer);
    alder_log2("counter()[%d) Bytes: %zu", i_ni, data->n_byte);
    *n_byte += data->n_byte;
    *n_kmer += data->n_kmer;
    *n_hash += data->n_hash;
    *n_current_kmer += data->n_kmer;
    
    alder_kmer_thread8_destroy(data);
    alder_log5("Counting Kmers has been finished with %d threads.", n_counter);
    return 0;
}

#pragma mark buffer

/**
 *  This function assign incremental numbers to counter threads.
 *
 *  @return assigned counter id
 */
static int
assign_counter_id()
{
    int oval = counter_id_counter;
    int cval = oval + 1;
    while (cval != oval) {
        oval = counter_id_counter;
        cval = oval + 1;
        cval = __sync_val_compare_and_swap(&counter_id_counter,
                                           (int)oval, (int)cval);
    }
    return oval;
}

#pragma mark thread


/**
 *  This function returns 0,1,2,3 for DNA.
 *
 *  @param e encoder
 *
 *  @return int
 *  0,1,2,3 for A,C,T,G.
 *  4 for others.
 *  5 for the end of buffer.
 */
static int
read_token(size_t *curbuf, uint8_t *inbuf, size_t lenbuf, int type_infile)
{
    char c;
    size_t vcurbuf = *curbuf;
#if !defined(NDEBUG)
    if (vcurbuf % 10000 == 0) {
        alder_log4("i_inbuf(%zu): %zu", lenbuf, vcurbuf);
    }
#endif
    if (vcurbuf < lenbuf) {
        c = inbuf[vcurbuf++];
    } else {
        //        assert(0);
        *curbuf = vcurbuf;
        return 5;
    }
    if (type_infile == FILETYPE_FASTA) {
        if (c == '>' || c == '\n') {
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            if (c == '>') {
                uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
                vcurbuf = (size_t)(cinbuf - inbuf + 1);
            }
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            if (vcurbuf < lenbuf) {
                c = inbuf[vcurbuf++];
            } else {
                //                e->n_byte += *curbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    } else if (type_infile == FILETYPE_FASTQ) {
        if (c == '@') {
            // header
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            c = inbuf[vcurbuf++];
            assert(c != '\n');
        } else if (c == '\n') {
            // +
            c = inbuf[vcurbuf++];
            assert(c == '+');
            
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            uint8_t *cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            cinbuf = memchr(inbuf + vcurbuf, '\n', lenbuf - vcurbuf);
            vcurbuf = (size_t)(cinbuf - inbuf + 1);
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            // quality score
            //            c = inbuf[vcurbuf++];
            //            while (c != '\n') {
            //                c = inbuf[vcurbuf++];
            //            }
            /*****************************************************************/
            /*                          OPTIMIZATION                         */
            /*****************************************************************/
            
            if (vcurbuf < lenbuf) {
                c = 'X';
            } else {
                //                e->n_byte += e->i_inbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    } else {
        assert(type_infile == FILETYPE_SEQ);
        if (c == '\n') {
            if (vcurbuf < lenbuf) {
                // no code.
            } else {
                //                e->n_byte += e->i_inbuf;
                *curbuf = vcurbuf;
                return 5;
            }
        }
    }
    
    *curbuf = vcurbuf;
    return dna_char2int[c];
    //    int ci = alder_dna_char2int(c);
    //    return ci;
}



/**
 *  This function writes the i_np-th table to a file.
 *
 *  @param a       counter
 *  @param c_table table id
 *  @param i_np    partition id
 *
 *  @return SUCCESS or FAIL
 */
static __attribute__ ((noinline)) int
write_tabfile(alder_kmer_thread8_t *a)
{
    // c_table of main i_np must enter first.
    int s = ALDER_STATUS_SUCCESS;
    
    // Use counter id to find the table from the input buffer.
    int64_t s_count = alder_hashtable_mcas2_printPackToFD(a->ht,
                                                          fileno(a->fpout),
                                                          a->size_writebuffer);
    if (s_count < 0) {
        alder_loge(ALDER_ERR_HASH, "failed to save a hash table");
        // FIXME: return an error code
        return ALDER_STATUS_ERROR;
    }
    a->n_hash += s_count;
    assert(s == ALDER_STATUS_SUCCESS);
    
    /* Reset the table. */
    alder_hashtable_mcas2_reset(a->ht);
    
    /* Change the main i_np. */
//    a->main_i_np++;
    
    if (a->progress_flag) {
        size_t c = a->n_current_kmer;
        for (int i = 0; i < a->n_counter; i++) {
            c += a->n_i_kmer[i];
        }
        alder_progress_step(c, a->n_total_kmer, a->progressToError_flag);
    }
    
    return s;
}

///**
// *  This function reads a block of partition files.
// *
// *  @param a          counter
// *  @param counter_id counter id
// *
// *  @return SUCCESS or FAIL or ALDER_KMER_THREAD7_NO_READ
// */
//static __attribute__ ((noinline)) int
//read_parfile(alder_kmer_thread8_t *a, int counter_id)
//{
//    int s = ALDER_STATUS_SUCCESS;
//    pthread_mutex_lock(&mutex_read);
//    
//    if (a->reader_lenbuf == 0) {
//        if (a->reader_i_parfile > 0) {
//            close_parfile(a);
//        }
//        if (a->reader_i_parfile < a->n_np) {
//            s = open_parfile(a, a->reader_i_parfile);
//            if (s != ALDER_STATUS_SUCCESS) {
//                return s;
//            }
//            a->reader_i_parfile++;
//        } else {
//            // No more input file to read.
//            pthread_mutex_unlock(&mutex_read);
//            return NO_READ;
//        }
//    }
//    
//    /* Read a block of input buffer. */
//    ssize_t lenbuf = 0;
//    uint8_t *inbuf = a->inbuf + counter_id * a->size_subinbuf;
//    uint8_t *inbuf_body = inbuf + INBUFFER_BODY;
//    size_t cur_next_lenbuf = a->next_lenbuf;
//    if (a->next_lenbuf > 0) {
//        memcpy(inbuf_body, a->next_inbuf, a->next_lenbuf);
//        inbuf_body += a->next_lenbuf;
//    }
//    lenbuf = read(fileno(a->fpin), inbuf_body, a->size_readbuffer);
//    assert(lenbuf == 0 || lenbuf >= a->b);
//    if (lenbuf > 0 && a->b > 1) {
//        assert(cur_next_lenbuf + lenbuf >= a->b);
//        a->next_lenbuf = (cur_next_lenbuf + lenbuf) % a->b;
//        memcpy(a->next_inbuf, inbuf_body + lenbuf - a->next_lenbuf, a->next_lenbuf);
//        a->reader_lenbuf = cur_next_lenbuf + lenbuf - a->next_lenbuf;
//    } else if (lenbuf > 0 && a->b == 1) {
//        a->next_lenbuf = 0;
//        a->reader_lenbuf = lenbuf;
//    } else if (lenbuf == 0) {
//        assert(cur_next_lenbuf == 0);
//        a->reader_lenbuf = lenbuf;
//    } else {
//        // Error in reading.
//        assert(0);
//        abort();
//    }
//    assert(a->reader_i_parfile <= a->n_np);
//    
//    to_uint64(inbuf,INBUFFER_I_NP) = a->reader_i_parfile - 1;
//    to_size(inbuf,INBUFFER_LENGTH) = a->reader_lenbuf;
//    assert(a->reader_lenbuf % a->b == 0);
//    
//    /* Progress */
//    a->n_byte += lenbuf;
//    
//    pthread_mutex_unlock(&mutex_read);
//    return ALDER_STATUS_SUCCESS;
//}

/**
 *  This function returns 0,1,2 depending on the type of an input file.
 *
 *  @param a encoder
 *
 *  @return 0 for FASTA, 1 for FASTQ, 2 otherwise.
 */
static uint8_t
what_type_infile(alder_kmer_thread8_t *a)
{
    uint8_t type_infile;
    if (a->type_infile & ALDER_FILETYPE_ISFASTA) {
        type_infile = FILETYPE_FASTA;
    } else if (a->type_infile & ALDER_FILETYPE_ISFASTQ) {
        type_infile = FILETYPE_FASTQ;
    } else {
        type_infile = FILETYPE_SEQ;
    }
    return type_infile;
}


/**
 *  This function closes a partition file.
 *
 *  @param o        encoder
 *
 *  @return SUCCESS or FAIL
 */
static int close_seqfile (alder_kmer_thread8_t *o)
{
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    
    if (o->fx != NULL) {
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            gzclose(o->fx);
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            BZ2_bzclose(o->fx);
        } else {
            close((int)((intptr_t)o->fx));
        }
    }
    return ALDER_STATUS_SUCCESS;
}

/**
 *  This function opens a partition file.
 *
 *  @param o        encoder
 *  @param i_infile input file index
 *
 *  @return SUCCESS or FAIL
 */
static int open_seqfile (alder_kmer_thread8_t *o, int i_infile)
{
    int fp = -1;
    gzFile fpgz = NULL;
    BZFILE *fpbz = NULL;
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    
    char *fn = bdata(o->infile->entry[i_infile]);
    if (strcmp(fn, "-")) {
        o->type_infile = alder_fileseq_type(fn);
        
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            fpgz = gzopen(fn, "r");
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            fpbz = BZ2_bzopen(fn, "r");
        } else {
            fp = open(fn, O_RDONLY);
        }
        if (fp < 0 && fpgz == Z_NULL && fpbz == NULL) {
            alder_loge(ALDER_ERR_FILE, "cannot open file %s - %s",
                       fn, strerror(errno));
            return ALDER_STATUS_ERROR;
        }
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            o->fx = fpgz;
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            o->fx = fpbz;
        } else {
            o->fx = (void *)(intptr_t)fp;
        }
        o->reader_type_infile = what_type_infile(o);
    } else {
        // STDIN
        fp = STDIN_FILENO;
        o->fx = (void *)(intptr_t)fp;
    }
    return ALDER_STATUS_SUCCESS;
}


static __attribute__ ((noinline)) int
read_seqfile(alder_kmer_thread8_t *a, int encoder_id)
{
    int s;
    pthread_mutex_lock(&mutex_read);
    
    if (a->reader_lenbuf == 0) {
        if (a->reader_i_infile > 0) {
            close_seqfile(a);
        }
        if (a->reader_i_infile < a->infile->qty) {
            s = open_seqfile(a, a->reader_i_infile);
            assert(s == ALDER_STATUS_SUCCESS);
            a->reader_i_infile++;
            //            fprintf(stdout, "read_i_infile: %d\n", a->reader_i_infile);
            a->reader_lenbuf2 = 0;
        } else {
            // No more input file to read.
            // I may still have some outbuf.
            // I would call writer_to_partition_file after while-loop
            // in encoder function.
            
            pthread_mutex_unlock(&mutex_read);
            return (int)a->n_counter;
        }
    }
    assert(a->reader_i_infile <= a->infile->qty);
    
    uint8_t *inbuf = a->inbuf + encoder_id * a->size_subinbuf;
    inbuf[INBUFFER_TYPE] = a->reader_type_infile;
    uint8_t *inbuf_body = inbuf + INBUFFER_BODY;
    size_t size_body = a->size_subinbuf - INBUFFER_BODY;
    assert(size_body == a->size_readbuffer + SECONDARY_BUFFER_SIZE);
    s = alder_fileseq_chunk(&a->reader_lenbuf,
                            (char*)inbuf_body,
                            a->size_readbuffer,
                            &a->reader_lenbuf2,
                            (char*)a->inbuf2,
                            a->size_inbuf2,
                            a->k, NULL, a->fx, a->type_infile);
    if (s == ALDER_STATUS_SUCCESS) {
        assert(a->reader_lenbuf > 0);
    } else {
        assert(a->reader_lenbuf == 0);
    }
    to_size(inbuf, INBUFFER_CURRENT) = (size_t)0;
    to_size(inbuf, INBUFFER_LENGTH) = a->reader_lenbuf;
    
    alder_log4("encoder(%d): read %zu (MB)", encoder_id, a->reader_lenbuf >> 20);
    
    /* Progress */
    a->n_byte += a->reader_lenbuf;
    if (a->progress_flag) {
        alder_progress_step(a->n_byte, a->n_total_byte,
                            a->progressToError_flag);
    }
    
    pthread_mutex_unlock(&mutex_read);
    return encoder_id;
}

/**
 *  This function is for the counter thread.
 *
 *  @param t argument
 *
 *  @return SUCCESS or FAIL
 */
static void *counter(void *t)
{
    int counter_id = assign_counter_id();
    int s = ALDER_STATUS_SUCCESS;
    int s_table = ALDER_STATUS_SUCCESS;
    alder_kmer_thread8_t *a = (alder_kmer_thread8_t*)t;
    assert(a != NULL);
    alder_log2("counter(%d): START", counter_id);
    
    /* Numbers for encoding/decoding kmers, and buffers for reading. */
    int K = a->k;
    alder_encode_number2_t *s1 = alder_encode_number2_createWithKmer(K);
    alder_encode_number2_t *s2 = alder_encode_number2_createWithKmer(K);
    alder_encode_number_t *m = alder_encode_number_create_for_kmer(K);
    size_t *len_outbuf2 = malloc(sizeof(*len_outbuf2));
    uint64_t *res_key = malloc(sizeof(*res_key) * m->s);
    uint64_t *res_key2 = malloc(sizeof(*res_key) * m->s);
    if (m == NULL || res_key == NULL || res_key2 == NULL ||
        s1 == NULL || s2 == NULL || len_outbuf2 == NULL) {
        goto cleanup;
        alder_loge(ALDER_ERR_MEMORY, "Fatal - counter(): not enough memory");
        pthread_exit(NULL);
    }
    memset(res_key, 0, sizeof(*res_key) * m->s);
    memset(res_key2, 0, sizeof(*res_key2) * m->s);
    memset(len_outbuf2, 0, sizeof(*len_outbuf2));
    size_t kb = ALDER_BYTESIZE_KMER(s1->b, 8);
    
#if !defined(NDEBUG)
    size_t debug_counter = 0;
#endif
    
    /* Let's read inputs, and skip writing table files. */
    alder_hashtable_mcas2_t *c_ht = a->ht;
    int token = 5; // Start with a new Kmer.
    uint8_t *inbuf = a->inbuf + counter_id * a->size_subinbuf;
    uint8_t *inbuf_body = inbuf + INBUFFER_BODY;
    size_t curbuf = 0;
    size_t lenbuf = 0;
    int c_inbuffer = a->n_counter;
    
    ///////////////////////////////////////////////////////////////////////////
    // Fill the table upto 70%.
    ///////////////////////////////////////////////////////////////////////////
    while (1) {
        
        /* setup of inbuf */
        if (c_inbuffer == a->n_counter) {
            s = read_seqfile(a, counter_id);
        }
        int type_infile = (int)inbuf[INBUFFER_TYPE];
        if (c_inbuffer == a->n_counter) {
            curbuf = to_size(inbuf, INBUFFER_CURRENT);
            lenbuf = to_size(inbuf, INBUFFER_LENGTH);
            alder_log3("encoder(%d): fresh new input buffer");
            token = 4;
        } else {
            alder_log3("encoder(%d): use the previous input buffer");
        }
        if (lenbuf == 0) {
            break;
        }
        
        /* 3. While either input is not empty or output is not full: */
        while (token < 5) {
            /* 4. Form a Kmer and its reverse complementary. */
            if (token == 4) {
                token = 0;
                alder_log5("A starting Kmer is being created.");
                for (int i = 0; i < a->k - 1; i++) {
                    token = read_token(&curbuf, inbuf_body,
                                       lenbuf, type_infile);
                    int b1 = token;
                    int b2;
                    if (b1 == 0) { b2 = 2; } else if (b1 == 1) { b2 = 3; }
                    else if (b1 == 2) { b2 = 0; } else if (b1 == 3) { b2 = 1; }
                    else { break; }
                    alder_encode_number2_shiftLeftWith(s1,b1);
                    alder_encode_number2_shiftRightWith(s2,b2);
                }
                if (token >= 4) { continue; }
            }
            token = read_token(&curbuf, inbuf_body, lenbuf, type_infile);
            if (token >= 4) continue;
            alder_log5("token: %d", token);
            int b1 = token;
            int b2;
            if (b1 == 0) { b2 = 2; } else if (b1 == 1) { b2 = 3; }
            else if (b1 == 2) { b2 = 0; } else if (b1 == 3) { b2 = 1; }
            else { assert(0); }
            alder_encode_number2_shiftLeftWith(s1,b1);
            alder_encode_number2_shiftRightWith(s2,b2);
            
            /* 5. Select kmer of the two. */
            alder_encode_number2_t *ss = NULL;
            uint64_t hash_s1 = alder_encode_number2_hash(s1);
            uint64_t hash_s2 = alder_encode_number2_hash(s2);
            uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
            ss = (hash_s1 < hash_s2) ? s1 : s2;
            for (size_t i = 0; i < kb; i++) {
                m->n[i] = ss->n[i].key64;
            }
            
            /* 6. Add the kmer to the table. */
            uint64_t added_x = c_ht->size;
            s = alder_hashtable_mcas2_increment_hashcode(c_ht, m->n,
                                                         hash_ss,
                                                         res_key,
                                                         &added_x,
                                                         a->isMultithreaded);
#if !defined(NDEBUG)
            alder_hashtable_mcas2_debug_print(c_ht,
                                              m->n,
                                              hash_ss,
                                              added_x,
                                              c_ht->size,
                                              bdata(a->boutdir),
                                              bdata(a->boutfile));
#endif
            if (s == ALDER_KMER_TABLE_FULL70) {
                s_table = ALDER_KMER_TABLE_FULL70;
                to_size(inbuf, INBUFFER_CURRENT) = curbuf;
                break;
            } else if (s != ALDER_STATUS_SUCCESS) {
                alder_loge(ALDER_ERR, "Fatal - table is full!");
                assert(0);
                abort();
            }
        }
        
        if (s_table == ALDER_KMER_TABLE_FULL70) {
            c_inbuffer = counter_id;
            break;
        }
        
        if (token == 5) {
            c_inbuffer = a->n_counter;
        } else {
            c_inbuffer = counter_id;
        }
    }
    
    if (s_table != ALDER_KMER_TABLE_FULL70) {
        alder_log("Table is not full, and is finished.");
        goto cleanup;
    }
    
    // All of the threads must wait here.
    // Use a counter to check if all of the threads reach here.
#if !defined(NDEBUG)
//    alder_hashtable_mcas2_printToFILE(c_ht, stdout);
#endif
    alder_hashtable_mcas2_set_pointer_to_tail(c_ht);
    
    /* Delete old keys when adding a new key. */
    while (1) {
        /* setup of inbuf */
        if (c_inbuffer == a->n_counter) {
            s = read_seqfile(a, counter_id);
        }
        int type_infile = (int)inbuf[INBUFFER_TYPE];
        if (c_inbuffer == a->n_counter) {
            curbuf = to_size(inbuf, INBUFFER_CURRENT);
            lenbuf = to_size(inbuf, INBUFFER_LENGTH);
            alder_log3("encoder(%d): fresh new input buffer");
            token = 4;
        } else {
            // No code.
            alder_log3("encoder(%d): use the previous input buffer");
        }
        if (lenbuf == 0) {
            break;
        }
        
        /* 3. While either input is not empty or output is not full: */
        while (token < 5) {
            /* 4. Form a Kmer and its reverse complementary. */
            if (token == 4) {
                token = 0;
                alder_log5("A starting Kmer is being created.");
                for (int i = 0; i < a->k - 1; i++) {
                    token = read_token(&curbuf, inbuf_body,
                                       lenbuf, type_infile);
                    int b1 = token;
                    int b2;
                    if (b1 == 0) { b2 = 2; } else if (b1 == 1) { b2 = 3; }
                    else if (b1 == 2) { b2 = 0; } else if (b1 == 3) { b2 = 1; }
                    else { break; }
                    alder_encode_number2_shiftLeftWith(s1,b1);
                    alder_encode_number2_shiftRightWith(s2,b2);
                }
                if (token >= 4) {
                    continue;
                }
            }
            token = read_token(&curbuf, inbuf_body, lenbuf, type_infile);
            if (token >= 4) continue;
            alder_log5("token: %d", token);
            int b1 = token;
            int b2;
            if (b1 == 0) { b2 = 2; } else if (b1 == 1) { b2 = 3; }
            else if (b1 == 2) { b2 = 0; } else if (b1 == 3) { b2 = 1; }
            else { assert(0); }
            alder_encode_number2_shiftLeftWith(s1,b1);
            alder_encode_number2_shiftRightWith(s2,b2);
            
            /* 5. Select partition, iteration, and kmer of the two. */
            alder_encode_number2_t *ss = NULL;
            uint64_t hash_s1 = alder_encode_number2_hash(s1);
            uint64_t hash_s2 = alder_encode_number2_hash(s2);
            uint64_t hash_ss = ALDER_MIN(hash_s1, hash_s2);
            ss = (hash_s1 < hash_s2) ? s1 : s2;
            for (size_t i = 0; i < kb; i++) {
                m->n[i] = ss->n[i].key64;
            }
            
            /* 6. Add the kmer to the table, and delete one if needed. */
            uint64_t added_x = c_ht->size;
            uint64_t removed_x = c_ht->size;
            s = alder_hashtable_mcas2_increment_spacesaving(c_ht, m->n,
                                                            hash_ss,
                                                            res_key, res_key2,
                                                            &added_x,
                                                            &removed_x,
                                                            a->isMultithreaded);
#if !defined(NDEBUG)
            alder_hashtable_mcas2_debug_print(c_ht,
                                              m->n,
                                              hash_ss,
                                              added_x,
                                              removed_x,
                                              bdata(a->boutdir),
                                              bdata(a->boutfile));
#endif
            if (s != ALDER_STATUS_SUCCESS) {
                alder_loge(ALDER_ERR, "Fatal - table is full!");
                assert(0);
                abort();
            }
        }
        if (token == 5) {
            c_inbuffer = a->n_counter;
        } else {
            c_inbuffer = counter_id;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    // Writer
    ///////////////////////////////////////////////////////////////////////
    if (counter_id == 0) {
        s = write_tabfile(a);
        if (s != ALDER_STATUS_SUCCESS) {
            alder_loge(ALDER_ERR_FILE, "failed to write to the table");
        }
    }
    
cleanup:
    XFREE(res_key);
    XFREE(res_key2);
    alder_encode_number_destroy(m);
    alder_encode_number2_destroy(s1);
    alder_encode_number2_destroy(s2);
    XFREE(len_outbuf2);
    alder_log2("counter(%d)[%llu]: END", counter_id);
    pthread_exit(NULL);
}

