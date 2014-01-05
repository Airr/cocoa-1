/**
 * This file, alder_wordtable_intblocklist.c, is part of alder-wordtable.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-wordtable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-wordtable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-wordtable.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file provides functions for creating a list of edges, which is used
 * to make a KFMindex. 
 * A block contains an edge in KFMindex. The blockarray in intblocklist is a
 * 2-dimensional array: i.e., an array of arrays. The element of the 
 * 2-dimensional array is called a subarray. A subarray in intblocklist
 * contains blocks or edges. A block consists of a node sequence and in-coming
 * edges in letters. It also contains edge types; normal or final-completing
 * edges. Rodland13 documents this block; let me copy them here. The chapter 4
 * in the document details how this intblocklist is implemented.
 * --- Rodland13doc ---
 * Each edge is stored with three pieces of information: the l-string of the
 * node (using the 0 token for $), the position of $ in or relative to the
 * l-string (0 for the first position, l  1 for the last position, l for
 * immediately after the l-string, 1 otherwise or as a default), data
 * specifying the in-edge (or no in-edge).
 * --- Rodland13doc ---
 * Each block looks like this in bitwise format:
 * ACGT$$$$...[value p][value a].
 * The first is a node sequence, and p denotes the position of the first $
 * token. The value a means the in-comming edge. Any other values are
 * irrelevant; they are reset to zeros.
 * Each letter needs 2 bits, the value a requires 3 bits. The value p would be
 * 0 to l-1. Sometimes, the value p marks final nodes using k=l+1, no $ tokens
 * using all bits set to 1. The special marking k for final nodes are used to
 * help generate the final completing nodes. In the end, values of p would take
 * 0 to l-1, or all set to 1.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alder_cmacro.h"
#include "alder_wordtable_compute.h"
#include "alder_wordtable_kfmdefaults.h"
#include "alder_wordtable_bits.h"
#include "alder_wordtable_intblocklist.h"

static void
alder_wordtable_intblocklist_copyBlock(alder_wordtable_intblocklist_t *o,
                                       alder_wordtable_t *src,
                                       size_t srcPos,
                                       alder_wordtable_t *dest,
                                       size_t destPos);

static size_t
alder_wordtable_intblocklist_partition(alder_wordtable_intblocklist_t *o,
                                       size_t l, size_t r);

static size_t
alder_wordtable_intblocklist_resizeSize(alder_wordtable_intblocklist_t *o);

static void
alder_wordtable_intblocklist_swapIntervals(alder_wordtable_intblocklist_t *o,
                                           size_t p,
                                           size_t q,
                                           size_t len);

static void
alder_wordtable_intblocklist_swap(alder_wordtable_intblocklist_t *o,
                                  size_t p,
                                  size_t q);

static int
alder_wordtable_intblocklist_compare(alder_wordtable_intblocklist_t *o,
                                     size_t p,
                                     size_t q);

static size_t
alder_wordtable_intblocklist_arrayindex(alder_wordtable_intblocklist_t *o,
                                        size_t pos);

static size_t
alder_wordtable_intblocklist_subarraypos(alder_wordtable_intblocklist_t *o,
                                         size_t pos);

static size_t
alder_wordtable_intblocklist_subindex(alder_wordtable_intblocklist_t *o,
                                      size_t pos,
                                      size_t subpos);

static void
alder_wordtable_intblocklist_init(alder_wordtable_intblocklist_t *o,
                                  size_t blocksize);

struct alder_wordtable_intblocklist_struct {
    size_t blocksize;                 /* number of words (int) for a block */
    size_t subarraysize;              /* number of block in a subarray     */
    size_t size;                      /* number of blocks                  */
    size_t maxsize;                   /* maximum capacity in blocks        */
    size_t blockarraySize1;           /* number of subarrays               */
    size_t blockarraySize2;           /* subarraysize x blocksize          */
    alder_wordtable_t **blockarray;
};

/* This function initializes variables in intblocklist. 
 * blocksize is the size of a block, which is determined by the order of a 
 * de Bruijn subgraph. A block is a unit in intblocklist.
 */
void
alder_wordtable_intblocklist_init(alder_wordtable_intblocklist_t *o,
                                  size_t blocksize)
{
    o->blocksize = blocksize;
    o->subarraysize = 1 << ALDER_WORDTABLE_SUBARRAYSIZE;
    o->size = 0;
    o->maxsize = 0;
    o->blockarraySize1 = ALDER_WORDTABLE_BLOCKARRAY_SIZE;
    o->blockarraySize2 = o->subarraysize * blocksize;
    o->blockarray = NULL;
}

/* The function creates an intblocklist, which would be used by inedgelist. 
 * An initial intblocklist is created for a list of blocks. Each block occupies
 * blocksize x size of word (i.e., 4 or 8 bytes) bytes. If a unit of word is 4
 * byte word or int, and the block size is 1, then a block needs 4 bytes. The
 * maximum allowed memory would be managed by inedgelist.
 */
alder_wordtable_intblocklist_t*
alder_wordtable_intblocklist_create(size_t blocksize)
{
    alder_wordtable_intblocklist_t *o = malloc(sizeof(*o));
    ALDER_RETURN_NULL_IF_NULL(o);
    alder_wordtable_intblocklist_init(o, blocksize);
    /* memory */
    o->blockarray = malloc(sizeof(*o->blockarray) * o->blockarraySize1);
    if (o->blockarray == NULL) {
        alder_wordtable_intblocklist_destroy(o);
        return NULL;
    }
    memset(o->blockarray, 0, sizeof(*o->blockarray) * o->blockarraySize1);
    for (size_t i = 0; i < o->blockarraySize1; i++) {
        o->blockarray[i] = malloc(sizeof(*o->blockarray[i]) *
                                  o->blockarraySize2);
        if (o->blockarray[i] == NULL) {
            alder_wordtable_intblocklist_destroy(o);
            return NULL;
        }
    }
    o->maxsize = o->blockarraySize1 * o->subarraysize;
    return o;
}

/* This destroys an intblocklist.
 * blockarray is the only variable that needs attention.
 */
void alder_wordtable_intblocklist_destroy(alder_wordtable_intblocklist_t *o)
{
    if (o != NULL) {
        if (o->blockarray != NULL) {
            for (size_t i = 0; i < o->blockarraySize1; i++) {
                if (o->blockarray[i] != NULL) {
                    XFREE(o->blockarray[i]);
                }
            }
            XFREE(o->blockarray);
        }
        XFREE(o);
    }
}

void
alder_wordtable_intblocklist_empty(alder_wordtable_intblocklist_t *o)
{
    o->size = 0;
}


/* This function returns the number of blocks.
 */
size_t alder_wordtable_intblocklist_size(alder_wordtable_intblocklist_t *o)
{
    return o->size;
}

/* This function copies a block of intblocklist at pos to dest.
 */
void
alder_wordtable_intblocklist_getBlock(alder_wordtable_intblocklist_t *o,
                                      alder_wordtable_t *dest,
                                      size_t pos)
{
    assert(pos < o->maxsize);
    size_t arrayindexpos = alder_wordtable_intblocklist_arrayindex(o,pos);
    size_t subarraypospos = alder_wordtable_intblocklist_subarraypos(o,pos);
    alder_wordtable_intblocklist_copyBlock(o,
                                           *(o->blockarray + arrayindexpos),
                                           subarraypospos,
                                           dest,
                                           0);
}

/* This function sends src to the block at location pos.
 */
void
alder_wordtable_intblocklist_setBlock(alder_wordtable_intblocklist_t *o,
                                      alder_wordtable_t *src,
                                      size_t pos)
{
    size_t arrayindexpos = alder_wordtable_intblocklist_arrayindex(o,pos);
    size_t subarraypospos = alder_wordtable_intblocklist_subarraypos(o,pos);
    alder_wordtable_intblocklist_copyBlock(o,
                                           src,
                                           0,
                                           *(o->blockarray + arrayindexpos),
                                           subarraypospos);
    
}

/* This function increases the size to add a blank block. 
 * returns
 * a next position for a new block on success
 * ALDER_WORDTABLE_MAX_SIZE_T if something goes wrong or not enough memory.
 *
 * This is the only function where the size is increased. When a new block is
 * added, this function is called to obtain a space for a block and use other
 * functions to access or modifiy the block. Let's see how the memory is 
 * increases in size. Because maxsize is the the number of empty blocks, I 
 * return a next position unless size reaches maxsize.  If size reached
 * maxsize, I need to increase the memory allocated for blockarray.
 */
size_t
alder_wordtable_intblocklist_addBlankBlock(alder_wordtable_intblocklist_t *o)
{
    
    if (o->size == o->maxsize) {
        assert(o->size % o->subarraysize == 0);
        size_t arrayindex = o->size / o->subarraysize;
        assert(arrayindex == o->blockarraySize1);
//        size_t newSize1 = o->blockarraySize1 * 3 / 2 + 1;
        size_t newSize1 = o->blockarraySize1 + 1;
        alder_wordtable_t **t = realloc(o->blockarray,
                                        sizeof(*o->blockarray) * newSize1);
        ALDER_RETURN_ERROR_IF_NULL(t, ALDER_WORDTABLE_MAX_SIZE_T);
        o->blockarray = t;
        o->blockarraySize1 = newSize1;
        
        o->blockarray[arrayindex] = malloc(sizeof(*o->blockarray[0]) *
                                           o->blockarraySize2);
        ALDER_RETURN_ERROR_IF_NULL(o->blockarray[arrayindex],
                                   ALDER_WORDTABLE_MAX_SIZE_T);
        o->maxsize += o->subarraysize;
    }
    return o->size++;
}

/* This function copies p-th block to q-th block.
 */
void
alder_wordtable_intblocklist_copy(alder_wordtable_intblocklist_t *o,
                                  size_t p,
                                  size_t q)
{
    assert(p < o->maxsize);
    assert(q < o->maxsize);
    size_t px = p / o->subarraysize;
    size_t py = p % o->subarraysize;
    size_t qx = q / o->subarraysize;
    size_t qy = q % o->subarraysize;
    memcpy(o->blockarray[qx] + qy * o->blocksize,
           o->blockarray[px] + py * o->blocksize,
           sizeof(*o->blockarray[qx]) * o->blocksize);
}

/* This function gets a value of type wordtable_t (could be a 32-bit or 64-bit
 * a value) in a block at pos. The position in a block is given by subpos. If
 * the block size is 1, then subpos must be always 0. The function name is a
 * misnomer because a word could be a int type or other types. In this case,
 * I would use uint32_t, but could use uint64_t.
 *
 * x is the position of the subarray that has the block.
 * y is the position of the word or int in the block.
 */
alder_wordtable_t
alder_wordtable_intblocklist_getIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos)
{
    assert(subpos < o->blocksize);
    size_t x = alder_wordtable_intblocklist_arrayindex(o, pos);
    size_t y = alder_wordtable_intblocklist_subindex(o,pos,subpos);
//    size_t x = pos / o->subarraysize;
//    size_t y = (pos % o->subarraysize) * o->blocksize + subpos;
    return o->blockarray[x][y];
}

/* This function sets a value at location of pos/subpos.
 */
void
alder_wordtable_intblocklist_setIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos,
                                      alder_wordtable_t value)
{
    size_t x = pos / o->subarraysize;
    size_t y = (pos % o->subarraysize) * o->blocksize + subpos;
    o->blockarray[x][y] = value;
}

/* This function bitwise-ORs the value with the value at location of pos/subpos.
 */
void
alder_wordtable_intblocklist_orIntAt(alder_wordtable_intblocklist_t *o,
                                     size_t pos, size_t subpos,
                                     alder_wordtable_t value)
{
    size_t x = pos / o->subarraysize;
    size_t y = (pos % o->subarraysize) * o->blocksize + subpos;
    o->blockarray[x][y] |= value;
}

/* This function bitwise-ANDs the value with the value at location of pos/subpos.
 */
void
alder_wordtable_intblocklist_andIntAt(alder_wordtable_intblocklist_t *o,
                                      size_t pos, size_t subpos,
                                      alder_wordtable_t value)
{
    size_t x = pos / o->subarraysize;
    size_t y = (pos % o->subarraysize) * o->blocksize + subpos;
    o->blockarray[x][y] &= value;
}

/* This function frees the memory leaving the size-many elements intact.
 * e.g.,
 * size = 0; all of the subarrays are freed.
 * size = 1; the first subarray is left intact, and the rest of them are freed.
 * size = subarraysize; the same for the case of size = 1.
 * size = subarraysize + 1; the first two subarrays are left intact.
 * now, you see what this function does.
 * It frees memory of subarrays in the back of the blockarray.
 */
void
alder_wordtable_intblocklist_compact(alder_wordtable_intblocklist_t *o)
{
    size_t x = alder_wordtable_compute_groupmaxsize(o->size, o->subarraysize);
    for (size_t i = x; i < o->blockarraySize1; i++) {
        if (o->blockarray[i] == NULL) {
            return;
        }
        XFREE(o->blockarray[i]);
    }
}

/* This function returns the number of bits (from top) that are identical.
 * This function is not what the name implies. 
 * This only work
 * FIXME: this function needs to be checked. Something is ambiguous.
 */
int
alder_wordtable_intblocklist_nBitsEqual(alder_wordtable_intblocklist_t *o,
                                        size_t p,
                                        size_t q)
{
    int equal = 0;
    for (size_t i = 0; i < o->blocksize; i++) {
        equal += (int)(sizeof(alder_wordtable_t) * 8);
        alder_wordtable_t x = alder_wordtable_intblocklist_getIntAt(o, p, i);
        alder_wordtable_t y = alder_wordtable_intblocklist_getIntAt(o, q, i);
        alder_wordtable_t diff = x ^ y;
        if (diff != 0) {
            return equal - (int)alder_wordtable_bits_bitSizeOfWord(diff);
        }
    }
    return equal;
}

/* This function copies src's srcPos to dest's destPos by blocksize.
 */
inline void
alder_wordtable_intblocklist_copyBlock(alder_wordtable_intblocklist_t *o,
                                       alder_wordtable_t *src,
                                       size_t srcPos,
                                       alder_wordtable_t *dest,
                                       size_t destPos)
{
    memcpy(dest + destPos * o->blocksize,
           src + srcPos * o->blocksize,
           sizeof(*dest) * o->blocksize);
}

/* This function shifts block leftshift bits to the left, insert value, 
 * and return segment shifted out.
 * A block at pos is chosen to insert a value from right to the left 
 * by leftshift bits.
 */
int
alder_wordtable_intblocklist_shiftLeft(alder_wordtable_intblocklist_t *o,
                                       size_t pos,
                                       int leftshift,int insert)
{
    size_t arrayindexpos = alder_wordtable_intblocklist_arrayindex(o,pos);
    size_t subarraypospos = alder_wordtable_intblocklist_subarraypos(o,pos);
    return alder_wordtable_bits_shiftLeft2(*(o->blockarray + arrayindexpos),
                                           subarraypospos * o->blocksize,
                                           o->blocksize,
                                           leftshift, insert);
}

/* This function copies a block src to the block at pos. It also shifts insert
 * value from right to the left by leftshift bits.
 */
int
alder_wordtable_intblocklist_copyShiftLeftTo(alder_wordtable_intblocklist_t *o,
                                             alder_wordtable_t *src,
                                             size_t pos,
                                             int leftshift,int insert)
{
    size_t arrayindexpos = alder_wordtable_intblocklist_arrayindex(o,pos);
    size_t subarraypospos = alder_wordtable_intblocklist_subarraypos(o,pos);
    return alder_wordtable_bits_copyShiftLeft(src, 0,
                                              o->blockarray[arrayindexpos],
                                              subarraypospos * o->blocksize,
                                              o->blocksize, leftshift, insert);
}

/* 
 * FIXME: see who this is used.
 */
void
alder_wordtable_intblocklist_remove(alder_wordtable_intblocklist_t *o,
                                    size_t start, size_t end)
{
    assert(!(end < start));
    assert(!(o->size < end));
//    size_t len = end - start;
//    if (o->size - end < len) {
//        len = o->size - end;
//    }
    while (end > start) {
        o->size--;
        end--;
        alder_wordtable_intblocklist_copy(o, o->size, end);
    }
}

/* This sorts blocks in the list.
 */
void
alder_wordtable_intblocklist_sort(alder_wordtable_intblocklist_t *o)
{
    alder_wordtable_intblocklist_sort2(o,0,o->size - 1);
}

/* This sorts blocks in the list.
 */
void
alder_wordtable_intblocklist_sort2(alder_wordtable_intblocklist_t *o,
                                   size_t start, size_t end)
{
    if (start < end) {
        size_t middle = alder_wordtable_intblocklist_partition(o, start, end);
        
        if (middle > 0) {
            alder_wordtable_intblocklist_sort2(o, start, middle - 1);
        }
        alder_wordtable_intblocklist_sort2(o, middle + 1, end);
    }
}

/* This function returns the block size.
 */
size_t
alder_wordtable_intblocklist_blocksize(alder_wordtable_intblocklist_t *o)
{
    return o->blocksize;
}

#pragma mark static

/* blockarray
 *
 * The following is an example of block array.
 * blocksize = 2
 * subarraysize = 5
 *
 *  0  1 -  2  3 -  4  5 -  6  7 -  8  9
 * 10 11 - 12 13 - 14 15 - 16 17 - 18 19
 * 20 21 - 22 23 - 24 25 - 26 27 - 28 29
 *
 * pos = 7 -> 14 15
 * See how the position 7 can point 14 and 15.
 * arrayindex  = pos / subarraysize = 7 / 5 = 1
 * subarraypos = pos % subarraysize = 7 % 5 = 2
 * subindex(7,subpos) = 2 * 2 + pos
 * so, ...
 * subindex(7,0) = 2 * 2 + 0 = 4 -> 5th element in the 2nd subarray -> 14
 * subindex(7,1) = 2 * 2 + 1 = 5 -> 6th element in the 2nd subarray -> 15
 */
inline size_t
alder_wordtable_intblocklist_arrayindex
(alder_wordtable_intblocklist_t *o, size_t pos)
{
    return pos / o->subarraysize;
}

inline size_t
alder_wordtable_intblocklist_subarraypos
(alder_wordtable_intblocklist_t *o, size_t pos)
{
    return pos % o->subarraysize;
}

inline size_t
alder_wordtable_intblocklist_subindex
(alder_wordtable_intblocklist_t *o, size_t pos, size_t subpos)
{
    return (pos % o->subarraysize) * o->blocksize + subpos;
}

/* This function is used to sort the list.
 */
int
alder_wordtable_intblocklist_compare(alder_wordtable_intblocklist_t *o,
                                     size_t p,
                                     size_t q)
{
    size_t arrayindex_p = alder_wordtable_intblocklist_arrayindex(o,p);
    size_t subarraypos_p = alder_wordtable_intblocklist_subarraypos(o,p);
    size_t arrayindex_q = alder_wordtable_intblocklist_arrayindex(o,q);
    size_t subarraypos_q = alder_wordtable_intblocklist_subarraypos(o,q);
    alder_wordtable_t *pArray = o->blockarray[arrayindex_p];
    alder_wordtable_t *qArray = o->blockarray[arrayindex_q];
    size_t pIndex = subarraypos_p * o->blocksize;
    size_t qIndex = subarraypos_q * o->blocksize;
    for (size_t i = 0; i < o->blocksize; i++) {
        // The following is not necessary if word is unsigned.
        alder_wordtable_t u = pArray[pIndex + i];
        alder_wordtable_t v = qArray[qIndex + i];
        if (u > v) {
            return 1;
        } else if (u < v) {
            return -1;
        }
    }
    return 0;
}

/* This function is used to sort the list.
 */
void
alder_wordtable_intblocklist_swap(alder_wordtable_intblocklist_t *o,
                                  size_t p,
                                  size_t q)
{
    size_t arrayindex_p = alder_wordtable_intblocklist_arrayindex(o,p);
    size_t subarraypos_p = alder_wordtable_intblocklist_subarraypos(o,p);
    size_t arrayindex_q = alder_wordtable_intblocklist_arrayindex(o,q);
    size_t subarraypos_q = alder_wordtable_intblocklist_subarraypos(o,q);
    alder_wordtable_t *pArray = o->blockarray[arrayindex_p];
    alder_wordtable_t *qArray = o->blockarray[arrayindex_q];
    size_t pIndex = subarraypos_p * o->blocksize;
    size_t qIndex = subarraypos_q * o->blocksize;
    for (size_t i = 0; i < o->blocksize; i++) {
        alder_wordtable_t t = pArray[pIndex + i];
        pArray[pIndex + i] = qArray[qIndex + i];
        qArray[qIndex + i] = t;
    }
}


/* This function is used to sort the list (quick-sort).
 */
size_t
alder_wordtable_intblocklist_partition(alder_wordtable_intblocklist_t *o,
                                       size_t l, size_t r)
{
    int a;
    size_t pivot, i, j;
    pivot = l;
    i = l;
    j = r + 1;
    
    while (1) {
        do {
            ++i;
            a = alder_wordtable_intblocklist_compare(o,i,pivot);
        } while (a <= 0 && i <= r);
//        } while (a[i] <= pivot && i <= r);
        
        do {
            --j;
            a = alder_wordtable_intblocklist_compare(o,j,pivot);
        } while (a > 0);
//        while (a[j] > pivot);
        
        if (i >= j) break;
        
        // swap i and j.
        alder_wordtable_intblocklist_swap(o,i,j);
//        t = a[i]; a[i] = a[j]; a[j] = t;
    }
    // swap l and j.
    alder_wordtable_intblocklist_swap(o,l,j);
//    t = a[l]; a[l] = a[j]; a[j] = t;
    return j;
}

/* This function copies src pointed by block to dest.
 */
//void
//alder_wordtable_intblocklist_copy2(alder_wordtable_intblocklist_t *o,
//                                   alder_wordtable_t *src,
//                                   alder_wordtable_t *dest)
//{
//    memcpy(dest, src, sizeof(*dest) * o->blocksize);
//}

/*
 */
//void
//alder_wordtable_intblocklist_swapIntervals(alder_wordtable_intblocklist_t *o,
//                                           size_t p,
//                                           size_t q,
//                                           size_t len)
//{
//    while (len > 0) {
//        len--;
//        alder_wordtable_intblocklist_swap(o, p+len, q+len);
//    }
//}

/* Increase maxsize only. */
/*
 */
//size_t
//alder_wordtable_intblocklist_resizeSize(alder_wordtable_intblocklist_t *o)
//{
//    return o->maxsize + (o->maxsize >> 1) + 1;
//}

/*
 */
//void
//alder_wordtable_intblocklist_resetSizeCount(alder_wordtable_intblocklist_t *o)
//{
//    assert(0);
//    abort();
//    // Nothing.
//}

/* 
 * This function deletes blocks.
 */
//void
//alder_wordtable_intblocklist_empty(alder_wordtable_intblocklist_t *o)
//{
//    o->size = 0;
//    alder_wordtable_intblocklist_resetSizeCount(o);
//}
