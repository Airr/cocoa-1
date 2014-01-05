
/* #line 1 "alder_fastq_count_kmer.rl" */
/**
 * This file, alder_fastq_count_kmer.c, is part of alder-fastq.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fastq is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fastq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "alder_file.h"
#include "alder_fastq_count_kmer.h"

#define BUFSIZE (1 << 23)

/* See https://github.com/lomereiter/bioragel/blob/master/examples/d/fastq.rl
 */

/* #line 33 "alder_fastq_count_kmer.c" */
static const char _fastqKmer_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 2, 
	1, 2
};

static const char _fastqKmer_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 6, 7, 
	8, 9, 11, 15, 16
};

static const char _fastqKmer_trans_keys[] = {
	64, 10, 10, 10, 10, 13, 43, 10, 
	10, 33, 126, 10, 13, 33, 126, 10, 
	64, 0
};

static const char _fastqKmer_single_lengths[] = {
	0, 1, 1, 1, 1, 2, 1, 1, 
	1, 0, 2, 1, 1
};

static const char _fastqKmer_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 0, 0
};

static const char _fastqKmer_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 11, 13, 
	15, 17, 19, 23, 25
};

static const char _fastqKmer_trans_targs[] = {
	2, 0, 0, 3, 4, 3, 0, 5, 
	6, 5, 5, 7, 0, 9, 8, 9, 
	8, 10, 0, 12, 11, 10, 0, 12, 
	0, 2, 0, 0
};

static const char _fastqKmer_trans_actions[] = {
	0, 0, 0, 1, 0, 0, 0, 3, 
	5, 7, 3, 0, 0, 0, 1, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0
};

static const int fastqKmer_start = 1;
static const int fastqKmer_first_final = 12;
static const int fastqKmer_error = 0;

static const int fastqKmer_en_main = 1;


/* #line 56 "alder_fastq_count_kmer.rl" */


static int alder_fastq_count_kmer_file(const char *fn, int kmer_size, uint64_t *volume);
static int alder_fastq_count_kmer_gzip(const char *fn, int kmer_size, uint64_t *volume);

/* This function counts kmers (size of kmer_size) in a fastq file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
int alder_fastq_count_kmer(const char *fn, int kmer_size, uint64_t *volume)
{
    int s = alder_file_isgzip(fn);
    if (s) {
        alder_fastq_count_kmer_gzip(fn, kmer_size, volume);
    } else {
        alder_fastq_count_kmer_file(fn, kmer_size, volume);
    }
    return s;
}

static int alder_fastq_count_kmer_file(const char *fn, int kmer_size, uint64_t *volume)
{
    
/* #line 80 "alder_fastq_count_kmer.rl" */
    int cs;
//    int have = 0;
    char *buf = malloc(sizeof(*buf) * BUFSIZE);
    *volume = 0;
    uint64_t t1 = 0;
    
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    
/* #line 124 "alder_fastq_count_kmer.c" */
	{
	cs = fastqKmer_start;
	}

/* #line 92 "alder_fastq_count_kmer.rl" */
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        size_t len = fread(buf, sizeof(char), BUFSIZE, fp);
        if (len == 0) {
            break;
        }
        
        p = buf;
        pe = buf + len;
        if (len < BUFSIZE) {
            eof = pe;
        }
        
/* #line 144 "alder_fastq_count_kmer.c" */
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _fastqKmer_trans_keys + _fastqKmer_key_offsets[cs];
	_trans = _fastqKmer_index_offsets[cs];

	_klen = _fastqKmer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _fastqKmer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	cs = _fastqKmer_trans_targs[_trans];

	if ( _fastqKmer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastqKmer_actions + _fastqKmer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 31 "alder_fastq_count_kmer.rl" */
	{
        t1 = 0;
    }
	break;
	case 1:
/* #line 34 "alder_fastq_count_kmer.rl" */
	{
        t1++;
    }
	break;
	case 2:
/* #line 37 "alder_fastq_count_kmer.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 237 "alder_fastq_count_kmer.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

/* #line 106 "alder_fastq_count_kmer.rl" */
    }
    fclose(fp);
    return 0;
}

static int alder_fastq_count_kmer_gzip(const char *fn, int kmer_size, uint64_t *volume)
{
    
/* #line 114 "alder_fastq_count_kmer.rl" */
    int cs;
//    int have = 0;
    char buf[BUFSIZE];
    *volume = 0;
    uint64_t t1 = 0;
    
    gzFile fp = gzopen(fn, "r");
//    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    
/* #line 273 "alder_fastq_count_kmer.c" */
	{
	cs = fastqKmer_start;
	}

/* #line 127 "alder_fastq_count_kmer.rl" */
    while ( 1 ) {
        char *p;
        char *pe;
        char *eof;
        int len = gzread ((gzFile)fp, buf, BUFSIZE);
//        size_t len = fread(buf, sizeof(char), BUFSIZE, fp);
        if (len < 0) {
            break;
        } else if (len == 0) {
            break;
        }
        p = buf;
        pe = buf + len;
        if (len < BUFSIZE) {
            eof = pe;
        }
        
/* #line 296 "alder_fastq_count_kmer.c" */
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _fastqKmer_trans_keys + _fastqKmer_key_offsets[cs];
	_trans = _fastqKmer_index_offsets[cs];

	_klen = _fastqKmer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _fastqKmer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	cs = _fastqKmer_trans_targs[_trans];

	if ( _fastqKmer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastqKmer_actions + _fastqKmer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 31 "alder_fastq_count_kmer.rl" */
	{
        t1 = 0;
    }
	break;
	case 1:
/* #line 34 "alder_fastq_count_kmer.rl" */
	{
        t1++;
    }
	break;
	case 2:
/* #line 37 "alder_fastq_count_kmer.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 389 "alder_fastq_count_kmer.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

/* #line 144 "alder_fastq_count_kmer.rl" */
    }
    gzclose(fp);
//    fclose(fp);
    return 0;
}