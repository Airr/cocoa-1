
#line 1 "alder_fasta.rl"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <gsl/gsl_errno.h>

#include "alder_fasta.h"
#include "alder-align/rng/rvgs.h"

#define BUFSIZE 10000
enum { GFFELEMENTSIZE = 100 };

fasta_t *alder_fasta_alloc(const char* fastaFilename)
{
    int status;
    uint64_t ui1 = 0;
    uint64_t ui2 = 0;
    fasta_t *v = malloc(sizeof(fasta_t));
    if (v == NULL) return NULL;
    alder_fasta_length(fastaFilename, &ui1, &ui2);
    v.numberOfSequence = ui1;
    v.numberOfBase = ui2;
    v.index = malloc(ui1*sizeof(unsigned int));
    if (v.index == NULL) {
        free(v);
        return NULL;
    }
    v.data = malloc(ui2*sizeof(char));
    if (v.data == NULL) {
        free(v);
        free(v.index);
        return NULL;
    }
    status = alder_fasta_read(fastaFilename, v);
    if (status != GSL_SUCCESS) {
        free(v.data);
        free(v.index);
        free(v);
        return NULL;
    }
    return v;
}

void alder_fasta_free(fasta_t* pFastaDataStructure)
{
    free(pFastaDataStructure->index);
    free(pFastaDataStructure->data);
    free(pFastaDataStructure);
}

int alder_fasta_read(const char* fastaFilename, fasta_t* pFastaDataStructure)
{
    
#line 54 "alder_fasta.rl"
    
#line 59 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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


#line 55 "alder_fasta.rl"
    
    size_t n1 = 0;
    uint64_t n2 = 0;
    int cs;
    int have = 0;
    char buf[BUFSIZE];

    
    FILE *fin = fopen(fastaFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fastaFilename);
        GSL_ERROR ("failed to allocate space for vector struct", GSL_ENOMEM);
    }
    
#line 129 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
	{
	cs = fasta_start;
	}

#line 70 "alder_fasta.rl"
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            GSL_ERROR ("failed to allocate space for vector struct", GSL_ENOMEM);
        }
        len = fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        
#line 152 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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
#line 3 "alder_fasta.rh"
	{
        pFastaDataStructure->index[n1++] = n2;
    }
	break;
	case 1:
#line 6 "alder_fasta.rh"
	{
        pFastaDataStructure->data[n2++] = (*p);
    }
	break;
#line 238 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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

#line 87 "alder_fasta.rl"
        /* How much is still in the buffer? */
        have = data + len - pe;
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    return GSL_SUCCESS;
}

int alder_fasta_length(const char* fastaFilename, uint64_t *n1, uint64_t *n2)
{
    
#line 101 "alder_fasta.rl"
    
#line 268 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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


#line 102 "alder_fasta.rl"
    
    int cs;
    int have = 0;
    char buf[BUFSIZE];
    *n1 = 0;
    *n2 = 0;
    
    FILE *fin = fopen(fastaFilename, "r");
    if (fin == NULL)
    {
        fprintf(stderr, "Error: could not open %s\n", fastaFilename);
        return 0;
    }
    
#line 338 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
	{
	cs = fastaSize_start;
	}

#line 116 "alder_fasta.rl"
    while ( 1 ) {
        char *p, *pe, *data = buf + have;
        int len, space = BUFSIZE - have;
        if ( space == 0 ) {
            fprintf(stderr, "BUFFER OUT OF SPACE\n");
            exit(1);
        }
        len = fread( data, 1, space, fin );
        if ( len == 0 )
            break;
        /* Find the last newline by searching backwards. */
        p = buf;
        pe = data + len - 1;
        while ( *pe != '\n' && pe >= buf )
            pe--;
        pe += 1;
        
#line 361 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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
#line 18 "alder_fasta.rh"
	{
        (*n1)++;
    }
	break;
	case 1:
#line 21 "alder_fasta.rh"
	{
    }
	break;
	case 2:
#line 23 "alder_fasta.rh"
	{
        (*n2)++;
    }
	break;
#line 452 "/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-align/build/alder-align.build/Release/alder-test-mksa.build/DerivedSources/alder_fasta.c"
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

#line 133 "alder_fasta.rl"
        /* How much is still in the buffer? */
        have = data + len - pe;
        if ( have > 0 )
            memmove( buf, pe, have );
        if ( len < space )
            break;
    }
    
    
    return 0;
}

int randomFasta(unsigned int n1, unsigned int n2)
{
    enum { LENSEQUENCEPERLINE = 60 };
    char sequencePerLine[LENSEQUENCEPERLINE + 1];
    char acgt[4] = { 'A', 'C', 'G', 'T' };
    unsigned int i, j, k;
    long l;
    
    sequencePerLine[LENSEQUENCEPERLINE] = '\0';
    for (i = 0; i < n1; i++) {
        fprintf(stdout, ">%u\n", i);
        for (j = 0; j < n2; j++)
        {
            for (k = 0; k < LENSEQUENCEPERLINE; k++) {
                l = Equilikely(0,3);
                sequencePerLine[k] = acgt[l];
            }
            fprintf(stdout, "%s\n", sequencePerLine);
        }
    }
    
    return 0;
}