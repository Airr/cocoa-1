/**
 * This file, alder_thread_readwriter.c, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bstrmore.h"
#include "alder_cmacro.h"
#include "alder_thread_readwriter.h"

/* This is a model of threads where a single controller prepares an input data
 * for worker threads to consume. Processed data by workers are sent back to
 * the controller thread. The controller thread do something about it.
 * The problem that I wish to solve is to encode sequence data files to kmers.
 * The input file can be huge, which can be benefited by multi-thread
 * technique. A single ioreadwriter thread reads from a single large file and
 * writes to multiple files. The ioreadwriter would read a chunk of data
 * from a large file. It distributes the chunk to multiple encoder threads,
 * which do their jobs and notify the ioreadwriter of the completion of the
 * encoding. The ioreadwriter retrieves the encoded data, and writes it in
 * output files.
 */
static pthread_mutex_t ec2rw_mutex;
static pthread_cond_t ec2rw_cv;
static int selected_encoder;

static void *encoder(void *t)
{
    alder_thread_encoder_t *b = (alder_thread_encoder_t*)t;
    printf("Starting encoder(%d): thread.\n", b->i);
    b->end_inbuf = 1; // Just for entering while loop.
    for (int i = 0; i < b->n_outbuf; i++) {
        b->end_outbuf[i] = 0;
    }
    b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
    
    while (b->status != ALDER_THREAD_ENCODER_CAN_EXIT) {
        // 1. Keep notifying Readwriter that an Encoder needs inbuffer.
        //    Wait till Readwriter prepares the inbuffer for this Encoder.
        if (b->end_inbuf > 0) {
            printf("encoder(%d): waiting for readwriter\n", b->i);
        } else {
            printf("encoder(%d): last call readwriter\n", b->i);
            for (int i = 0; i < b->n_outbuf; i++) {
                b->end_outbuf[i] = 0;
            }
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
        while (b->status == ALDER_THREAD_ENCODER_INBUFFER_EMPTY) {
            pthread_mutex_lock(&ec2rw_mutex);
            selected_encoder = b->i;
            pthread_cond_signal(&ec2rw_cv);
            pthread_mutex_unlock(&ec2rw_mutex);
        }
        
        // 2. Work on encoding.
        if (b->end_inbuf > 0) {
            for (int i = 0; i < b->end_inbuf; i++) {
                int w_outbuf = i%b->n_outbuf;
                char *outbuf = b->outbuf[w_outbuf];
                int end_outbuf = b->end_outbuf[w_outbuf];
                outbuf[end_outbuf] = b->inbuf[i];
                end_outbuf++;
                b->end_outbuf[w_outbuf] = end_outbuf;
            }
            b->status = ALDER_THREAD_ENCODER_INBUFFER_EMPTY;
        }
    }
    printf("encoder(%d): exiting\n", b->i);
    pthread_exit(NULL);
}

static void *readwriter(void *t)
{
    printf("Starting readwriter(): thread.\n");
    alder_thread_rwe_t *a = (alder_thread_rwe_t*)t;
    int which_encoder = 0;
    
    // 1. Reads a chunk of data from the input file.
    int len_inbuf = (int)fread(a->rw->inbuf, 1, a->rw->size_inbuf, a->rw->fpi);
    a->rw->end_inbuf = len_inbuf;
    
    // 2. Repeat if there is a chunk of data.
    int remaining_encoder = a->n_encoder;
    while (remaining_encoder > 0) {
        // 3. Wait for a call from an Encoder.
        pthread_mutex_lock(&ec2rw_mutex);
        pthread_cond_wait(&ec2rw_cv, &ec2rw_mutex);
        // 4. Locate the Encoder in idle.
        which_encoder = selected_encoder;
        assert(which_encoder < a->n_encoder);
        alder_thread_encoder_t *ec = a->ec[which_encoder];
        // 5. Copy outbuffer of the encoder if needed
        int is_empty_outbuf = 1;
        for (int j = 0; j < a->rw->bfnout->qty; j++) {
            if (ec->end_outbuf[j] > 0) {
                is_empty_outbuf = 0;
                memcpy(a->rw->outbuf[j], ec->outbuf[j], ec->end_outbuf[j]);
                a->rw->end_outbuf[j] = ec->end_outbuf[j];
                ec->end_outbuf[j] = 0;
            }
        }
        // 6. Send part of chunk to encoder if needed.
        if (a->rw->end_inbuf > 0) {
            memcpy(ec->inbuf, a->rw->inbuf, a->rw->end_inbuf);
            ec->end_inbuf = a->rw->end_inbuf;
            a->rw->end_inbuf = 0;
            // 7. Let Encoder work on data.
            ec->status = ALDER_THREAD_ENCODER_INBUFFER_FULL;
            printf("readwriter(): full wakeup %d\n", which_encoder);
        } else if (is_empty_outbuf == 1) {
            remaining_encoder--;
            ec->end_inbuf = 0;
            ec->status = ALDER_THREAD_ENCODER_CAN_EXIT;
            printf("readwriter(): --- wakerup %d\n", which_encoder);
        } else {
            // The inbuffer is zero, but there were output.
            // So, encode might need to work on more data.
            // This is not the case for this example where
            // all of input are processed to create output.
            // Or, when output are there, input should be empty
            // in this case. But, when input can be processed
            // partially, I need this "else if is_empty_outbuf".
            ec->end_inbuf = 0;
            ec->status = ALDER_THREAD_ENCODER_CANNOT_EXIT;
            printf("readwriter(): ??? wakeup %d\n", which_encoder);
        }
        pthread_mutex_unlock(&ec2rw_mutex);
        
        // 8. Write outbuff to output files.
        for (int j = 0; j < a->rw->bfnout->qty; j++) {
            if (a->rw->end_outbuf[j] > 0) {
                fwrite(a->rw->outbuf[j], 1, a->rw->end_outbuf[j], a->rw->fpo[j]);
                a->rw->end_outbuf[j] = 0;
            }
        }
        // 9(1). Reads a chunk of data from the input file.
        if (len_inbuf > 0) {
            len_inbuf = (int)fread(a->rw->inbuf, 1, a->rw->size_inbuf, a->rw->fpi);
            a->rw->end_inbuf = len_inbuf;
        }
    }
    pthread_exit(NULL);
}


alder_thread_encoder_t * alder_thread_encoder_create(int id,
                                                     int n_outbuf,
                                                     int size_inbuf,
                                                     int size_outbuf)
{
    alder_thread_encoder_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->i = id;
    o->n_outbuf = n_outbuf;
    o->size_inbuf = size_inbuf;
    o->size_outbuf = size_outbuf;
    o->end_inbuf = 0;
    o->end_outbuf = 0;
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*n_outbuf);
    o->inbuf = malloc(size_inbuf);
    o->outbuf = malloc(sizeof(*o->outbuf)*n_outbuf);
    for (int i = 0; i < n_outbuf; i++) {
        o->outbuf[i] = malloc(sizeof(*o->outbuf[i])*size_outbuf);
        o->end_outbuf[i] = 0;
    }
    return o;
}

void alder_thread_encoder_destroy(alder_thread_encoder_t *o)
{
    if (o != NULL) {
        for (int i = 0; i < o->n_outbuf; i++) {
            XFREE(o->outbuf[i]);
        }
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        XFREE(o->end_outbuf);
    }
}

alder_thread_readwriter_t * alder_thread_readwriter_create(bstring bfnin,
                                                           struct bstrList *bfnout,
                                                           int size_inbuf,
                                                           int size_outbuf)
{
    alder_thread_readwriter_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    
    o->bfnin = bstrcpy(bfnin);
    o->bfnout = bstrVectorCopy(bfnout);
    
    o->begin_inbuf = 0;
    o->end_inbuf = 0;
    o->end_outbuf = malloc(sizeof(*o->end_outbuf)*o->bfnout->qty);
    o->size_inbuf = size_inbuf;
    o->size_outbuf = size_outbuf;
    o->inbuf = malloc(sizeof(*o->inbuf) * size_inbuf);
    o->outbuf = malloc(sizeof(*o->outbuf) * o->bfnout->qty);
    for (int j = 0; j < o->bfnout->qty; j++) {
        o->outbuf[j] = malloc(sizeof(*o->outbuf[j]) * size_outbuf);
    }
    
    o->fpi = fopen(bdata(o->bfnin), "r");
    if (o->fpi == NULL) {
        perror("cannot open the input file.");
    }
    o->fpo = malloc(sizeof(*o->fpo) * o->bfnout->qty);
    for (int j = 0; j < o->bfnout->qty; j++) {
        o->fpo[j] = fopen(bdata(o->bfnout->entry[j]), "w");
        if (o->fpo[j] == NULL) {
            perror("cannot open an output file.");
        }
    }
    return o;
}

void alder_thread_readwriter_destroy(alder_thread_readwriter_t *o)
{
    if (o != NULL) {
        for (int j = 0; j < o->bfnout->qty; j++) {
            fclose(o->fpo[j]);
        }
        XFREE(o->fpo);
        fclose(o->fpi);
        for (int j = 0; j < o->bfnout->qty; j++) {
            XFREE(o->outbuf[j]);
        }
        XFREE(o->outbuf);
        XFREE(o->inbuf);
        XFREE(o->end_outbuf);
        bstrVectorDelete(o->bfnout);
        bdestroy(o->bfnin);
    }
}


alder_thread_rwe_t * alder_thread_rwe_create(int n_encoder,
                                             bstring bfnin,
                                             struct bstrList *bfnout,
                                             int size_inbuf1,
                                             int size_outbuf1,
                                             int size_inbuf2,
                                             int size_outbuf2)
{
    alder_thread_rwe_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    o->n_encoder = n_encoder;
    o->rw = alder_thread_readwriter_create(bfnin, bfnout, size_inbuf1, size_outbuf1);
    o->ec = malloc(sizeof(*o->ec)*n_encoder);
    for (int i = 0; i < n_encoder; i++) {
        o->ec[i] = alder_thread_encoder_create(i, bfnout->qty, size_inbuf2, size_outbuf2);
    }
    return o;
}

void alder_thread_rwe_destroy(alder_thread_rwe_t *o)
{
    if (o != NULL) {
        for (int i = 0; i < o->n_encoder; i++) {
            alder_thread_encoder_destroy(o->ec[i]);
        }
        XFREE(o->ec);
        alder_thread_readwriter_destroy(o->rw);
    }
    XFREE(o);
}

int alder_thread_readwriter(int n_thread)
{
    /* Create buffers */
    assert(n_thread > 1);
    int n_encoder = n_thread - 1;
    const int N_OUTBUF= 2;
    const int SIZE_INBUF1 = 10;
    const int SIZE_OUTBUF1 = 5;
    const int SIZE_INBUF2 = 10;
    const int SIZE_OUTBUF2 = 5;
    bstring bfnin = bfromcstr("in");
    struct bstrList *bfnout = bstrVectorCreate(N_OUTBUF);
    bstrVectorAdd(bfnout, "out.1");
    bstrVectorAdd(bfnout, "out.2");
    
    alder_thread_rwe_t *rwe = alder_thread_rwe_create(n_encoder,
                                                      bfnin,
                                                      bfnout,
                                                      SIZE_INBUF1,
                                                      SIZE_OUTBUF1,
                                                      SIZE_INBUF2,
                                                      SIZE_OUTBUF2);
    bdestroy(bfnin);
    bstrVectorDelete(bfnout);
    
    
    selected_encoder = 0;
    pthread_t *threads = malloc(sizeof(*threads)*n_thread);
    pthread_attr_t attr;
    
    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&ec2rw_mutex, NULL);
    pthread_cond_init (&ec2rw_cv, NULL);
    
    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, readwriter, (void *)rwe);
    for (int i = 0; i < n_encoder; i++) {
        pthread_create(&threads[i+1], &attr, encoder, (void *)rwe->ec[i]);
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < n_thread; i++) {
        pthread_join(threads[i], NULL);
    }
    printf ("Main(): Waited on %d  threads. Done.\n", n_thread);
    
    /* Clean up and exit */
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&ec2rw_mutex);
    pthread_cond_destroy(&ec2rw_cv);
    XFREE(threads);
    
    /* Clean up buffers */
    alder_thread_rwe_destroy(rwe);

    
    return 0;
}