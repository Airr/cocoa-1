
/* #line 1 "alder_fasta.rl" */
/**
 * This file, alder_fasta.c, is part of alder-fasta.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-fasta is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-fasta is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-fasta.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "alder_file.h"
#include "bstrmore.h"
#include "alder_fasta.h"

#define BUFSIZE 1000000
enum { GFFELEMENTSIZE = 100 };
static uint64_t sSizeCapacity = 100;


/* #line 37 "alder_fasta.c" */
static const char _fasta_actions[] = {
	0, 1, 0, 1, 1
};

static const char _fasta_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fasta_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fasta_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fasta_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fasta_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fasta_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fasta_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5
};

static const char _fasta_trans_actions[] = {
	0, 0, 1, 0, 0, 3, 0
};

static const int fasta_start = 1;
static const int fasta_first_final = 5;
static const int fasta_error = 0;

static const int fasta_en_main = 1;


/* #line 46 "alder_fasta.rl" */



/* #line 95 "alder_fasta.c" */
static const char _fastaSize_actions[] = {
	0, 1, 1, 1, 2, 2, 0, 1
	
};

static const char _fastaSize_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fastaSize_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fastaSize_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fastaSize_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fastaSize_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fastaSize_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fastaSize_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5
};

static const char _fastaSize_trans_actions[] = {
	0, 0, 5, 0, 1, 3, 0
};

static const int fastaSize_start = 1;
static const int fastaSize_first_final = 5;
static const int fastaSize_error = 0;

static const int fastaSize_en_main = 1;


/* #line 63 "alder_fasta.rl" */



/* #line 154 "alder_fasta.c" */
static const char _fastaSeqSize_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 2, 
	0, 1
};

static const char _fastaSeqSize_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fastaSeqSize_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fastaSeqSize_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fastaSeqSize_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fastaSeqSize_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fastaSeqSize_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 7, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fastaSeqSize_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5, 2
};

static const char _fastaSeqSize_trans_actions[] = {
	0, 0, 7, 0, 1, 3, 0, 5
};

static const char _fastaSeqSize_eof_actions[] = {
	0, 0, 0, 0, 0, 5
};

static const int fastaSeqSize_start = 1;
static const int fastaSeqSize_first_final = 5;
static const int fastaSeqSize_error = 0;

static const int fastaSeqSize_en_main = 1;


/* #line 92 "alder_fasta.rl" */



/* #line 217 "alder_fasta.c" */
static const char _fastaBuild_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 2, 
	0, 1
};

static const char _fastaBuild_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fastaBuild_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fastaBuild_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fastaBuild_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fastaBuild_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fastaBuild_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 7, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fastaBuild_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5, 2
};

static const char _fastaBuild_trans_actions[] = {
	0, 0, 7, 0, 1, 3, 0, 5
};

static const char _fastaBuild_eof_actions[] = {
	0, 0, 0, 0, 0, 5
};

static const int fastaBuild_start = 1;
static const int fastaBuild_first_final = 5;
static const int fastaBuild_error = 0;

static const int fastaBuild_en_main = 1;


/* #line 122 "alder_fasta.rl" */


#pragma mark volume


/* #line 282 "alder_fasta.c" */
static const char _fastaKmer_actions[] = {
	0, 1, 0, 1, 1, 1, 2
};

static const char _fastaKmer_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fastaKmer_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fastaKmer_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fastaKmer_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fastaKmer_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fastaKmer_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 7, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fastaKmer_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5, 2
};

static const char _fastaKmer_trans_actions[] = {
	0, 0, 1, 0, 0, 3, 0, 5
};

static const char _fastaKmer_eof_actions[] = {
	0, 0, 0, 0, 0, 5
};

static const int fastaKmer_start = 1;
static const int fastaKmer_first_final = 5;
static const int fastaKmer_error = 0;

static const int fastaKmer_en_main = 1;


/* #line 144 "alder_fasta.rl" */


/* This function counts kmers (size of kmer_size) in a fasta file.
 * returns
 * 0 on success
 * 1 if an error occurs.
 */
static int alder_fasta_count_kmer_file(const char *fn, int kmer_size, uint64_t *volume);
static int alder_fasta_count_kmer_gzip(const char *fn, int kmer_size, uint64_t *volume);

int alder_fasta_count_kmer(const char *fn, int kmer_size,
                           size_t *curBufsize, size_t totalBufsize,
                           uint64_t *volume)
{
    int s = alder_file_isgzip(fn);
    if (s) {
        alder_fasta_count_kmer_gzip(fn, kmer_size, volume);
    } else {
        alder_fasta_count_kmer_file(fn, kmer_size, volume);
    }
    return 0;
}

static int alder_fasta_count_kmer_file(const char *fn, int kmer_size, uint64_t *volume)
{
    
/* #line 168 "alder_fasta.rl" */
    int cs;
//    int have = 0;
    char buf[BUFSIZE];
    *volume = 0;
    uint64_t t1 = 0;
    
    FILE *fp = fopen(fn, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    
/* #line 378 "alder_fasta.c" */
	{
	cs = fastaKmer_start;
	}

/* #line 180 "alder_fasta.rl" */
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
        
/* #line 398 "alder_fasta.c" */
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
	_keys = _fastaKmer_trans_keys + _fastaKmer_key_offsets[cs];
	_trans = _fastaKmer_index_offsets[cs];

	_klen = _fastaKmer_single_lengths[cs];
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

	_klen = _fastaKmer_range_lengths[cs];
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
	_trans = _fastaKmer_indicies[_trans];
	cs = _fastaKmer_trans_targs[_trans];

	if ( _fastaKmer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastaKmer_actions + _fastaKmer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 128 "alder_fasta.rl" */
	{
        t1 = 0;
    }
	break;
	case 1:
/* #line 131 "alder_fasta.rl" */
	{
        t1++;
    }
	break;
	case 2:
/* #line 134 "alder_fasta.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 492 "alder_fasta.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _fastaKmer_actions + _fastaKmer_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 2:
/* #line 134 "alder_fasta.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 516 "alder_fasta.c" */
		}
	}
	}

	_out: {}
	}

/* #line 194 "alder_fasta.rl" */
    }
    fclose(fp);
    return 0;
}

#include <zlib.h>

static int alder_fasta_count_kmer_gzip(const char *fn, int kmer_size, uint64_t *volume)
{
    
/* #line 204 "alder_fasta.rl" */
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
    
/* #line 549 "alder_fasta.c" */
	{
	cs = fastaKmer_start;
	}

/* #line 217 "alder_fasta.rl" */
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
        
/* #line 572 "alder_fasta.c" */
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
	_keys = _fastaKmer_trans_keys + _fastaKmer_key_offsets[cs];
	_trans = _fastaKmer_index_offsets[cs];

	_klen = _fastaKmer_single_lengths[cs];
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

	_klen = _fastaKmer_range_lengths[cs];
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
	_trans = _fastaKmer_indicies[_trans];
	cs = _fastaKmer_trans_targs[_trans];

	if ( _fastaKmer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastaKmer_actions + _fastaKmer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 128 "alder_fasta.rl" */
	{
        t1 = 0;
    }
	break;
	case 1:
/* #line 131 "alder_fasta.rl" */
	{
        t1++;
    }
	break;
	case 2:
/* #line 134 "alder_fasta.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 666 "alder_fasta.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _fastaKmer_actions + _fastaKmer_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 2:
/* #line 134 "alder_fasta.rl" */
	{
        if (t1 >= kmer_size) {
            *volume += (t1 - kmer_size + 1);
        }
    }
	break;
/* #line 690 "alder_fasta.c" */
		}
	}
	}

	_out: {}
	}

/* #line 234 "alder_fasta.rl" */
    }
    gzclose(fp);
//    fclose(fp);
    return 0;
}


#pragma mark ragel

/* This function finds the number of sequences and the total length of them.
 */
int alder_fasta_length(const char* fn, int64_t *n1, int64_t *n2)
{
    
/* #line 248 "alder_fasta.rl" */
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    *n1 = 0;
    *n2 = 0;
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 0;
    }
    
/* #line 728 "alder_fasta.c" */
	{
	cs = fastaSize_start;
	}

/* #line 262 "alder_fasta.rl" */
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            exit(1);
        }
        len = (int)fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        
/* #line 751 "alder_fasta.c" */
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
	_keys = _fastaSize_trans_keys + _fastaSize_key_offsets[cs];
	_trans = _fastaSize_index_offsets[cs];

	_klen = _fastaSize_single_lengths[cs];
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

	_klen = _fastaSize_range_lengths[cs];
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
	_trans = _fastaSize_indicies[_trans];
	cs = _fastaSize_trans_targs[_trans];

	if ( _fastaSize_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastaSize_actions + _fastaSize_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 50 "alder_fasta.rl" */
	{
        (*n1)++;
    }
	break;
	case 1:
/* #line 53 "alder_fasta.rl" */
	{
    }
	break;
	case 2:
/* #line 55 "alder_fasta.rl" */
	{
        (*n2)++;
    }
	break;
/* #line 842 "alder_fasta.c" */
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

/* #line 279 "alder_fasta.rl" */
        /* How much is still in the buffer? */
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    
    return 0;
}

int alder_fasta_read(const char* fn, alder_fasta_t* pf)
{
    
/* #line 294 "alder_fasta.rl" */
    
    size_t n1 = 0;
    size_t n2 = 0;
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "cannot open %s\n", fn);
        return 1;
    }
    
/* #line 887 "alder_fasta.c" */
	{
	cs = fasta_start;
	}

/* #line 309 "alder_fasta.rl" */
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "cannot read %s\n", fn);
            return 1;
        }
        len = (int)fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        
/* #line 910 "alder_fasta.c" */
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
	_keys = _fasta_trans_keys + _fasta_key_offsets[cs];
	_trans = _fasta_index_offsets[cs];

	_klen = _fasta_single_lengths[cs];
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

	_klen = _fasta_range_lengths[cs];
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
	_trans = _fasta_indicies[_trans];
	cs = _fasta_trans_targs[_trans];

	if ( _fasta_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fasta_actions + _fasta_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 35 "alder_fasta.rl" */
	{
        pf->index[n1++] = n2;
    }
	break;
	case 1:
/* #line 38 "alder_fasta.rl" */
	{
        pf->data[n2++] = (char)(*p);
    }
	break;
/* #line 996 "alder_fasta.c" */
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

/* #line 326 "alder_fasta.rl" */
        /* How much is still in the buffer? */
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    return 0;
}

#pragma mark fasta_t

alder_fasta_t *alder_fasta_alloc(const char* fn)
{
    int status;
    int64_t ui1 = 0;
    int64_t ui2 = 0;
    alder_fasta_t *v = malloc(sizeof(*v));
    if (v == NULL) return NULL;
    memset(v,0,sizeof(*v));
    alder_fasta_length(fn, &ui1, &ui2);
    v->numberOfSequence = ui1;
    v->numberOfBase = ui2;
    v->sizeCapacity = ui2 + sSizeCapacity;
    v->index = malloc(ui1*sizeof(unsigned int));
    if (v->index == NULL) {
        free(v);
        return NULL;
    }
    memset(v->index,0,ui1*sizeof(unsigned int));
    v->data = malloc(v->sizeCapacity*sizeof(char));
    if (v->data == NULL) {
        free(v);
        free(v->index);
        return NULL;
    }
    memset(v->data,0,v->sizeCapacity*sizeof(char));
    status = alder_fasta_read(fn, v);
    if (status != 0) {
        free(v->data);
        free(v->index);
        free(v);
        return NULL;
    }
    return v;
}

void alder_fasta_free(alder_fasta_t* pf)
{
    if (pf->name != NULL) {
        bstrVectorDelete(pf->name);
    }
    free(pf->index);
    free(pf->data);
    free(pf);
}

/* This function find sequence names and their lengths.
 */
int alder_fasta_seqname_length(const struct bstrList* fn,
                               struct bstrList *name,
                               int64_t *length)
{
    
/* #line 388 "alder_fasta.rl" */
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    bstring bseqname;
    
    int64_t t1 = 0;
    int64_t t2 = 0;
    for (size_t i = 0; i < fn->qty; i++) {
        int64_t n1 = 0;
        int64_t n2 = 0;
        FILE *fin = fopen(bdata(fn->entry[i]), "r");
        if (fin == NULL)
        {
            fprintf(stderr, "cannot open %s\n", bdata(fn->entry[i]));
            return 1;
        }
        
/* #line 1091 "alder_fasta.c" */
	{
	cs = fastaSeqSize_start;
	}

/* #line 406 "alder_fasta.rl" */
        while ( 1 ) {
            char *eof, *p, *pe, *data = buf + have;
            eof = NULL;
            int len, space = BUFSIZE - have;
            if ( space == 0 ) {
                fprintf(stderr, "BUFFER OUT OF SPACE\n");
                exit(1);
            }
            len = (int)fread( data, 1, space, fin );
            if ( len == 0 )
                break;
            /* Find the last newline by searching backwards. */
            p = buf;
            pe = data + len - 1;
            while ( *pe != '\n' && pe >= buf )
                pe--;
            pe += 1;
            if (feof(fin)) {
                eof = pe;
            }
            
/* #line 1118 "alder_fasta.c" */
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
	_keys = _fastaSeqSize_trans_keys + _fastaSeqSize_key_offsets[cs];
	_trans = _fastaSeqSize_index_offsets[cs];

	_klen = _fastaSeqSize_single_lengths[cs];
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

	_klen = _fastaSeqSize_range_lengths[cs];
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
	_trans = _fastaSeqSize_indicies[_trans];
	cs = _fastaSeqSize_trans_targs[_trans];

	if ( _fastaSeqSize_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastaSeqSize_actions + _fastaSeqSize_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 67 "alder_fasta.rl" */
	{
        bstrVectorAdd(name, "");
        bseqname = name->entry[t1];
//        printf("e[%c]", fc);
        (n1)++;
        t1++;
        t2 = 0;
    }
	break;
	case 1:
/* #line 75 "alder_fasta.rl" */
	{
        bconchar(bseqname, (*p));
//        printf("t[%c]", fc);
    }
	break;
	case 2:
/* #line 79 "alder_fasta.rl" */
	{
        (n2)++;
        t2++;
    }
	break;
	case 3:
/* #line 83 "alder_fasta.rl" */
	{
        length[t1-1] = t2;
//        printf("f[%c]", fc);
    }
	break;
/* #line 1224 "alder_fasta.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _fastaSeqSize_actions + _fastaSeqSize_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
/* #line 83 "alder_fasta.rl" */
	{
        length[t1-1] = t2;
//        printf("f[%c]", fc);
    }
	break;
/* #line 1247 "alder_fasta.c" */
		}
	}
	}

	_out: {}
	}

/* #line 427 "alder_fasta.rl" */
            /* How much is still in the buffer? */
            have = (int)(data + len - pe);
            if ( have > 0 )
                memmove( buf, pe, have );
            if ( len < space )
                break;
        }
        fclose(fin);
        printf("n1 : n2 - %lld - %lld\n", n1, n2);
    }
    
    return 0;
}

alder_fasta_t *alder_fasta_list_alloc(const struct bstrList *fn,
                                      const int64_t tSeq,
                                      const int64_t tBase,
                                      const int64_t plusK)
{
    assert(tSeq > 0);
    assert(tBase > 0);
    
/* #line 449 "alder_fasta.rl" */
    
    alder_fasta_t *af = malloc(sizeof(alder_fasta_t));
    af->name = bstrVectorCreate((int)tSeq);
    af->index = malloc((size_t)tSeq * sizeof(int64_t));
    af->data = malloc((tBase+1+plusK)*sizeof(char));
    af->data[tBase] = '\0';
    af->numberOfSequence = tSeq;
    af->numberOfBase = tBase;
    af->sizeCapacity = tBase + 1 + plusK;
    af->sizeOfDataWithDollar = -1;
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    bstring bseqname;
    
    int64_t t1 = 0;
    int64_t t2 = 0;
    for (size_t i = 0; i < fn->qty; i++) {
        int64_t n1 = 0;
        int64_t n2 = 0;
        FILE *fin = fopen(bdata(fn->entry[i]), "r");
        if (fin == NULL)
        {
            fprintf(stderr, "cannot open %s\n", bdata(fn->entry[i]));
            alder_fasta_list_free(af);
            return NULL;
        }
        
/* #line 1308 "alder_fasta.c" */
	{
	cs = fastaBuild_start;
	}

/* #line 478 "alder_fasta.rl" */
        while ( 1 ) {
            char *eof, *p, *pe, *data = buf + have;
            eof = NULL;
            int len, space = BUFSIZE - have;
            if ( space == 0 ) {
                fprintf(stderr, "BUFFER OUT OF SPACE\n");
                exit(1);
            }
            len = (int)fread( data, 1, space, fin );
            if ( len == 0 )
                break;
            /* Find the last newline by searching backwards. */
            p = buf;
            pe = data + len - 1;
            while ( *pe != '\n' && pe >= buf )
                pe--;
            pe += 1;
            if (feof(fin)) {
                eof = pe;
            }
            
/* #line 1335 "alder_fasta.c" */
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
	_keys = _fastaBuild_trans_keys + _fastaBuild_key_offsets[cs];
	_trans = _fastaBuild_index_offsets[cs];

	_klen = _fastaBuild_single_lengths[cs];
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

	_klen = _fastaBuild_range_lengths[cs];
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
	_trans = _fastaBuild_indicies[_trans];
	cs = _fastaBuild_trans_targs[_trans];

	if ( _fastaBuild_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastaBuild_actions + _fastaBuild_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 96 "alder_fasta.rl" */
	{
        bstrVectorAdd(af->name, "");
        bseqname = af->name->entry[t1];
//        printf("e[%c]", fc);
        (n1)++;
        t1++;
    }
	break;
	case 1:
/* #line 103 "alder_fasta.rl" */
	{
        bconchar(bseqname, (*p));
//        printf("t[%c]", fc);
    }
	break;
	case 2:
/* #line 107 "alder_fasta.rl" */
	{
//        printf("%c", fc);
        af->data[t2] = (*p);
        (n2)++;
        t2++;
    }
	break;
	case 3:
/* #line 113 "alder_fasta.rl" */
	{
        af->index[t1-1] = t2;
//        printf("f[%c]", fc);
    }
	break;
/* #line 1442 "alder_fasta.c" */
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _fastaBuild_actions + _fastaBuild_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
/* #line 113 "alder_fasta.rl" */
	{
        af->index[t1-1] = t2;
//        printf("f[%c]", fc);
    }
	break;
/* #line 1465 "alder_fasta.c" */
		}
	}
	}

	_out: {}
	}

/* #line 499 "alder_fasta.rl" */
            /* How much is still in the buffer? */
            have = (int)(data + len - pe);
            if ( have > 0 )
                memmove( buf, pe, have );
            if ( len < space )
                break;
        }
        fclose(fin);
    }
    
    return af;
    
}

void alder_fasta_list_free(alder_fasta_t *pf)
{
    if (pf->name != NULL) bstrVectorDelete(pf->name);
    if (pf->index != NULL) free(pf->index);
    if (pf->data != NULL) free(pf->data);
    if (pf != NULL) free(pf);
}

int alder_fasta_list_length(const struct bstrList *fs, int64_t *n1, int64_t *n2)
{
    int64_t tSeq = 0;
    int64_t tBase = 0;
    for (size_t i = 0; i < fs->qty; i++) {
        int64_t nSeq = 0;
        int64_t nBase = 0;
        alder_fasta_length(bdata(fs->entry[i]), &nSeq, &nBase);
        tSeq += nSeq;
        tBase += nBase;
    }
    *n1 = tSeq;
    *n2 = tBase;
    return 0;
}
//int alder_fasta_list_read(const struct bstrList *fn, alder_fasta_t* pf);


/**
 * This function returns a (not owned) bstring pointer using a position in 
 * the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * a bstring pointer if successful
 * NULL otherwise.
 */
char * alder_fasta_name_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    return bstr2cstr(af->name->entry[i], '\0');
}

/**
 * This function returns the index of reference names using a position in
 * the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * the index of the reference name if successful
 * -1 otherwise.
 */
int64_t alder_fasta_index_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    return i;
}

/**
 * This function returns the position in the reference name using a position
 * in the whole sequence of a suffix array.
 * Argument:
 * af - alder_fasta_t
 * position - position in a suffix array
 * Return:
 * the position in the reference name if successful
 * -1 otherwise.
 */
int64_t alder_fasta_position_by_saindex(const alder_fasta_t *af, int64_t position)
{
    int64_t i = 0;
    for (i = 0; i < af->numberOfSequence; i++) {
        if (position < af->index[i]) {
            break;
        }
    }
    if (i > 0) {
        position -= af->index[i-1];
    }
    assert(position >= 0);
    return position;
}




