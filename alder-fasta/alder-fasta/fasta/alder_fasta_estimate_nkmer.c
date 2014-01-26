
#line 1 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
/**
 * This file, alder_fasta_estimate_nkmer.c, is part of alder-fasta.
 *
 * Copyright 2014 by Sang Chul Choi
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

#include <assert.h>
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "bzlib.h"
#include "alder_file.h"
#include "alder_fasta_estimate_nkmer.h"


#line 33 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.c"
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


#line 47 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"


/**
 *  This funciton counts the number of DNA letters to compute the maximum
 *  number of Kmers in the file.
 *
 *  @param fn                   file name
 *  @param kmer_size            kmer size
 *  @param curBufsize           for displaying progress
 *  @param totalBufsize         for displaying progress
 *  @param volume               number of kmer
 *  @param progress_flag        progress
 *  @param progressToError_flag progress to std err
 *
 *  @return SUCCESS or FAIL
 */
int alder_fasta_estimate_nkmer(uint64_t *v,
                               int kmer_size,
                               const char *fn)
{
    int s = ALDER_STATUS_SUCCESS;
    
#line 69 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
    int cs;
    
    size_t curBufsize = 0;
    size_t totalBufsize = ALDER_BUFSIZE_8MB;
    unsigned int used_byte_bzip2 = 0;
    
    int isGzip = alder_file_isgzip(fn);
    int isBzip = alder_file_isbzip2(fn);
    *v = 0;
    uint64_t t1 = 0;
    
    gzFile fpgz = Z_NULL;
    BZFILE *fpbz = NULL;
    bz_stream strm;
    FILE *fp = NULL;
    
    if (isGzip == ALDER_YES) {
        fpgz = gzopen(fn, "r");
    } else if (isBzip == ALDER_YES) {
        fp = fopen(fn, "r");
        int verbosity = 0;
        int small = 0;
        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;
        s = BZ2_bzDecompressInit(&strm, verbosity, small);
        if (s != BZ_OK) {
            fprintf(stderr, "Error: failed to init bz2 %s\n", fn);
            return 1;
        }
    } else {
        fp = fopen(fn, "r");
    }
    if (fpgz == Z_NULL && fpbz == NULL && fp == NULL) {
        fprintf(stderr, "Error: could not open %s\n", fn);
        return 1;
    }
    char *buf = malloc(sizeof(*buf) * ALDER_BUFSIZE_8MB);
    memset(buf,0,sizeof(*buf) * ALDER_BUFSIZE_8MB);
    char *ibuf = malloc(sizeof(*ibuf) * ALDER_BUFSIZE_8MB);
    memset(ibuf,0,sizeof(*ibuf) * ALDER_BUFSIZE_8MB);
    
    
#line 158 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.c"
	{
	cs = fastaKmer_start;
	}

#line 112 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
    while (1) {
        char *p;
        char *pe;
        char *eof;
        int len;
        
        if (totalBufsize <= curBufsize) {
            break;
        }
        if (isGzip == ALDER_YES) {
            len = gzread(fpgz, buf, ALDER_BUFSIZE_8MB);
        } else if (isBzip == ALDER_YES) {
            len = (int)fread(ibuf, sizeof(char), ALDER_BUFSIZE_8MB, fp);
            
            strm.next_in = (char *)ibuf;
            strm.avail_in = (unsigned int)len;
            
            
            strm.next_out = (char*)buf;
            strm.avail_out = (unsigned int)ALDER_BUFSIZE_8MB;
            s = BZ2_bzDecompress(&strm);
            if (BZ_OK == s || BZ_STREAM_END == s) {
                //
                used_byte_bzip2 = (unsigned int)len - strm.avail_in;
                len = (int)(strm.next_out - (char*)buf);
            } else {
                // Error!
                len = 0;
            }
        } else {
            len = (int)fread(buf, sizeof(char), ALDER_BUFSIZE_8MB, fp);
        }
        if (len <= 0) {
            break;
        }
        curBufsize += len;
        
        p = buf;
        pe = buf + len;
        if (len < ALDER_BUFSIZE_8MB) {
            eof = pe;
        }
        
#line 207 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.c"
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
#line 31 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
	{
        t1 = 0;
    }
	break;
	case 1:
#line 34 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
	{
        t1++;
    }
	break;
	case 2:
#line 37 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
	{
        if (t1 >= kmer_size) {
            *v += (t1 - kmer_size + 1);
        }
    }
	break;
#line 301 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.c"
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
#line 37 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
	{
        if (t1 >= kmer_size) {
            *v += (t1 - kmer_size + 1);
        }
    }
	break;
#line 325 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.c"
		}
	}
	}

	_out: {}
	}

#line 155 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fasta/alder-fasta/fasta/alder_fasta_estimate_nkmer.rl"
    }
    
    /* Estimate the number of K-mers using the count. */
    size_t filesize = 0;
    alder_file_size(fn, &filesize);
    if (isGzip == ALDER_YES) {
        if (!gzeof(fpgz)) {
            z_off_t cur = gzoffset(fpgz);
            if (cur != -1) {
                *v = (uint64_t)(*v * ((double)filesize / (double)cur));
            } else {
                *v = 0;
                s = ALDER_STATUS_ERROR;
            }
        }
    } else if (isBzip == ALDER_YES) {
        if (ALDER_BUFSIZE_8MB < filesize || strm.avail_in > 0) {
            *v = (uint64_t)(*v * ((double)filesize / (double)(used_byte_bzip2)));
            
        }
    } else {
        if (ALDER_BUFSIZE_8MB < filesize) {
            *v = (uint64_t)(*v * ((double)filesize / (double)curBufsize));
        }
    }
    
    if (isGzip == ALDER_YES) {
        gzclose(fpgz);
        fpgz = Z_NULL;
    } else if (isBzip == ALDER_YES) {
        XFCLOSE(fp);
    } else {
        XFCLOSE(fp);
    }
    free(buf);
    free(ibuf);
    return s;
}