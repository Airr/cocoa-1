/**
 * This file, alder_encode_kmer.c, is part of alder-encode.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-encode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-encode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-encode.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "alder_cmacro.h"
#include "alder_encode_number.h"
#include "alder_encode_kmer.h"

alder_encode_kmer_t * alder_encode_kmer_create(int kmer_size, size_t len)
{
    alder_encode_kmer_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    memset(o, 0, sizeof(*o));
    o->k = kmer_size;
    o->len_in = len + 2;
    o->len_out = (len - kmer_size + 1) * 2 * kmer_size / 8 + 1;
    o->end_in = 0;
    o->end_out = 0;
    o->in = malloc(sizeof(*o->in)*o->len_in);
    o->out = malloc(sizeof(*o->out)*o->len_out);
    if (o->in == NULL || o->out == NULL) {
        alder_encode_kmer_destroy(o);
        return NULL;
    }
    memset(o->in, 0, sizeof(*o->in)*o->len_in);
    memset(o->out, 0, sizeof(*o->out)*o->len_out);
    return o;
}

void alder_encode_kmer_destroy(alder_encode_kmer_t *o)
{
    if (o != NULL) {
        XFREE(o->in);
        XFREE(o->out);
        XFREE(o);
    }
}

int alder_encode_kmer_in(alder_encode_kmer_t *o, char *in, size_t len)
{
    assert(len + 2<= o->len_in);
    for (int i = 0; i < len; i++) {
        o->in[i+2] = ALDER_ENCODING_ACGT(in[i]);
    }
    o->end_in = len + 2;
    return 0;
}

int alder_encode_kmer_convert(alder_encode_kmer_t *o)
{
    fprintf(stderr, "Correct it: Assigned value is garbage or undefined.\n");
    assert(0);
    uint8_t *in = o->in;
    uint8_t buf[3];
    int buf_end = 0;
    size_t o_out_i = 0;
    for (int i = 0; i < (o->end_in - 2) - o->k + 1; i++) {
        int kmer_start = i + 2;
        for (int j = 0; j < buf_end; j++) {
            fprintf(stderr, "Here: Assigned value is garbage or undefined.\n");
            assert(0);
            in[kmer_start - (buf_end - j)] = buf[j];
        }
        int kmer_end = kmer_start + o->k;
        int kmer_point = kmer_start - buf_end;
        
        while (kmer_point < kmer_end - 3) {
            // ACGT or 0132 -> 0x1E
            uint8_t x = 0;
            x |= in[kmer_point++]; x <<= 2;
            x |= in[kmer_point++]; x <<= 2;
            x |= in[kmer_point++]; x <<= 2;
            x |= in[kmer_point++];
            assert(o_out_i < o->len_out);
            o->out[o_out_i++] = x;
        }
        for (int j = kmer_point; j < kmer_end; j++) {
            buf[j - kmer_point] = in[j];
        }
        buf_end = kmer_end - kmer_point;
    }
    return 0;
}

int alder_encode_kmer_decode(alder_encode_kmer_t *o)
{
//    uint8_t choose_bit_p[4] = {0xC0,0x30,0x0C,0x03};
    alder_encode_number_t * kmer_number = alder_encode_number_create_for_kmer(o->k);
    
    int shift_bit_p[4] = {6,4,2,0};
    uint8_t *out = o->out;
    int o_out_i = 0;
    int bit_i = 0;
    uint8_t y = 0;
    uint8_t *kmer = malloc(sizeof(*kmer)*o->k);
    memset(kmer, 0, sizeof(*kmer)*o->k);
    for (int i = 0; i < (o->end_in - 2) - o->k + 1; i++) {
        for (int j = 0; j < o->k; j++) {
            int bit_p = bit_i % 4;
            if (bit_p == 0) {
                assert(o_out_i < o->len_out);
                y = out[o_out_i++];
            }
//            kmer[j] = x & choose_bit_p[bit_p];
            kmer[j] = y >> shift_bit_p[bit_p] & 3;
            bit_i++;
        }
        for (int j = 0; j < o->k; j++) {
            char c;
            if (kmer[j] == 0) {
                c = 'A';
            } else if (kmer[j] == 1) {
                c = 'C';
            } else if (kmer[j] == 3) {
                c = 'T';
            } else {
                c = 'G';
            }
            fprintf(stdout, "%c", c);
        }
        fprintf(stdout, "\n");
        alder_encode_number_kmer(kmer_number, kmer);
        alder_encode_number_print(kmer_number);
    }
    XFREE(kmer);
    alder_encode_number_destroy(kmer_number);
    return 0;
}

int alder_encode_kmer_decode_as_number(alder_encode_kmer_t *o)
{
    //    uint8_t choose_bit_p[4] = {0xC0,0x30,0x0C,0x03};
    alder_encode_number_t * kmer_number = alder_encode_number_create_for_kmer(o->k);
    
    int shift_bit_p[4] = {6,4,2,0};
    uint8_t *out = o->out;
    int o_out_i = 0;
    int bit_i = 0;
    uint8_t y = 0;
    int kmer_size = o->k;
    for (int i = 0; i < (o->end_in - 2) - o->k + 1; i++) {
        
        uint64_t x = 0;
        int ni = (kmer_size%32) ? kmer_size/32 : kmer_size/32 - 1;
        for (int j = 0; j < o->k; j++) {
            int bit_p = bit_i % 4;
            if (bit_p == 0) {
                assert(o_out_i < o->len_out);
                y = out[o_out_i++];
            }
            
            ////////////////////////////////////////////////////////////
            uint64_t kmer64 = (uint64_t)(y >> shift_bit_p[bit_p] & 3);
            x = (x | kmer64);
            if ((kmer_size - j - 1) % 32 == 0) {
                kmer_number->n[ni--] = x;
            } else {
                x <<= 2;
            }
            ////////////////////////////////////////////////////////////

            bit_i++;
        }
        alder_encode_number_print(kmer_number);
    }
//    XFREE(kmer);
    alder_encode_number_destroy(kmer_number);
    return 0;
}
