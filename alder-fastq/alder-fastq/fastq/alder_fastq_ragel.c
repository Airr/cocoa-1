
/* #line 1 "alder_fastq_ragel.rl" */
/**
 * This file, alder_fastq_ragel.c, is part of alder-fastq.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alder_fastq_ragel.h"


/* #line 29 "alder_fastq_ragel.c" */
static const char _fastqBuild_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 2, 
	0, 1
};

static const char _fastqBuild_key_offsets[] = {
	0, 0, 1, 3, 8, 31
};

static const char _fastqBuild_trans_keys[] = {
	62, 32, 126, 127, 0, 31, 32, 126, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 62, 
	75, 89, 107, 121, 127, 0, 31, 65, 
	68, 71, 72, 77, 78, 82, 87, 97, 
	100, 103, 104, 109, 110, 114, 119, 0
};

static const char _fastqBuild_single_lengths[] = {
	0, 1, 0, 1, 5, 6
};

static const char _fastqBuild_range_lengths[] = {
	0, 0, 1, 2, 9, 9
};

static const char _fastqBuild_index_offsets[] = {
	0, 0, 2, 4, 8, 23
};

static const char _fastqBuild_indicies[] = {
	0, 1, 2, 1, 3, 3, 4, 1, 
	5, 5, 5, 5, 3, 3, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 7, 
	5, 5, 5, 5, 6, 6, 5, 5, 
	5, 5, 5, 5, 5, 5, 1, 0
};

static const char _fastqBuild_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 5, 2
};

static const char _fastqBuild_trans_actions[] = {
	0, 0, 7, 0, 1, 3, 0, 5
};

static const char _fastqBuild_eof_actions[] = {
	0, 0, 0, 0, 0, 5
};

static const int fastqBuild_start = 1;
static const int fastqBuild_first_final = 5;
static const int fastqBuild_error = 0;

static const int fastqBuild_en_main = 1;


/* #line 55 "alder_fastq_ragel.rl" */


rseq_t * alder_rseq_init (const char *fn, int type)
{
//    %% machine fastqBuild;
    
    FILE *fin = fopen(fn, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "cannot open %s\n", fn);
//        alder_fasta_list_free(af);
        return NULL;
    }
    
    int cs;
    
/* #line 105 "alder_fastq_ragel.c" */
	{
	cs = fastqBuild_start;
	}

/* #line 71 "alder_fastq_ragel.rl" */
    
    rseq_t *seq = malloc(sizeof(rseq_t));
    seq->f = malloc(sizeof(rstream_t));
    seq->f->have = 0;
    seq->f->f = fin;
    seq->f->cs = cs;
    
    return seq;
}

void alder_rseq_destroy (rseq_t * ks)
{
    fclose(ks->f->f);
    free(ks->f);
    free(ks);
}

int alder_rseq_read (rseq_t * seq)
{
//    %% machine fastqBuild;
    
    FILE *fin = (FILE*)seq->f->f;
    char *buf = seq->f->buf;
    int have = seq->f->have;
    int cs = seq->f->cs;
    seq->f->stop = 0;
    int n1 = 0;
    int n2 = 0;
    
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
        
/* #line 161 "alder_fastq_ragel.c" */
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
	_keys = _fastqBuild_trans_keys + _fastqBuild_key_offsets[cs];
	_trans = _fastqBuild_index_offsets[cs];

	_klen = _fastqBuild_single_lengths[cs];
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

	_klen = _fastqBuild_range_lengths[cs];
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
	_trans = _fastqBuild_indicies[_trans];
	cs = _fastqBuild_trans_targs[_trans];

	if ( _fastqBuild_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _fastqBuild_actions + _fastqBuild_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 27 "alder_fastq_ragel.rl" */
	{
//        bstrVectorAdd(af->name, "");
//        bseqname = af->name->entry[t1];
        printf("e[%c]", (*p));
        (n1)++;
//        t1++;
    }
	break;
	case 1:
/* #line 34 "alder_fastq_ragel.rl" */
	{
//        bconchar(bseqname, fc);
        printf("t[%c]", (*p));
    }
	break;
	case 2:
/* #line 38 "alder_fastq_ragel.rl" */
	{
        printf("%c", (*p));
//        af->data[t2] = fc;
        (n2)++;
//        t2++;
    }
	break;
	case 3:
/* #line 44 "alder_fastq_ragel.rl" */
	{
//        af->index[t1-1] = t2;
        printf("f[%c]", (*p));
        seq->f->stop = 1;
        goto _out;
    }
	break;
/* #line 270 "alder_fastq_ragel.c" */
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
	const char *__acts = _fastqBuild_actions + _fastqBuild_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
/* #line 44 "alder_fastq_ragel.rl" */
	{
//        af->index[t1-1] = t2;
        printf("f[%c]", (*p));
        seq->f->stop = 1;
        goto _out;
    }
	break;
/* #line 295 "alder_fastq_ragel.c" */
		}
	}
	}

	_out: {}
	}

/* #line 121 "alder_fastq_ragel.rl" */
        
        /* How much is still in the buffer? */
        have = (int)(data + len - pe);
        if ( have > 0 )
            memmove( buf, pe, have );
        
        if (seq->f->stop == 1) {
            seq->f->cs = cs;
            seq->f->have = have;
            break;
        }
        
        if ( len < space )
            break;
    }
    printf("n1/n2: %d/%d\n", n1, n2);

    return 0;
}




