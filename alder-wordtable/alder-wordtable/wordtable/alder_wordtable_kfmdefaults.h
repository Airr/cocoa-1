/**
 * This file, alder_wordtable_kfmdefaults.h, is part of alder-wordtable.
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

#ifndef alder_wordtable_alder_wordtable_kfmdefaults_h
#define alder_wordtable_alder_wordtable_kfmdefaults_h

#include <stdint.h>

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

/* Specify the default number of vertices in KFMindex.
 * The number is the exponent of 2: 2^20
 */
#define ALDER_WORDTABLE_BITPACKEDLIST_DEFAULT_INITSIZE 20

/* Specify the number of items in a subarray.
 * The number is the exponent of 2: 2^18
 */
#define ALDER_WORDTABLE_BITPACKEDLIST_SUBARRAYITEMS 0      // 18

/* Specify the number of blocks (8 bytes) of a subarray.
 * The number is the exponent of 2.
 */
#define ALDER_WORDTABLE_BIGBITARRAY_SUBARRAYSIZE 10 /* < 64 */

/* Specify the stepsize in the store-rho table.
 */
#define ALDER_WORDTABLE_KFMDEFAULTS_STEPSIZE  32

/* Specify the number of edges in a inedgelist for a KFMindex variable.
 */
#define ALDER_WORDTABLE_KFMDEFAULTS_BLOCKSIZE 1000000

/* NOT USE THIS
 * I'd use a different KFMmerging scheme.
 *
 * 0: no prune during merging
 * 1: prune merged one whenever I merged two KFMindex.
 * 2: wait a little
 * 3: wait more ...
 * Large values let us wait more till pruning.
 */
#define ALDER_WORDTABLE_KFMDEFAULTS_PRUNE_INTERMEDIATE_LEVELS 3


/* intblocklist */

/* Specify the initial number of subarrays for inedgelist.
 * This number does not affect the rate of increase in memory.
 * It justs specifies the initial size of inedgelist.
 */
#define ALDER_WORDTABLE_BLOCKARRAY_SIZE 10

/* Specify the number of blocks in a subarray.
 * The number is the exponent of 2: 2^15.
 * This number might need tuning because inedgelist increases by this size
 * of subarrays. If this is 20, then roughly 1 millions of blocks are added
 * when more space is needed. Each block would occupy a 4-byte or 8-byte value.
 * So, a number 20 means that 4 megabytes or 8 megabytes would be added 
 * whenever more space is needed.
 */
#define ALDER_WORDTABLE_SUBARRAYSIZE 15

/* KFMindex */
#define ALDER_WORDTABLE_KFMDEFAULTS_SIZE 100

/* */
#define ALDER_WORDTABLE_KFMDEFAULTS_BYTESIZE  8
#define ALDER_WORDTABLE_KFMDEFAULTS_ORDER     25
#define ALDER_WORDTABLE_KFMDEFAULTS_PRUNE_FINALS true


/* KFMaggregator */
#define ALDER_WORDTABLE_KFMDEFAULTS_STACKSIZE 10



/* GrowableArray */
#define ALDER_WORDTABLE_GROWABLEARRAY_INIT_SIZE 1

/* KFMmerger */
#define ALDER_WORDTABLE_KFMMERGER_FREESIZE 1000


#define ALDER_WORDTABLE_SUCCESS 0
#define ALDER_WORDTABLE_FAIL    1

#define ALDER_WORDTABLE_WORDSIZE_32
//#define ALDER_WORDTABLE_WORDSIZE_64

#if defined(ALDER_WORDTABLE_WORDSIZE_32)
typedef uint32_t alder_wordtable_t;
typedef uint64_t alder_dwordtable_t;
#elif defined(ALDER_WORDTABLE_WORDSIZE_64)
typedef uint64_t alder_wordtable_t;
typedef uint64_t alder_dwordtable_t;
#endif

#define ALDER_WORDTABLE_TOKENSIZE 4
#define ALDER_WORDTABLE_MAX_SIZE_T (~(size_t)0)

__END_DECLS


#endif /* alder_wordtable_alder_wordtable_kfmdefaults_h */
