
#line 1 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
/**
 * This file, alder_fastq_estimate_nkmer.c, is part of alder-fastq.
 *
 * Copyright 2014 by Sang Chul Choi
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
 * along with alder-fastq.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "alder_cmacro.h"
#include "bzlib.h"
#include "alder_file.h"
#include "alder_fastq_estimate_nkmer.h"

/* See https://github.com/lomereiter/bioragel/blob/master/examples/d/fastq.rl
 */

#line 35 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.c"
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


#line 58 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"



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
int alder_fastq_estimate_nkmer(uint64_t *v,
                               int kmer_size,
                               const char *fn)
{
    int s = ALDER_STATUS_SUCCESS;
    
#line 81 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
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
//        fpbz = BZ2_bzopen(fn, "r");
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
    
    
#line 157 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.c"
	{
	cs = fastqKmer_start;
	}

#line 125 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
    while ( 1 ) {
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
//            len = BZ2_bzread(fpbz, buf, ALDER_BUFSIZE_8MB);
            
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
        
#line 207 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.c"
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
#line 33 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
	{
        t1 = 0;
    }
	break;
	case 1:
#line 36 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
	{
        t1++;
    }
	break;
	case 2:
#line 39 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
	{
        if (t1 >= kmer_size) {
            *v += (t1 - kmer_size + 1);
        }
    }
	break;
#line 300 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.c"
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

#line 169 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-fastq/alder-fastq/fastq/alder_fastq_estimate_nkmer.rl"
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
//        BZ2_bzclose(fpbz);
//        fpbz = NULL;
        XFCLOSE(fp);
    } else {
        XFCLOSE(fp);
    }
    free(buf);
    free(ibuf);
    return s;
}
