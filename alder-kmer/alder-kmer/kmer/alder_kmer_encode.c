/**
 * This file, alder_kmer_encode.c, is part of alder-kmer.
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

/* Use the following in compilation in heavy debug
 * -DDETAILEDDEBUG
 */

/* This is a model of threads where a single controller prepares input data
 * for worker threads to consume. Processed data by workers are sent back to
 * the controller thread. The controller thread does something about it.
 * The problem that I wish to solve is to encode sequence data files to kmers.
 * The input file can be huge, which can be benefited by multi-threaded
 * technique. A single ioreadwriter thread reads from a single large file and
 * writes to multiple files. The ioreadwriter would read a chunk of data
 * from a large file. It distributes chunks to multiple encoder threads,
 * which do their jobs and notify the readwriter of the completion of the
 * encoding. The readwriter retrieves the encoded data, and writes it in
 * output files.
 *
 * THREAD MODEL SCENARIO
 * ---------------------
 * The readwriter waits for a call from an encoder. I'd use a mutex and a 
 * conditional variable. The readwrite waits by acquiring the mutex, and 
 * waiting conditionally. Then, an encoder would signal to the readwriter to
 * do something for it. The encoder would acquire the mutex when the readwriter
 * was waiting for a signal so that no other encoder would mess up. Before
 * signaling to the readwriter, the encoder marks the encoder ID using the
 * global variable, `selected_encoder'. Then, the woke-up readwriter would 
 * be informed of which encoder needs their input or output buffer updated. 
 * An encoder signals the readwriter whenever it needs its input buffer or
 * output buffer to be updated; e.g., an encoder used up all of the input, and
 * needs more input, or an encoder filled up the output buffer, and needs the
 * output buffer to be emptied.
 *
 * NUMBER OF THREADS
 * -----------------
 * I'd expect that the readwriter does not run all the time while encoders 
 * would consume of the CPU time. The user-specified or automatically 
 * determined number of CPU cores would be equal to the number of encoders.
 * Let's say this number is called N. The main function `alder_kmer_encode' 
 * would create N+1 threads. One is for the readwriter, and the rest of them
 * are for the encoders. So, the number that users specify for threads is 
 * equal to the number of encoders. When the readwriter update input and output
 * buffers for an encoder, the encoder would stand-by. So, this makes sense
 * in terms of the number user-specified of threads.
 *
 * READWRITER
 * ----------
 * The readwrtier does two jobs: 1. keep reading a chunk of data from all of
 * the input files, 2. write some output from encoders. It keeps working while
 * any encoder is running. Only after all of the encoder end, can the
 * readwriter finish its job.
 *
 * ENCODER
 * -------
 * The encoder asks the readwriter for an input buffer. The encoder gets the
 * input buffer to convert it to kmers. Kmers are packed in bytes. Some of 
 * kmers might not be packed in bytes for writing them in a file. Kmers are 
 * packed in K bytes where K is the kmer size. Those that are not packed are
 * are less than K bytes. They are not written in a file, but appended to 
 * the output in a next round.
 *
 * K=1,2,3
 * -------
 * Note that A, AA, AAA are impossible with the hash function that I'd use.
 * This allows me to use these A letters as the end of kmer buffer. Kmers of
 * size being greater than 3 can be extracted from a buffer. Those of size 
 * being 1,2,3 are not easy to tell. For example, consider a byte that contains
 * AAAA. Does this contain 1 A or 2 As? Or 3 As or 4As? I'd not be able to know
 * without size variable, which I do not have. But, the hash function does not
 * allow A to be a valid key because A's hash has a greater hash value than T's
 * hash. So, I'd see always T not A. A is naturally used as an end mark in kmer
 * buffers. This is similar in K=2 and K=3.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include "bzlib.h"
#include "alder_logger.h"
#include "alder_file.h"
#include "alder_cmacro.h"
#include "bstrmore.h"
#include "alder_fasta.h"
#include "alder_fastq.h"
#include "alder_fileseq_type.h"
#include "alder_fileseq_chunk.h"
#include "bstrmore.h"
#include "alder_encode.h"
#include "alder_fileseq_streamer.h"
#include "alder_progress.h"
#include "alder_dna.h"
#include "alder_hashtable_mcas.h"
#include "alder_kmer_encode.h"

typedef struct alder_kmer_encoder_struct alder_kmer_encoder_t;
typedef struct alder_kmer_readwriter_struct alder_kmer_readwriter_t;

/* Readwriter thread accesses this type.
 */
struct alder_kmer_readwriter_struct {
    int k;                     /* k - kmer size                              */
    uint64_t n_ni;             /* n_ni - number of iteration                 */
    uint64_t n_np;             /* number of iteration                        */
    int i_infile;              /* index of current infile                    */
    int type_infile;           /* type of input file                         */
    size_t size_inbuf;         /* size_inbuf - size of the inbuf             */
    size_t size_inbuf2;        /* size_inbuf2 - size of the inbuf2           */
    int begin_inbuf;           /* begin_inbuf - beginning point in the input
                                buffer                                       */
    size_t end_inbuf;          /* end_inbuf - ending position of input buffer
                                e.g., 0 empty, size_inbuf full               */
    size_t end_inbuf2;
    size_t *end_kmerbit;       /* n_np: end position of kmerbuf in bit       */
    size_t size_outbuf;        /* size_outbuf - size of the output buffer    */
    int n_encoder;             /* n_encoder - number of encoder threads      */
    
    size_t totalFilesize;      /* total file size                            */
    int progress_flag;
    int progressToError_flag;
    struct bstrList *infile;   /* input files                                */
    bstring dout;              /* output directory                           */
    size_t *end_outbuf;        /* n_np: end_outbuf - ending position of output
                                buffer e.g., 0 empty, size_outbuf full       */
    FILE *fpin;                /* fpin - input file pointer                  */
    void *fx;                  /* fx - input file pointer                    */
    char *inbuf;               /* size_inbuf: inbuf - input buffer           */
    char *inbuf2;              /* size_inbuf: inbuf2 - input buffer          */
    char **outbuf;             /* n_np x size_outbuf: outbuf - output buff   */
    char **kmerbuf;            /* n_np x (kmersize x n_encoder: kmer buff    */
    FILE **fpout;              /* n_np: fpout - output file pointers         */
    alder_kmer_encoder_t **ec; /* n_encoder: encoders         */
    
    /* stat */
    uint64_t n_byte;           /* number of bytes sent                       */
    uint64_t n_kmer;           /* number of Kmers written to an out file     */
};

/* Encoder thread accesses this type.
 */
struct alder_kmer_encoder_struct {
    int i;                     /* encoder ID                                 */
    int k;                     /* kmer size                                  */
    int status;                /* status of encoder's buffers                */
    int flag;                  /* status for readwriter                      */
    uint64_t i_ni;             /* iteration index                            */
    uint64_t n_ni;             /* number of iteration                        */
    uint64_t n_np;             /* number of out buffers or partitions        */
    size_t size_inbuf;         /* size of the input buffer                   */
    size_t size_outbuf;        /* size of the output buffer                  */
    int type_infile;           /* type of input file                         */
    
    uint64_t nb;               /* for saving inbuf's header                  */
    size_t  i_inbuf;           /* current position in fasta                  */
    size_t end_inbuf;          /* ending position of input buffer            */
    size_t *end_outbuf;        /* n_np: ending position of output buffer     */
    size_t *end_kmerbuf;       /* n_np: ending position of kmer buffer       */
    size_t *end_kmerbit;       /* n_np: ending of kmer buffer in bit         */
    char *inbuf;               /* size_inbuf: in buffer (size_inbuf)         */
    char **outbuf;             /* n_np x size_outbuf: out buffers            */
    char **kmerbuf;            /* n_np x k: out buffers                      */
    int *nk;                   /* n_np: remaining bits for packing           */
    alder_encode_number_t **n; /* n_np x encode_number: numbers for package  */
    
    /* stat */
    uint64_t n_kmer;           /* number of Kmers encoded by this encoder    */
    uint64_t n_letter;         /* number of DNA letters processed            */
    uint64_t n_byte;           /* number of bytes       processed            */
};


#pragma mark static variable

static pthread_mutex_t ec2rw_mutex;
static pthread_cond_t ec2rw_cv;
static int selected_encoder;

#pragma mark static function

static void *encoder(void *t);
static void *readwriter(void *t);
//static size_t alder_fasta_kmer_encode_inbuf_size(int M, int nt, int k);
//static size_t alder_fasta_kmer_encode_outbuf_size(size_t x, uint64_t np, int k);

static bool
alder_kmer_readwriter_copyEncoderFinalOutput (alder_kmer_readwriter_t *o,
                                              alder_kmer_encoder_t *e);

static bool
alder_kmer_readwriter_flush(alder_kmer_readwriter_t *o);


static void
alder_kmer_encode_token_reset(alder_kmer_encoder_t *e);

static int
alder_kmer_encode_token(alder_kmer_encoder_t *e);

static int
readwriter_open_infile(alder_kmer_readwriter_t *o);

static void
alder_kmer_encoder_init(alder_kmer_encoder_t *o,
                        int id,
                        uint64_t i_iteration,
                        uint64_t n_iteration,
                        uint64_t n_np,
                        int kmer_size,
                        size_t size_inbuf,
                        size_t size_outbuf);

static alder_kmer_encoder_t*
alder_kmer_encoder_create(int id,
                          uint64_t i_iteration,
                          uint64_t n_iteration,
                          uint64_t n_np,
                          int kmer_size,
                          int size_inbuf,
                          int size_outbuf);

static void
alder_kmer_encoder_destroy_with_error(alder_kmer_encoder_t *o, int s);

static void
alder_kmer_encoder_destroy(alder_kmer_encoder_t *o);


static void
alder_kmer_readwriter_init(alder_kmer_readwriter_t *o,
                           int n_encoder,
                           int i_iteration,
                           int kmer_size,
                           long disk_space,
                           long memory_available,
                           uint64_t n_iteration,
                           uint64_t n_partition,
                           size_t totalfilesize,
                           size_t n_byte,
                           int progress_flag,
                           int progressToError_flag,
                           struct bstrList *infile,
                           size_t size_inbuf,
                           size_t size_inbuf2,
                           size_t size_outbuf);

static alder_kmer_readwriter_t *
alder_kmer_readwriter_create(int n_encoder,
                             int i_iteration,
                             int kmer_size,
                             long disk_space,
                             long memory_available,
                             long sizeInbuffer,
                             long sizeOutbuffer,
                             uint64_t n_iteration,
                             uint64_t n_partition,
                             size_t totalfilesize,
                             size_t n_byte,
                             int progress_flag,
                             int progressToError_flag,
                             struct bstrList *infile,
                             const char *outdir,
                             const char *outfile);

static void
alder_kmer_readwriter_destroy_with_error
(alder_kmer_readwriter_t *o, int s);

/* This function destroys the argument object for a readwrite thread.
 */
static void
alder_kmer_readwriter_destroy
(alder_kmer_readwriter_t *o);

static bool
alder_kmer_readwriter_copyEncoderOutput
(alder_kmer_readwriter_t *o,
 alder_kmer_encoder_t *e);

static bool
alder_kmer_readwriter_write(alder_kmer_readwriter_t *o);

static void encoder_append_kmerbuf_n_kmer(alder_kmer_encoder_t *b,
                                          uint64_t *n_kmer);
static void encoder_append_kmerbuf(alder_kmer_encoder_t *b);
static void encoder_save_kmerbuf(alder_kmer_encoder_t *b);

#pragma mark main

/* This function converts input sequence files to files with kmers.
 * returns
 * SUCCESS or FAIL
 *
 * It encodes or converts sequences in `infile' to K-mers (size: `kmer_size')
 * in `n_partition' out files in `dout' directory. I'd use the disk space for
 * saving those output files. This is `i_iteration' out of `n_iteration'.
 * The `memory_available' would determine the size of hash tables that could
 * be loaded at once.
 *
 * The total number of threads is one plus the number of encoders.
 * The number of threads is equal to the number of worker threads or encoders.
 * The file read/writer reads sequence files, and prepares buffers that
 * encoders can process. Encoders encode sequences to K-mers, and send buffers
 * back to the file read/writer, which saves the buffers in files.
 */
int
alder_kmer_encode(int n_encoder,
                  int i_iteration,
                  int kmer_size,
                  long disk_space,
                  long memory_available,
                  long sizeInbuffer,
                  long sizeOutbuffer,
                  uint64_t n_iteration,
                  uint64_t n_partition,
                  size_t totalfilesize,
                  size_t *n_byte,
                  int progress_flag,
                  int progressToError_flag,
                  struct bstrList *infile,
                  const char *outdir,
                  const char *outfile)
{
    assert(n_encoder >= 1);
    alder_log5("preparing encoding Kmers...");
    selected_encoder = 0;
    int s = ALDER_STATUS_SUCCESS;
    int n_thread = n_encoder + 1;
    /* Create variables for the threads. */
    alder_kmer_readwriter_t *rw =
    alder_kmer_readwriter_create(n_encoder,
                                 i_iteration,
                                 kmer_size,
                                 disk_space,
                                 memory_available,
                                 sizeInbuffer,
                                 sizeOutbuffer,
                                 n_iteration,
                                 n_partition,
                                 totalfilesize,
                                 *n_byte,
                                 progress_flag,
                                 progressToError_flag,
                                 infile,
                                 outdir,
                                 outfile);
    alder_log5("creating % threads: one for the readwriter", n_thread);
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    memset(threads, 0, sizeof(*threads)*n_thread);
    if (rw == NULL || threads == NULL) {
        alder_loge(ALDER_ERR_MEMORY,
                   "failed to create fileseq_kmer_thread_readwriter");
        XFREE(threads);
        alder_kmer_readwriter_destroy(rw);
        return ALDER_STATUS_ERROR;
    }
    /* Initialize mutex and condition variable objects */
    pthread_attr_t attr;
    s += pthread_mutex_init(&ec2rw_mutex, NULL);
    s += pthread_cond_init (&ec2rw_cv, NULL);
    s += pthread_attr_init(&attr);
    s += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    s += pthread_create(&threads[0], &attr, readwriter, (void *)rw);
    for (int i = 0; i < n_encoder; i++) {
        s += pthread_create(&threads[i+1], &attr, encoder, (void *)rw->ec[i]);
    }
    if (s != 0) {
        alder_loge(ALDER_ERR_THREAD, "cannot create threads - %s",
                   strerror(errno));
        goto cleanup;
    }
    /* Wait for all threads to complete */
    alder_log5("Waiting for all threads to complete...");
    for (int i = 0; i < n_thread; i++) {
        s += pthread_join(threads[i], NULL);
        if (s != 0) {
            alder_loge(ALDER_ERR_THREAD, "cannot join threads - %s",
                       strerror(errno));
            goto cleanup;
        }
    }
    
cleanup:
    /* Cleanup! */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&ec2rw_mutex);
    pthread_cond_destroy(&ec2rw_cv);
    XFREE(threads);
    *n_byte = rw->n_byte;
    alder_kmer_readwriter_destroy(rw);
    alder_log5("Encoding Kmer has been finished with %d threads.", n_thread);
    return ALDER_STATUS_SUCCESS;
}

#pragma mark thread

/* This function runs as a separate thread.
 * It keeps reading from a sequence file to send sequences to worker threads,
 * called encoder. Let me consider possible cases between readwriter and
 * encoder:
 *
 * READWRITER's SCENARIO
 * . readwriter is waiting, and encoder needs inbuf.
 * . readwriter tries to copy outbuf whether there is anything or not.
 * . readwriter sends some inbuf only if encoder's inbuf is empty.
 * . readwriter let encoder resume without sending inbuf if encoder has inbuf
 * . readwriter let encoder work more on non-empty inbuf, if encoder's inbuf is
 *   not empty (encoder->nb > 0).
 * . readwriter does not let encoder exit unless encoder informs it.
 * . readwriter sends inbuf when encoder's inbuf is empty, and readwriter's
 *   inbuf is not empty.
 * . readwriter cannot send inbuf if its inbuf is empty even when encoder's
 *   inbuf is empty. readwriter just let encoder work on any remaining data.
 * . readwriter would let encoder exit when no inbuf in either readwriter's or
 *   encoder's, and when no outbuf with final flag from encoder.
 * . Whenever readwriter wakes up, it reads the outbuf from the encoder that
 *   called.
 *
 * ENCODER's SCENARIO
 * . encoder decides whether to mark the final stage or not.
 * . encoder would be in the final stage only when no input and some output.
 * . encoder has a flag: ALDER_KMER_ENCODE_FINAL_OUTBUF
 * . encoder can change this flag, not readwriter.
 * . encoder calls readwriter when it has no inbuf, or it has some outbuf.
 * . encoder's outbuf can be full, which means that there is at least one full
 *   outbuf among as many outbuf as partition.
 */
static void *readwriter(void *t)
{
    alder_log("readwriter(): START");
    int s = ALDER_STATUS_SUCCESS;
    alder_kmer_readwriter_t *a = (alder_kmer_readwriter_t*)t;
    
    assert(t != NULL);
    assert(a != NULL);
    assert(a->fpin == NULL);
    assert(a->end_inbuf == 0);
    
    /* 1. Repeat while there is a encoder that processes a chunk of data. */
    int remaining_encoder = a->n_encoder;
    while (remaining_encoder > 0) {
        
        /* 2. Reads a chunk of data from the input file. */
        if (a->end_inbuf == 0 && s == ALDER_STATUS_SUCCESS) {
            s = alder_fileseq_chunk(&a->end_inbuf, a->inbuf, a->size_inbuf,
                                    &a->end_inbuf2, a->inbuf2, a->size_inbuf2,
                                    a->k, a->fpin, a->fx, a->type_infile);
            if (s != ALDER_STATUS_SUCCESS) {
                s = readwriter_open_infile(a);
                if (s == ALDER_STATUS_SUCCESS) {
                    s = alder_fileseq_chunk(&a->end_inbuf, a->inbuf, a->size_inbuf,
                                            &a->end_inbuf2, a->inbuf2, a->size_inbuf2,
                                            a->k, a->fpin, a->fx, a->type_infile);
                } else {
                    if (a->infile->qty == a->i_infile) {
                        // No input file available.
                    } else {
                        alder_loge(ALDER_ERR_THREAD,
                                   "readwrite: cannot open infile");
                    }
                }
            }
        }
        
        /* 3. Wait for a call from an Encoder. */
        ///////////////////////////////////////////////////////////////////////
        // Critical Section: BEGIN
        ///////////////////////////////////////////////////////////////////////
        pthread_mutex_lock(&ec2rw_mutex);
        alder_log4("rwriter( ): waiting for a call from an encoder...");
        selected_encoder = -1;
        pthread_cond_wait(&ec2rw_cv, &ec2rw_mutex);
        alder_log3("rwriter( ): wake-up by encoder(%d)", selected_encoder);
        
        /* 4. Locate the Encoder in idle: either no input or some output. */
        alder_kmer_encoder_t *ec = a->ec[selected_encoder];
        assert(selected_encoder < a->n_encoder);
        assert(ec != NULL);
        assert(ec->i == selected_encoder);
        
        /* 5. Copy outbuffer of the encoder if needed */
        bool isEmptyOutbuf;
        if (ec->flag == ALDER_KMER_ENCODE_MORE_OUTBUF) {
            isEmptyOutbuf = alder_kmer_readwriter_copyEncoderOutput(a, ec);
        } else {
            alder_log3("rwriter( ): final output");
            isEmptyOutbuf = alder_kmer_readwriter_copyEncoderFinalOutput(a, ec);
        }
        
#if !defined(NDEBUG)
        if (isEmptyOutbuf == false) {
            alder_log3("rwriter( ): Some out buffer copied");
        } else {
            alder_log3("rwriter( ): No out buffer copied");
        }
#endif
        
        /* 8. Write outbuff to output files. */
        if (ec->flag == ALDER_KMER_ENCODE_MORE_OUTBUF) {
            alder_kmer_readwriter_write(a);
        } else {
            alder_log2("rwriter( ): no writing, will flush them later.");
        }
        
        /* Test whether encoder's input buffer is empty (==0) or not. */
        if (ec->nb == 0) {
            /* Encoder's input buffer is empty. */
            /* 6. Send part of chunk to encoder if needed. */
            if (a->end_inbuf > 0) {
                assert(a->end_inbuf <= a->size_inbuf);
                memcpy(ec->inbuf, a->inbuf, a->end_inbuf);
                alder_log2("rwriter( ): sending data to "
                           "encoder(%d)... %zu MB or %zu B",
                           selected_encoder,
                           (a->end_inbuf >> 20), a->end_inbuf);
                a->n_byte += a->end_inbuf;
                if (a->progress_flag) {
                    alder_progress_step(a->n_byte, a->totalFilesize,
                                        a->progressToError_flag);
                }
                
                ec->type_infile = a->type_infile;
                ec->end_inbuf = a->end_inbuf;
                a->end_inbuf = 0;
#if defined(DETAILEDDEBUG)
                logc_logArray(MAIN_LOGGER, LOG_FINEST, "data:",
                              (uint8_t*)ec->inbuf, ec->end_inbuf);
#endif
                /* 7. Let Encoder work on data. */
                ec->status = ALDER_KMER_ENCODE_INBUFFER_FULL;
                alder_log3("rwriter( ): full wakeup encoder(%d)",
                           selected_encoder);
            } else if (isEmptyOutbuf == true &&
                       ec->flag == ALDER_KMER_ENCODE_FINAL_OUTBUF) {
                assert(a->infile->qty == a->i_infile);
                remaining_encoder--;
                ec->end_inbuf = 0;
                ec->status = ALDER_KMER_ENCODE_CAN_EXIT;
                alder_log2("rwriter( ): final wakeup encoder(%d)",
                           selected_encoder);
            } else {
                assert(a->infile->qty == a->i_infile);
                ec->end_inbuf = 0;
                ec->status = ALDER_KMER_ENCODE_CANNOT_EXIT;
                alder_log2("rwriter( ): empty wakeup encoder(%d)",
                           selected_encoder);
            }
        } else {
            /* Encoder's output was full, but inbuf is not empty yet.*/
            /* Encoder's input buffer is not copied, and just let it work. */
            alder_log2("rwriter( ): remain wakeup(%d) - not empty inbuf",
                       selected_encoder);
            ec->status = ALDER_KMER_ENCODE_INBUFFER_FULL;
        }
        alder_log2("rwriter( ): [rw --------> ec(%d)] let encoder(%d) work ",
                   selected_encoder, selected_encoder);
        pthread_mutex_unlock(&ec2rw_mutex);
        ///////////////////////////////////////////////////////////////////////
        // Critical Section: END
        ///////////////////////////////////////////////////////////////////////
        
    }
    
    // Flush the remaining buffer that would be incomplete.
    alder_kmer_readwriter_flush(a);
    
    alder_log3("rwriter( ): waiting for all workers to exit...");
    remaining_encoder = a->n_encoder;
    uint64_t n_kmer = 0;
    uint64_t n_letter = 0;
    uint64_t n_byte = 0;
    while (remaining_encoder > 0) {
        remaining_encoder = a->n_encoder;
        for (int i = 0; i < a->n_encoder; i++) {
            if (a->ec[i]->flag == ALDER_KMER_ENCODE_EXIT) {
                remaining_encoder--;
                n_kmer += a->ec[i]->n_kmer;
                n_letter += a->ec[i]->n_letter;
                n_byte += a->ec[i]->n_byte;
                a->ec[i]->n_kmer = 0;
                a->ec[i]->n_letter = 0;
                a->ec[i]->n_byte = 0;
            }
        }
    }
    
    alder_log("readwriter: dna   - %llu (from encoder)", n_letter);
    alder_log("readwriter: bytes - %llu (from encoder)", n_byte);
    alder_log("readwriter: bytes - %llu", a->n_byte);
    alder_log("readwriter: Kmers - %llu (from encoder)", n_kmer);
    alder_log("readwriter: Kmers - %llu", a->n_kmer);
    alder_log("readwriter(): END");
cleanup:
    // FIXME: make sure all of the encoders are gone.
    pthread_exit(NULL);
}


/* This function encodes and saves kmers in multiple output buffers.
 *
 * 1. The encoder keeps checking whether its status changed.
 * 2. If there are something in the input buffer, it processes the input.
 *    Two cases are possible: 1. The input is a fresh new (b->nb == 0) or given
 *    by the readwriter just now, and 2. The input is not new, and I'd have to
 *    resume where I stopped before because of full out buffer (b->nb != 0).
 * 3. Keep taking a token at a time to create a Kmer. Token 0,1,2,3 are for
 *    DNA letters, and token 4 is the end of the current sequence. Token 5
 *    means that input buffer is empty.
 * 4. A kmer is created.
 * 5. Choose which kmer of a kmer and its reverse complementary would be used
 *    to decide which partition I use for saving for this itertation.
 *    Selected i_ni, i_np are used whether I save the Kmer or not. One of two
 *    kmers is slected as ss for saving.
 * 6. The out buffer for the chosen partition is selected. The current end
 *    position for pushing the current kmer is in end_kmerbuf, n, nk. I'd need
 *    to know which byte and which bit I need to start to save the kmer. 
 *    end_kmerbuf points to the position in the outbuf, 
 *    n is a encode number just like kmer, but for saving bits that were not
 *    saved before because they did not fit to a byte.
 *    nk is the number of DNA letters that were not saved before.
 *    Once I did the packed writing, I update nk and end_kmerbuf. n is 
 *    automatically updated during the packed writing.
 *    end_outbuf is always less than or equal to end_kmerbuf. end_outbuf marks
 *    the position upto which I can copy it to the readwriter for writing 
 *    the out buffer in a file.
 *    I'd readjust the kmerbuf position so that it saves multiple of K. The
 *    end_outbuf is multiple of K.
 * 7. b->n_kmer : total number of kmers packed so far.
 *    n_kmer : number of kmers encoded in the outbuf.
 *    I'd also compute kmers that could be copied to the readwriter.
 * 8. Because only multiple of K kmers in out buffer would be sent to the
 *    readwriter, the remaining bytes in the out buffer are saved. These bytes
 *    would be copied to the front of the out buffer when I reenter this loop.
 * 9. I mentioned that I'd have two cases: 1. the input buffer is empty, 2.
 *    the out buffer is full. I'd mark nb to signify which case happened.
 *    In the readwriter, I'd use this to tell whether a new input buffer is
 *    needed or not.
 * 10. When no input is available from the readwriter, I need to send all of 
 *     the remaining out buffer even if kmers do not fit to the byte. 
 *     Instead of using bytes, I'd use bits. Let the readwriter know the bits.
 *
 * Status: 1. Input buffer is not empty -> could be inbuf empty or outbuf full.
 *         2. Input buffer is empty, but outbuf is not empty.
 *         3. Input and output buffers are both empty.
 */
static void *encoder(void *t)
{
    alder_encode_number_t * s1 = NULL;
    alder_encode_number_t * s2 = NULL;
    alder_kmer_encoder_t *b = (alder_kmer_encoder_t*)t;
    assert(b != NULL);
    assert(t != NULL);
    assert(b->nb == 0);        // Just for getting data in the first time
    assert(b->end_inbuf == 1); // Just for entering while loop.
    assert(b->end_outbuf != NULL);
    assert(b->status == ALDER_KMER_ENCODE_INBUFFER_EMPTY);
    
    int i_encoder = b->i;
    int kmer_size = b->k;
    alder_log("encoder(%d): START", i_encoder);
    s1 = alder_encode_number_create_for_kmer(kmer_size);
    s2 = alder_encode_number_create_for_kmer(kmer_size);
    uint64_t *n_kmer = malloc(sizeof(*n_kmer) * b->n_np);
    memset(n_kmer,0,sizeof(*n_kmer) * b->n_np);
    uint64_t n_ni = b->n_ni;
    uint64_t n_np = b->n_np;
    
    while (b->status != ALDER_KMER_ENCODE_CAN_EXIT) {
        /* 1. Wait till Readwriter prepares the inbuffer for this Encoder. */
        assert(b->status == ALDER_KMER_ENCODE_INBUFFER_EMPTY);
        if (b->status == ALDER_KMER_ENCODE_INBUFFER_EMPTY) {
            alder_log4("encoder(%d): start calling readwriter.", i_encoder);
        }
        while (b->status == ALDER_KMER_ENCODE_INBUFFER_EMPTY) {
            pthread_mutex_lock(&ec2rw_mutex);
            if (selected_encoder == -1) {
                alder_log2("encoder(%d): [rw <-------- ec(%d)] called readwriter.",
                           i_encoder, i_encoder);
                selected_encoder = i_encoder;
                pthread_cond_signal(&ec2rw_cv);
            } else {
                alder_log5("encoder(%d): zzz", i_encoder);
            }
            pthread_mutex_unlock(&ec2rw_mutex);
            sleep(1);
        }
        alder_log3("encoder(%d): wake-up with %d", i_encoder, b->end_inbuf);
#if !defined(NDEBUG)
        if (b->status == ALDER_KMER_ENCODE_INBUFFER_FULL) {
            assert(b->end_inbuf > 0);
        }
#endif        
        encoder_append_kmerbuf(b);
        
        /* 2. Work on encoding of sequences. */
        if (b->end_inbuf > 0) {
            encoder_append_kmerbuf_n_kmer(b,n_kmer);
            
            bool isFullOutbuf = false;
            int token;
            if (b->nb == 0) {
                alder_log3("encoder(%d): fresh new input buffer", i_encoder);
                alder_kmer_encode_token_reset(b);
                token = 4; // Start with a new Kmer.
            } else {
                alder_log3("encoder(%d): resuming where it stopped", i_encoder);
                token = 0; // Continue from where I stopped.
            }
            alder_log2("encoder(%d): working with %d B at %d",
                       i_encoder, b->end_inbuf, b->i_inbuf);
            alder_log2("encoder(%d): .................................",
                       i_encoder);
            
            /* 3. While either input is not empty or output is not full: */
            while (token < 5 && isFullOutbuf == false) {
                if (token == 4) {
                    // Create a number.
                    token = 0;
                    alder_log5("A starting Kmer is being created.");
                    for (int i = 0; i < b->k - 1 && token < 4; i++) {
                        token = alder_kmer_encode_token(b);
                        int b1 = token;
                        int b2 = (b1 + 2) % 4;
                        alder_encode_number_kmer_shift_left_with(s1, b1);
                        alder_encode_number_kmer_shift_right_with(s2, b2);
                    }
                    if (token >= 4) {
                        continue;
                    }
                }
                token = alder_kmer_encode_token(b);
                if (token >= 4) {
                    continue;
                }
                alder_log5("token: %d", token);
                
                /* 4. Form a Kmer and its reverse complementary. */
                int b1 = token;
                int b2 = (b1 + 2) % 4;
                alder_encode_number_kmer_shift_left_with(s1, b1);
                alder_encode_number_kmer_shift_right_with(s2, b2);
#if defined(DETAILEDDEBUG)
                alder_log4("Kmer");
                alder_encode_number_log(s1);
                alder_encode_number_log(s2);
#endif
                /* 5. Select partition, iteration, and kmer of the two. */
                alder_encode_number_t *ss = NULL;
                uint64_t i_ni = 0;
                uint64_t i_np = 0;
                alder_hashtable_mcas_select(&ss, &i_ni, &i_np, s1, s2,
                                            n_ni, n_np);
//#if defined(FORWARDSTRAND)
//                ss = s1;
//#endif
                /* 6. Save a chosen one in the output buffers. */
                if (i_ni == b->i_ni) {
                    uint8_t *outbuf = (uint8_t*)b->outbuf[i_np];
                    size_t end_outbuf = b->end_kmerbuf[i_np];
                    alder_encode_number_t *n2 = b->n[i_np];
                    int nk = b->nk[i_np];
                    alder_encode_number_packwrite_buffer((uint8_t*)outbuf,
                                                         &end_outbuf,
                                                         ss, n2, &nk);
                    b->n_kmer++;
                    n_kmer[i_np]++;
                    
                    b->nk[i_np] = nk;
                    b->end_kmerbuf[i_np] = end_outbuf;
                    b->end_outbuf[i_np] = end_outbuf / b->k * b->k;
                    
                    assert(b->size_outbuf > end_outbuf);
                    if (b->size_outbuf - end_outbuf < ss->s * 8) {
                        // The outbuf is full.
                        // Stop here, and I'd restart after sending the output
                        // buffer back.
                        break;
                    }
                    
#if defined(DETAILEDDEBUG)
                    alder_log4("nk: %d", nk);
                    alder_encode_number_buffer_log(outbuf, end_outbuf, b->k);
                    alder_encode_number_log(ss);
                    alder_encode_number_log(n2);
#endif
                }
            }
            /* 7. Some report */
            alder_log2("encoder(%d): encoded total kmer = %zu",
                       b->i, b->n_kmer);
            for (int i = 0; i < n_np; i++) {
                assert((b->end_outbuf[i] * 4) % b->k == 0);
                alder_log2("encoder(%d): %llu B in kmerbuf[%d]",
                           i_encoder, b->end_kmerbuf[i], i);
                alder_log2("encoder(%d): %llu kmers in kmerbuf[%d](encoded+appended)",
                           i_encoder, n_kmer[i], i);
                alder_log2("encoder(%d): %zu kmers in outbuf[%d], "
                           "%zu kmers diff will be saved.",
                           i_encoder,
                           b->end_outbuf[i] * 4 / b->k, i,
                           n_kmer[i] - b->end_outbuf[i] * 4 / b->k);
            }
            /* 8. Save unfit kmers. */
            encoder_save_kmerbuf(b);
            
            /* 9. Mark the status if inbuf. */
            if (token == 5) {
                // The inbuf is emtpy.
                b->nb = 0;
                alder_log2("encoder(%d): inbuf is empty.", b->i);
            } else {
                // The outbuf is full.
                b->nb = 1;
                alder_log2("encoder(%d): outbuf is full.", b->i);
            }
            b->status = ALDER_KMER_ENCODE_INBUFFER_EMPTY;
        } else if (b->flag == ALDER_KMER_ENCODE_MORE_OUTBUF) {
            /* 10. Flush any remaining buffer. */
            alder_log2("encoder(%d): flushing out outbuf...", i_encoder);
            alder_log2("encoder(%d): .................................",
                       i_encoder);
            for (int i_np = 0; i_np < b->n_np; i_np++) {
                // 1. kmerbuf
                // 2. and some in n2.
                uint8_t *outbuf = (uint8_t*)b->outbuf[i_np];
                size_t end_outbuf = b->end_kmerbuf[i_np];
                alder_encode_number_t *n2 = b->n[i_np];
                int nk = b->nk[i_np];
                assert(b->end_outbuf[i_np] < b->k);
                b->end_kmerbit[i_np] = 8 * b->end_kmerbuf[i_np] + 2 * nk;
                assert(b->end_kmerbit[i_np] < 8 * b->k);
                alder_encode_number_packflush_buffer(outbuf,
                                                     &end_outbuf,
                                                     n2, &nk);
                b->nk[i_np] = nk;
                assert(nk == 0);
                b->end_kmerbuf[i_np] = 0;
                b->end_outbuf[i_np] = end_outbuf;
                alder_log2("encoder(%d): kmerbit[%d] = %zu, outbuf[%d] = %zu",
                           i_encoder, i_np, b->end_kmerbit[i_np],
                           i_np, b->end_outbuf[i_np]);
            }
            b->flag = ALDER_KMER_ENCODE_FINAL_OUTBUF;
        } else {
            /* 11. Exit the loop. */
            assert(b->flag == ALDER_KMER_ENCODE_FINAL_OUTBUF);
            if (b->status != ALDER_KMER_ENCODE_CAN_EXIT) {
                alder_log2("encoder(%d): ready to exit!", i_encoder);
            } else {
                alder_log2("encoder(%d): exiting...", i_encoder);
            }
            alder_log2("encoder(%d): .......................", i_encoder);
        }
        
        if (b->end_inbuf == 0 && b->status != ALDER_KMER_ENCODE_CAN_EXIT) {
            assert(b->status == ALDER_KMER_ENCODE_INBUFFER_EMPTY ||
                   b->status == ALDER_KMER_ENCODE_CANNOT_EXIT);
            b->status = ALDER_KMER_ENCODE_INBUFFER_EMPTY;
        }
    }
    
    alder_log("encoder(%d): total number of dna   - %llu", b->i, b->n_letter);
    alder_log("encoder(%d): total number of bytes - %llu", b->i, b->n_byte);
    alder_log("encoder(%d): total number of Kmers - %llu", b->i, b->n_kmer);
    b->flag = ALDER_KMER_ENCODE_EXIT;
    XFREE(n_kmer);
    alder_encode_number_destroy(s1);
    alder_encode_number_destroy(s2);
    
    alder_log("encoder(%d): END", i_encoder);
    pthread_exit(NULL);
}


#pragma mark thread argument

/* This function initializes encoder sruct.
 */
void
alder_kmer_encoder_init(alder_kmer_encoder_t *o,
                        int id,
                        uint64_t i_ni,
                        uint64_t n_ni,
                        uint64_t n_np,
                        int kmer_size,
                        size_t size_inbuf,
                        size_t size_outbuf)
{
    o->i = id;
    o->k = kmer_size;
    o->status = ALDER_KMER_ENCODE_INBUFFER_EMPTY;
    o->flag = ALDER_KMER_ENCODE_MORE_OUTBUF;
    o->i_ni = i_ni;
    o->n_ni = n_ni;
    o->n_np = n_np;
    o->size_inbuf = size_inbuf;
    o->size_outbuf = size_outbuf;
    // nb?      - set by encoder
    // i_nb?
    // i_kmer?
    o->i_inbuf = 0;
    o->nb = 0;        // set by encoder
    o->end_inbuf = 1; // set by encoder
    o->end_outbuf = NULL;
    o->inbuf = NULL;
    o->outbuf = NULL;
    o->nk = NULL;
    o->n = NULL;
    o->n_byte = 0;
    o->n_kmer = 0;
    o->n_letter = 0;
}

/* This function prepares a data structure for encoder thread.
 * returns
 * alder_kmer_encoder_t on success
 * nil if an error occurs.
 *
 * memory
 *
 * 1D arrays
 * . end_outbuf
 * . nk
 * . inbuf
 *
 * 2D arrays
 * . outbuf
 * . n
 */
alder_kmer_encoder_t*
alder_kmer_encoder_create(int id,
                          uint64_t i_iteration,
                          uint64_t n_iteration,
                          uint64_t n_np,
                          int kmer_size,
                          int size_inbuf,
                          int size_outbuf)
{
    alder_log5("Creating fileseq_kmer_thread_encoder...");
    /* initialization */
    alder_kmer_encoder_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY,
                   "failed to create fileseq_kmer_thread_encoder");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    alder_kmer_encoder_init(o,
                            id,
                            i_iteration,
                            n_iteration,
                            n_np,
                            kmer_size,
                            size_inbuf,
                            size_outbuf);
    /* memory allocation */
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*n_np);
    o->end_kmerbuf = malloc(sizeof(*o->end_kmerbuf)*n_np);
    o->end_kmerbit= malloc(sizeof(*o->end_kmerbit)*n_np);
    o->nk = malloc(sizeof(*o->nk)*n_np);
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    if (o->end_outbuf == NULL ||
        o->end_kmerbuf == NULL ||
        o->end_kmerbit == NULL ||
        o->nk == NULL ||
        o->inbuf == NULL) {
        alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->end_outbuf, 0, sizeof(*o->end_outbuf)*n_np);
    memset(o->end_kmerbuf, 0, sizeof(*o->end_kmerbuf)*n_np);
    memset(o->end_kmerbit, 0, sizeof(*o->end_kmerbit)*n_np);
    memset(o->nk, 0, sizeof(*o->nk)*n_np);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * size_inbuf);
    // 2D
    o->outbuf = malloc(sizeof(*o->outbuf)*n_np);
    if (o->outbuf == NULL) {
        alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->outbuf, 0, sizeof(*o->outbuf)*n_np);
    for (int i = 0; i < n_np; i++) {
        o->outbuf[i] = malloc(sizeof(*o->outbuf[i])*size_outbuf);
        if (o->outbuf[i] == NULL) {
            alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
        memset(o->outbuf[i],0,sizeof(*o->outbuf[i])*size_outbuf);
    }
    // 2D
    o->n = malloc(sizeof(*o->n)*n_np);
    if (o->n == NULL) {
        alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->n, 0, sizeof(*o->n)*n_np);
    for (int i = 0; i < n_np; i++) {
        o->n[i] = alder_encode_number_create_for_kmer(kmer_size);
        if (o->n[i] == NULL) {
            alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
    }
    // 2D
    o->kmerbuf = malloc(sizeof(*o->kmerbuf)*n_np);
    if (o->kmerbuf == NULL) {
        alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->kmerbuf, 0, sizeof(*o->kmerbuf)*n_np);
    for (int i = 0; i < n_np; i++) {
        o->kmerbuf[i] = malloc(sizeof(*o->kmerbuf[i])*kmer_size);
        if (o->kmerbuf[i] == NULL) {
            alder_kmer_encoder_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
        memset(o->kmerbuf[i],0,sizeof(*o->kmerbuf[i])*kmer_size);
    }
    return o;
}

void alder_kmer_encoder_destroy_with_error
(alder_kmer_encoder_t *o, int s)
{
    if (s != 0) {
        alder_loge(s,
                   "cannot create fileseq_kmer_thread_encoder");
    }
    alder_kmer_encoder_destroy(o);
}

/* This function frees the memory.
 *
 * memory
 *
 * 1D arrays
 * . end_outbuf
 * . nk
 * . inbuf
 *
 * 2D arrays
 * . outbuf
 * . n
 */
void
alder_kmer_encoder_destroy (alder_kmer_encoder_t *o)
{
    if (o != NULL) {
        if (o->n != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                alder_encode_number_destroy(o->n[i]);
            }
            XFREE(o->n);
        }
        if (o->outbuf != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                XFREE(o->outbuf[i]);
            }
            XFREE(o->outbuf);
        }
        if (o->kmerbuf != NULL) {
            for (int i = 0; i < o->n_np; i++) {
                XFREE(o->kmerbuf[i]);
            }
            XFREE(o->kmerbuf);
        }
        XFREE(o->inbuf);
        XFREE(o->nk);
        XFREE(o->end_kmerbit);
        XFREE(o->end_kmerbuf);
        XFREE(o->end_outbuf);
        XFREE(o);
    }
}

void
alder_kmer_readwriter_init(alder_kmer_readwriter_t *o,
                           int n_encoder,
                           int i_iteration,
                           int kmer_size,
                           long disk_space,
                           long memory_available,
                           uint64_t n_iteration,
                           uint64_t n_partition,
                           size_t totalfilesize,
                           size_t n_byte,
                           int progress_flag,
                           int progressToError_flag,
                           struct bstrList *infile,
                           size_t size_inbuf,
                           size_t size_inbuf2,
                           size_t size_outbuf)
{
    o->infile = NULL;
    o->dout = NULL;
    o->fpin = NULL;
    o->fpout = NULL;
    o->end_outbuf = NULL;
    o->inbuf = NULL;
    o->inbuf2 = NULL;
    o->outbuf = NULL;
    o->ec = NULL;
    o->fx = NULL;
    
    o->totalFilesize = totalfilesize;
    o->progress_flag = progress_flag;
    o->progressToError_flag = progressToError_flag;
    
    o->i_infile = 0;
    o->type_infile = 0;
    o->n_ni = n_iteration;
    o->n_np = n_partition;
    o->k = kmer_size;
    o->size_inbuf = size_inbuf;
    o->size_inbuf2 = size_inbuf2;
    o->begin_inbuf = 0;
    o->end_inbuf = 0;
    o->size_outbuf = size_outbuf;
    o->n_encoder = n_encoder;
    o->n_byte = n_byte;
    o->n_kmer = 0;
    
}
/* This function prepares a data structure for readwriter thread.
 * returns
 * a valid object on success
 * nil if an error occurs.
 *
 * create partition files.
 *
 * memory
 * 1D
 * . infile
 * . dout
 * . end_outbuf
 * . inbuf
 * . inbuf2
 * 2D
 * . outbuf
 * . fpin
 * . fpout
 */
alder_kmer_readwriter_t *
alder_kmer_readwriter_create(int n_encoder,
                             int i_iteration,
                             int kmer_size,
                             long disk_space,
                             long memory_available,
                             long sizeInbuffer,
                             long sizeOutbuffer,
                             uint64_t n_iteration,
                             uint64_t n_partition,
                             size_t totalfilesize,
                             size_t n_byte,
                             int progress_flag,
                             int progressToError_flag,
                             struct bstrList *infile,
                             const char *outdir,
                             const char *outfile)
{
    alder_log5("Creating fileseq_kmer_thread_readwriter...");
    int s = 0;
    int size_inbuf;
    int size_outbuf;
    
    
    if (sizeInbuffer == 0) {
        size_inbuf = 1 << 23;
//        (int) alder_fasta_kmer_encode_inbuf_size((int)memory_available,
//                                                 n_encoder, kmer_size);
    } else {
        size_inbuf = (int)sizeInbuffer;
    }
    int size_inbuf2 = ALDER_KMER_SECONDARY_BUFFER_SIZE;
    
    if (sizeOutbuffer == 0) {
        size_outbuf = size_inbuf * (kmer_size / 4);
        if (size_outbuf > size_inbuf * 3) {
            size_outbuf = size_inbuf * 3;
        }
//        (int) alder_fasta_kmer_encode_outbuf_size((size_t) size_inbuf,
//                                                  n_partition, kmer_size);
    } else {
        size_outbuf = (int)sizeOutbuffer;
    }
    
    int width = ALDER_LOG_TEXTWIDTH;
    alder_log("%*s %d bytes", width, "inbuf size:", size_inbuf);
    alder_log("%*s %d bytes", width, "outbuf size:", size_outbuf);
    
    alder_kmer_readwriter_t *o = malloc(sizeof(*o));
    if (o == NULL) {
        alder_loge(ALDER_ERR_MEMORY, "cannot create fileseq_kmer_thread_readwriter");
        return NULL;
    }
    memset(o, 0, sizeof(*o));
    alder_kmer_readwriter_init(o,
                               n_encoder,
                               i_iteration,
                               kmer_size,
                               disk_space,
                               memory_available,
                               n_iteration,
                               n_partition,
                               totalfilesize,
                               n_byte,
                               progress_flag,
                               progressToError_flag,
                               infile,
                               size_inbuf,
                               size_inbuf2,
                               size_outbuf);
    
    /* Memory */
    o->infile = bstrVectorCopy(infile);
    o->dout = bfromcstr(outdir);
    
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*n_partition);
    o->end_kmerbit = malloc(sizeof(*o->end_kmerbit)*n_partition);
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    o->inbuf2 = malloc(sizeof(*o->inbuf2) * size_inbuf);
    if (o->infile == NULL ||
        o->dout == NULL ||
        o->end_outbuf == NULL ||
        o->end_kmerbit == NULL ||
        o->inbuf == NULL ||
        o->inbuf2 == NULL) {
        alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->end_outbuf, 0, sizeof(*o->end_outbuf)*n_partition);
    memset(o->end_kmerbit, 0, sizeof(*o->end_kmerbit)*n_partition);
    memset(o->inbuf, 0, sizeof(*o->inbuf) * size_inbuf);
    memset(o->inbuf2, 0, sizeof(*o->inbuf2) * size_inbuf);
    
    // 2D
    /* I could have created outbuf in the above. But, when errors occur,
     * it would cause memory error because I would not know whether the
     * memory for each element was allocated or not. This would cause
     * the program to crash in the cleanup function
     * or alder_kmer_readwriter_destroy.
     */
    o->outbuf = malloc(sizeof(*o->outbuf) * n_partition);
    if (o->outbuf == NULL) {
        alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->outbuf, 0, sizeof(*o->outbuf) * n_partition);
    for (int j = 0; j < n_partition; j++) {
        o->outbuf[j] = malloc(sizeof(*o->outbuf[j]) * size_outbuf);
        if (o->outbuf[j] == NULL) {
            alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
        memset(o->outbuf[j],0,sizeof(*o->outbuf[j]) * size_outbuf);
    }
    // 2D
    o->kmerbuf = malloc(sizeof(*o->kmerbuf) * n_partition);
    if (o->kmerbuf == NULL) {
        alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->kmerbuf, 0, sizeof(*o->kmerbuf) * n_partition);
    for (int j = 0; j < n_partition; j++) {
        o->kmerbuf[j] = malloc(sizeof(*o->kmerbuf[j]) * kmer_size * n_encoder);
        if (o->kmerbuf[j] == NULL) {
            alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
            return NULL;
        }
        memset(o->kmerbuf[j], 0, sizeof(*o->kmerbuf[j]) * kmer_size * n_encoder);
    }
    
    /* Create n_partition files in directory dout. */
    s = alder_file_mkdir(outdir);
    if (s != 0) {
        alder_kmer_readwriter_destroy(o);
        alder_loge(ALDER_ERR_FILE, "failed to make directoy: %s",
                   outdir);
        return NULL;
    }
    
    o->fpout = malloc(sizeof(*o->fpout)*n_partition);
    if (o->fpout == NULL) {
        alder_kmer_readwriter_destroy_with_error(o, ALDER_ERR_MEMORY);
        return NULL;
    }
    memset(o->fpout, 0, sizeof(*o->fpout)*n_partition);
    for (int i = 0; i < n_partition; i++) {
        bstring bfpar = bformat("%s/%s-%d-%d.par",
                                outdir, outfile, i_iteration, i);
        if (bfpar == NULL) {
            alder_kmer_readwriter_destroy_with_error
            (o, ALDER_ERR_MEMORY);
            
            return NULL;
        }
        o->fpout[i] = fopen(bdata(bfpar), "wb");
        if (o->fpout[i] == NULL) {
            alder_loge(ALDER_ERR_FILE, "failed to make a file: %s",
                       bdata(bfpar));
            bdestroy(bfpar);
            alder_kmer_readwriter_destroy_with_error
            (o, ALDER_ERR_MEMORY);
            
            return NULL;
        }
        bdestroy(bfpar);
    }
    
    o->ec = malloc(sizeof(*o->ec)*n_encoder);
    if (o->ec == NULL) {
        alder_kmer_readwriter_destroy_with_error
        (o, ALDER_ERR_MEMORY);
        
        return NULL;
    }
    memset(o->ec, 0, sizeof(*o->ec)*n_encoder);
    for (int i = 0; i < n_encoder; i++) {
        o->ec[i] = alder_kmer_encoder_create(i,
                                             i_iteration,
                                             n_iteration,
                                             n_partition,
                                             kmer_size,
                                             size_inbuf,
                                             size_outbuf);
        if (o->ec[i] == NULL) {
            alder_kmer_readwriter_destroy_with_error
            (o, ALDER_ERR_MEMORY);
            
            return NULL;
        }
    }
    assert(o->fpin == NULL);
    alder_log5("Finish - Creating fileseq_kmer_thread_readwriter...");
    return o;
}

void alder_kmer_readwriter_destroy_with_error
(alder_kmer_readwriter_t *o, int s)
{
    if (s != 0) {
        alder_loge(s, "cannot create fileseq_kmer_thread_readwriter");
    }
    alder_kmer_readwriter_destroy(o);
}

/* This function destroys the argument object for a readwrite thread.
 */
void alder_kmer_readwriter_destroy
(alder_kmer_readwriter_t *o)
{
    if (o != NULL) {
        if (o->ec != NULL) {
            for (int j = 0; j < o->n_encoder; j++) {
                alder_kmer_encoder_destroy(o->ec[j]);
            }
            XFREE(o->ec);
        }
        
        if (o->fpout != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFCLOSE(o->fpout[j]);
                o->fpout[j] = NULL;
            }
            XFREE(o->fpout);
        }
        
        if (o->outbuf != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFREE(o->outbuf[j]);
            }
            XFREE(o->outbuf);
        }
        
        if (o->kmerbuf != NULL) {
            for (int j = 0; j < o->n_np; j++) {
                XFREE(o->kmerbuf[j]);
            }
            XFREE(o->kmerbuf);
        }
        
        assert(o->fpin == NULL);
        
        XFREE(o->end_kmerbit);
        XFREE(o->inbuf2);
        XFREE(o->inbuf);
        XFREE(o->end_outbuf);
        bdestroy(o->dout);
        bstrVectorDelete(o->infile);
        XFREE(o);
    }
}

/* This function opens an input file among the input files.
 * returns
 * 0 on success
 * 1 if an error occurs or no input file is available.
 */
static int
readwriter_open_infile
(alder_kmer_readwriter_t *o)
{
    int fp = -1;
    gzFile fpgz = NULL;
    BZFILE *fpbz = NULL;
    assert(o != NULL);
    assert(o->infile != NULL);
    assert(o->infile->qty > 0);
    assert(o->i_infile >= 0);
    
#if 0
    if (o->fpin != NULL) {
        XFCLOSE(o->fpin);
        o->i_infile++;
    }
#endif
    if (o->fx != NULL) {
        if (o->type_infile & ALDER_FILETYPE_GZ) {
            gzclose(o->fx);
        } else if (o->type_infile & ALDER_FILETYPE_BZ) {
            BZ2_bzclose(o->fx);
        } else {
            close((int)((intptr_t)o->fx));
        }
        o->i_infile++;
    }
    if (o->i_infile < o->infile->qty) {
        char *fn = bdata(o->infile->entry[o->i_infile]);
        o->type_infile = alder_fileseq_type(fn);
        
#if 0
        o->fpin = fopen(fn, "r");
        if (o->fpin == NULL) {
            alder_loge(ALDER_ERR_FILE, "cannot open file %s - %s",
                       fn, strerror(errno));
            return ALDER_STATUS_ERROR;
        }
#endif
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
    } else {
        return ALDER_STATUS_ERROR;
    }
    return ALDER_STATUS_SUCCESS;
}

/* This function copies the output results of an encoder to the readwriter's
 * output buffer, which would be written to a file.
 * returns
 * true if the encoder's output is empty, otherwise false.
 *
 * The number of output buffers is equal to the number of partitions.
 */
bool
alder_kmer_readwriter_copyEncoderOutput(alder_kmer_readwriter_t *o,
                                        alder_kmer_encoder_t *e)
{
    bool isEmptyOutbuf = true;
    alder_log5("rwriter(): trying to copy readwriter's out buffer...");
    for (int i_np = 0; i_np < o->n_np; i_np++) {
        alder_log5("rwriter(): out buffer[p:%d](n:%d) of encoder(%d)",
                   i_np, e->end_outbuf[i_np], selected_encoder);
        size_t r = e->end_outbuf[i_np];
        if (r > 0) {
            assert(4*r % o->k == 0);
            alder_log3("rwriter( ): copying output[%d] %zu B, kmer=%zu",
                       i_np, r, 4*r/o->k);
            isEmptyOutbuf = false;
            assert(4 * e->end_outbuf[i_np] % o->k == 0);
            memcpy(o->outbuf[i_np], e->outbuf[i_np], e->end_outbuf[i_np]);
            o->end_outbuf[i_np] = e->end_outbuf[i_np];
            e->end_outbuf[i_np] = 0;
        }
    }
    return isEmptyOutbuf;
}

bool
alder_kmer_readwriter_copyEncoderFinalOutput(alder_kmer_readwriter_t *o,
                                             alder_kmer_encoder_t *e)

{
    bool isEmptyOutbuf = true;
    uint8_t x1 = 1;
    alder_log5("readwriter(): trying to copy readwriter's out buffer...");
    for (int i_np = 0; i_np < o->n_np; i_np++) {
        alder_log5("readwriter(): out buffer[p:%d](n:%d) of encoder(%d)",
                   i_np, e->end_outbuf[i_np], selected_encoder);
        
        if (e->end_outbuf[i_np] > 0) {
            isEmptyOutbuf = false;
            // Copy e->outbuf to o->kmerbuf
            // e->outbuf: e->end_outbuf - 1 (bytes) + e->end_kmerbit (bits)
            // o->kmerbuf: o->end_kmerbit
//            size_t n_srcbit = e->end_outbuf[i_np] * 8 + e->end_kmerbit[i_np];
            size_t n_srcbit = e->end_kmerbit[i_np];
            size_t j = o->end_kmerbit[i_np];
            alder_log3("rwriter( ): copying output[%d] %zu bits, kmer=%zu",
                       i_np, n_srcbit, n_srcbit/o->k);
            for (size_t i = 0; i < n_srcbit; i++, j++) {
                bool isTrue;
                // src
                size_t sx = i / 8;
                size_t sy = 7 - i % 8;
                uint8_t sc = e->outbuf[i_np][sx];
                if (((sc >> sy) & x1) == x1) {
                    isTrue = true;
                } else {
                    isTrue = false;
                }
                // dest
                size_t dx = j / 8;
                size_t dy = 7 - j % 8;
                uint8_t dc = o->kmerbuf[i_np][dx];
                if (isTrue) {
                    dc |= (x1 << dy);
                } else {
                    dc &= ~(x1 << dy);
                }
                o->kmerbuf[i_np][dx] = dc;
            }
            o->end_kmerbit[i_np] = j;
            e->end_outbuf[i_np] = 0;
        }
    }
    return isEmptyOutbuf;
}

/* This function writes the content of an output buffer in the readwriter.
 * The number of output buffers is equal to the number of partitions.
 */
bool
alder_kmer_readwriter_write(alder_kmer_readwriter_t *o)
{
    for (int j = 0; j < o->n_np; j++) {
        if (o->end_outbuf[j] > 0) {
            size_t r = fwrite(o->outbuf[j], sizeof(*o->outbuf[j]),
                              o->end_outbuf[j], o->fpout[j]);
            assert((4*r % o->k) == 0);
            alder_log2("rwriter( ): writing output[%d] %zu B, kmer=%zu",
                       j, r, 4*r/o->k);
            o->n_kmer += (4*r/o->k);
            if (r != o->end_outbuf[j]) {
                // Fatal Error!
                alder_loge(1, "readwriter(): fwrite failed.");
                assert(0);
                abort();
            }
            o->end_outbuf[j] = 0;
        }
    }
    return true;
}

/* This function writes all of the end
 */
bool
alder_kmer_readwriter_flush(alder_kmer_readwriter_t *o)
{
    for (int j = 0; j < o->n_np; j++) {
        if (o->end_kmerbit[j] > 0) {
            size_t size_kmerbyte = ALDER_BYTESIZE_KMER(o->end_kmerbit[j],8);
            size_t r = fwrite(o->kmerbuf[j], sizeof(*o->kmerbuf[j]),
                              size_kmerbyte, o->fpout[j]);
            alder_log2("rwriter( ): flushing output[%d] %zu, kmer=%zu",
                       j, r, 4*r/o->k);
            if (r != size_kmerbyte) {
                // Fatal Error!
                alder_loge(1, "readwriter(): fwrite failed.");
                assert(0);
                abort();
            }
            o->n_kmer += (4*r/o->k);
            o->end_kmerbit[j] = 0;
        }
        // FIXME: K=1,2,3 -> 2bits are 0 or A which denotes no data.
        // K=1: subtract 2bits counts of A.
        // K=2: subtract 2bits counts of AA.
        // K=3: subtract 2bits counts of AAA.
    }
    return true;
}

int time_main(void)
{
    time_t current_time;
    char* c_time_string;
    
    /* Obtain current time as seconds elapsed since the Epoch. */
    current_time = time(NULL);
    
    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to compute the current time.");
        return EXIT_FAILURE;
    }
    
    /* Convert to local time format. */
    c_time_string = ctime(&current_time);
    
    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.");
        return EXIT_FAILURE;
    }
    
    /* Print to stdout. */
    (void) printf("Current time is %s", c_time_string);
    return EXIT_SUCCESS;
}

static void
alder_kmer_encode_token_reset(alder_kmer_encoder_t *e)
{
    e->i_inbuf = 0;
}

/* This function returns 0,1,2,3 for DNA. 
 * returns 
 * 0,1,2,3 for A,C,T,G.
 * 4 for others.
 * 5 for the end of buffer.
 */
int
alder_kmer_encode_token(alder_kmer_encoder_t *e)
{
    char c;
    if (e->i_inbuf % 10000 == 0) {
        alder_log4("i_inbuf(%zu): %zu", e->end_inbuf, e->i_inbuf);
    }
    if (e->i_inbuf < e->end_inbuf) {
        c = e->inbuf[e->i_inbuf++];
    } else {
//        assert(0);
        return 5;
    }
    if (e->type_infile & ALDER_FILETYPE_ISFASTA) {
        if (c == '>' || c == '\n') {
            while (c != '\n') {
                c = e->inbuf[e->i_inbuf++];
            }
            if (e->i_inbuf < e->end_inbuf) {
                c = e->inbuf[e->i_inbuf++];
            } else {
                e->n_byte += e->i_inbuf;
                return 5;
            }
        }
    } else if (e->type_infile & ALDER_FILETYPE_ISFASTQ) {
        if (c == '@') {
            // header
            while (c != '\n') {
                c = e->inbuf[e->i_inbuf++];
            }
            c = e->inbuf[e->i_inbuf++];
            assert(c != '\n');
        } else if (c == '\n') {
            // +
            c = e->inbuf[e->i_inbuf++];
            assert(c == '+');
            while (c != '\n') {
                c = e->inbuf[e->i_inbuf++];
            }
            // quality score
            c = e->inbuf[e->i_inbuf++];
            while (c != '\n') {
                c = e->inbuf[e->i_inbuf++];
            }
            if (e->i_inbuf < e->end_inbuf) {
                c = 'X';
            } else {
                e->n_byte += e->i_inbuf;
                return 5;
            }
        }
    } else {
        assert(e->type_infile & ALDER_FILETYPE_ISSEQ);
        if (c == '\n') {
            if (e->i_inbuf < e->end_inbuf) {
                // no code.
            } else {
                e->n_byte += e->i_inbuf;
                return 5;
            }
        }
    }
    int ci = alder_dna_char2int(c);
    if (ci < 4) {
        e->n_letter++;
    }
    return ci;
}


/* This function estimates the input buffer size.
 * Controller and worker threads have one input buffer and multiple out buffers.
 * The total memory (roughly) fits to the size of these buffers.
 * given
 * M - memory in megabytes
 * nt - number of threads
 * k - Kmer size
 * procedure
 * x - input buffer size (this is the target size)
 * (x - k + 1) - number of Kmers in a sequence of length x
 * (x - k + 1)k - total number of bases in all of the Kmers
 * (x - k + 1)k/4 - total number of bytes, 4 bases in a byte
 * (x + (x - k + 1)k/4 + 1) - total size of buffer per thread
 * zM - memory size in bytes
 * zM ~ (x + (x - k + 1)k/4 + 1)nt - Solve this for x.
 */
//size_t
//alder_fasta_kmer_encode_inbuf_size(int M, int nt, int k)
//{
//    size_t zM = (size_t) M;
//    size_t znt = (size_t) nt;
//    size_t zk = (size_t) k;
//    zM <<= 20;
//    size_t v = (zM/znt - 1 + (zk-1)*zk/4)*4/(4+zk);
//    return v;
//}

/* This function estimates the output buffer size per partition or
 * per open partition file.
 * x must be computed first. The input buffer is computed using the function
 * above.
 * given
 * x - number of input buffer size
 * zk - Kmer size
 * np - number of partition files
 * procedure
 * (x - k + 1) - number of Kmers in a sequence of length x
 * (x - k + 1)k - total number of bases in all of the Kmers
 * (x - k + 1)k/4 - total number of bytes, 4 bases in a byte
 * (x - k + 1)k/4/np - number of bytes per partition file
 */
//size_t
//alder_fasta_kmer_encode_outbuf_size(size_t x, uint64_t np, int k)
//{
//    size_t znp = (size_t) np;
//    size_t zk = (size_t) k;
//    size_t v = ((x - zk + 1)*zk + 4)/4/znp + 1;
//    return v;
//}


void encoder_append_kmerbuf_n_kmer(alder_kmer_encoder_t *b,
                                   uint64_t *n_kmer)
{
    /* Put unfinished outbuffer back to outbuf. */
    for (uint64_t i = 0; i < b->n_np; i++) {
        size_t x = b->end_kmerbuf[i];
        size_t nk = b->nk[i];
        assert((x * 4 + nk) % b->k == 0);
        n_kmer[i] = (x * 4 + nk) / b->k;
    }
}

/* This function puts the kmerbuf back to outbuf.
 */
void encoder_append_kmerbuf(alder_kmer_encoder_t *b)
{
    /* Put unfinished outbuffer back to outbuf. */
    for (uint64_t i = 0; i < b->n_np; i++) {
        assert(b->end_outbuf[i] == 0);
        size_t x = b->end_kmerbuf[i];
        size_t nk = b->nk[i];
        assert((x * 4 + nk) % b->k == 0);
        if (x > 0) {
            memcpy(b->outbuf[i], b->kmerbuf[i], sizeof(*b->outbuf[i]) * x);
        }
        alder_log2("encoder(%d): append %zu kmers in kmerbuf[%d] to outbuf",
                   b->i, (x * 4 + nk) / b->k, i);
    }
}

/* This function saves a tail of outbput to kmerbuf if the tail does not
 * fit to the size of K-many "bytes".
 */
void encoder_save_kmerbuf(alder_kmer_encoder_t *b)
{
    for (uint64_t i = 0; i < b->n_np; i++) {
        size_t end_outbuf = b->end_kmerbuf[i];
        assert(b->end_outbuf[i] == end_outbuf / b->k * b->k);
        end_outbuf -= b->end_outbuf[i];
        b->end_kmerbuf[i] = end_outbuf;
        size_t nk = b->nk[i];
        assert((end_outbuf * 4 + nk) % b->k == 0);
        if (end_outbuf > 0) {
            assert(b->kmerbuf[i] != NULL);
            memcpy(b->kmerbuf[i],
                   b->outbuf[i] + b->end_outbuf[i],
                   sizeof(*b->kmerbuf[i]) * end_outbuf);
        }
        alder_log2("encoder(%d): save %zu kmers in kmerbuf[%d]",
                   b->i, (end_outbuf * 4 + nk) / b->k, i);
    }
}
