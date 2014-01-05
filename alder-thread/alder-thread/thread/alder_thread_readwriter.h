/**
 * This file, alder_thread_readwriter.h, is part of alder-thread.
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

#ifndef alder_thread_alder_thread_readwriter_h
#define alder_thread_alder_thread_readwriter_h

#include <stdint.h>
#include "bstrlib.h"

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif


__BEGIN_DECLS

enum {
    ALDER_THREAD_ENCODER_INBUFFER_EMPTY,
    ALDER_THREAD_ENCODER_INBUFFER_FULL,
    ALDER_THREAD_ENCODER_CANNOT_EXIT,
    ALDER_THREAD_ENCODER_CAN_EXIT
};


/* Encoder thread accesses this type.
 * i - encoder ID
 * status - 0
 * n_outbuf - number of out buffers, same as the number of out files
 * size_inbuf - size of the input buffer
 * size_outbuf - size of the output buffer
 * end_inbuf - ending position of input buffer e.g., 0 empty, size_inbuf full
 * end_outbuf - ending positions of output buffer e.g., 0 empty, size_outbuf full
 * inbuf - in buffer (size_inbuf)
 * outbuf - out buffers (n_outbuf x size_outbuf)
 *
 *
 */
typedef struct alder_thread_encoder_struct {
    int i;
    
    int status;
    
    int n_outbuf;
    int size_inbuf;
    int size_outbuf;
    int end_inbuf;
    int *end_outbuf;
    char *inbuf;
    char **outbuf;
} alder_thread_encoder_t;

alder_thread_encoder_t * alder_thread_encoder_create(int id,
                                                     int n_outbuf,
                                                     int size_inbuf,
                                                     int size_outbuf);
                                                     
void alder_thread_encoder_destroy(alder_thread_encoder_t *o);
        
                                                     


/* Readwriter thread accesses this type.
 * i - thread ID
 * n_encoder - number of encoder threads
 * bfnin - input file name
 * bfnout - output file names
 * fpin - input file pointer
 * fpout - output file pointers
 * size_inbuf - size of the input buffer
 * begin_inbuf - beginning point in the input buffer
 * end_inbuf - ending position of input buffer e.g., 0 empty, size_inbuf full
 * end_outbuf - ending position of output buffer e.g., 0 empty, size_outbuf full
 * size_outbuf - size of the output buffer
 * inbuf - input buffer
 * outbuf - output buff
 */
typedef struct alder_thread_readwriter_struct {
    bstring bfnin;
    struct bstrList *bfnout;
    FILE *fpi;
    FILE **fpo;
    
    int size_inbuf;
    int begin_inbuf;
    int end_inbuf;
    int *end_outbuf;
    int size_outbuf;
    char *inbuf;
    char **outbuf;
} alder_thread_readwriter_t;

alder_thread_readwriter_t * alder_thread_readwriter_create(bstring bfnin,
                                                           struct bstrList *bfnout,
                                                           int size_inbuf,
                                                           int size_outbuf);
void alder_thread_readwriter_destroy(alder_thread_readwriter_t *o);

typedef struct alder_thread_rwe_struct {
    int n_encoder;
    alder_thread_readwriter_t *rw;
    alder_thread_encoder_t **ec;
} alder_thread_rwe_t;

alder_thread_rwe_t * alder_thread_rwe_create(int n_encoder,
                                             bstring bfnin,
                                             struct bstrList *bfnout,
                                             int size_inbuf1,
                                             int size_outbuf1,
                                             int size_inbuf2,
                                             int size_outbuf2);
void alder_thread_rwe_destroy(alder_thread_rwe_t *o);

int alder_thread_readwriter(int n_thread);

__END_DECLS


#endif /* alder_thread_alder_thread_readwriter_h */
