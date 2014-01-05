/**
 * This file, alder_type_word.c, is part of alder-type.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-type is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-type is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-type.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "alder_type_word.h"

#pragma mark macro

/* The following function should be written in C macro. */

/* This function tests whether the value is a reference to a descriptor.
 */
bool IS_REFERENCE(word_t x)
{
   	return (x & REFERENCE_BIT_PATTERN) == REFERENCE_BIT_PATTERN;
}

/* This function converts the word type to a pointer to a descriptor.
 * The first bit must be 1.
 * I select the last 63 bits from a number of type word_t.
 */
descriptor_t *GET_DESCRIPTOR(word_t x)
{
    assert(((x >> (sizeof(x)*8-1)) & 1) == 1);
    assert(((1LLU << 63) - 1LLU) == ALL_ONE_MSB_ZERO);
    return (descriptor_t*)(x & ALL_ONE_MSB_ZERO);
}

/* This function converts a pointer to descriptor to a word.
 *
 * ----------------------------------------------------------------
 * Make sure that index is less than 1024 or 2^10.
 *
 * ----------------------------------------------------------------
 * Make sure that the first 11 bits are zero.
 * code: assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
 * 1. Create a number 1: 1LLU.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000000000000001
 * 2. Shift it 10 bits to the left: 1LLU << 11.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000100000000000
 * 3. Subtract 1 from it: (1LLU << 11) - 1LLU.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000011111111111
 * 4. Shift it 53 bits to the left: ((1LLU << 11) - 1LLU) << 53.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1111111111100000000000000000000000000000000000000000000000000000
 * 5. AND a number with the bit-wise number to make sure that 0 is the result.
 *
 * ----------------------------------------------------------------
 * Make sure that the first bit is 0.
 * code: assert((x & (1LLU << 63)) == 0);
 * 1. Create a number 1: 1LLU.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 0000000000000000000000000000000000000000000000000000000000000001
 * 2. Shift it 10 bits to the left: 1LLU << 63.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1000000000000000000000000000000000000000000000000000000000000000
 * 3. AND a number with the bit-wise number to make sure that you have 0.
 *
 * ----------------------------------------------------------------
 * Make sure that the index is less than 2^10 using bit operations.
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * The position has 1 and the rest of them are zero.
 * (1LLU << 10) makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000010000000000
 *                                                      ^
 * (1LLU << 10) - 1LLU ~ Subtracting 1 from it makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 *                                                      ^
 * ~((1LLU << 10) - 1LLU) ~ Negating it makes the number:
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 1111111111111111111111111111111111111111111111111111110000000000
 *                                                      ^
 * I can use it to check if the number can have 1's upto 10 bits.
 *
 * Now, let's convert the descriptor and the index to a word.
 * code: x = x | (1LLU << (sizeof(x)*8-1)) | (word_idx <<= 53);
 * 1. I have x which is the word value of the pointer.
 * word_t x = (word_t)descriptor;
 * 2. Create a bit-wise number with the first bit set.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                                ^
 * 1000000000000000000000000000000000000000000000000000000000000000
 * 3. Shift the idx 53 bits to the left.
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 *                                                      ^
 * 0111111111100000000000000000000000000000000000000000000000000000
 * 4. OR x and the two bit-wise numbers.
 */
word_t MAKE_REF(descriptor_t *descriptor, int idx)
{
    assert(0 <= idx && idx < 1024);
    word_t x = (word_t)descriptor;
    assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
    assert(((x >> (sizeof(x)*8-1)) & 1) == 0);
    assert((x & (1LLU << 63)) == 0);
    
    word_t word_idx = (word_t)idx;
    assert((word_idx & ~((1LLU << 10) - 1LLU)) == 0);
    x = x | (1LLU << (sizeof(x)*8-1)) | (word_idx <<= 53);
    return x;
}

/* This function converts a pointer to descriptor to a number.
 * The last 53 bits are the values to the pointer.
 * 
 * Make sure that the first 11 bits are zero (See MAKE_REF).
 */
word_t MAKE_DESCRIPTOR(descriptor_t *descriptor)
{
    word_t x = (word_t)descriptor;
    assert((x & (((1LLU << 11) - 1LLU) << 53)) == 0);
    x &= (1LLU << 53) - 1LLU;
    return x;
}

/* This function converts a pointer to descriptor to a status.
 */
word_t MAKE_STATUS_GIVE_DESCRIPTOR(descriptor_t *descriptor)
{
    word_t x = (word_t)descriptor;
    return x*2+2;
}

/* This function converts a pointer to descriptor to a status.
 */
word_t MAKE_STATUS_TRYING_DESCRIPTOR(descriptor_t *descriptor)
{
    word_t x = (word_t)(descriptor->seq);
    return x*2+3;
}

/* This function returns the index value.
 * The index value is between 63 and 54:
 * 2^10=1024 possible values of index.
 * 
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *  ^^^^^^^^^^
 * I want to take the 10-bit as a number of type word_t.
 * 1. Shift the number 53 bits to the right.
 * 2. Select the 10 bits.
 *
 * (1LLU << 10) - 1LLU -> be equal to 1023 or
 * the last 10 bits are set to 1, and the rest of bits are zero.
 *
 * 1. I have a number x, and want to extract the 10 bits of x.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *  ^^^^^^^^^^
 * 2. Shift the number 53 bits to the right.
 * 4321098765432109876543210987654321098765432109876543210987654321
 *                                                       ^^^^^^^^^^
 * 3. Create a bit-wise number with the last 10 bits set on.
 * (1LLU << 10) - 1LLU
 * 4321098765432109876543210987654321098765432109876543210987654321
 * 0000000000000000000000000000000000000000000000000000001111111111
 * 4. AND the two numbers to extract the 10 bits of index.
 */
word_t GET_INDEX(word_t x)
{
   	return (x >> 53) & ((1LLU << 10) - 1LLU);
}

/* This function gets the value of a word.
 * The MSB of the word must be 0.
 *
 * This is a 64-bit number positions.
 * 4321098765432109876543210987654321098765432109876543210987654321
 * ^
 * I want to check if the first bit is 0.
 * 1. Shift the number 63 (=8*8-1) bits to the right.
 * 2. Select the last bit.
 */
word_t GET_VALUE(word_t value)
{
    assert(((value >> (sizeof(value)*8-1)) & 1) == 0);
    return value;
}

word_t MAKE_VALUE(word_t value)
{
    return value;
}

// STATUS_TRYING<0> = 3 or status*2 + 3.
// STATUS_GIVE<0> = 2 or status*2 + 2.
int STATUS_TRYING_N(int N)
{
    return N*2 + 3;
}
int STATUS_GIVE_N(int N)
{
    return N*2 + 2;
}

word_t ADD_AVOIDLIST(word_t y, int x)
{
    y |= (1LLU << x);
    return y;
}

word_t IS_EMPTY_AVOIDLIST(word_t x)
{
    return x == 0;
}


#pragma mark atomic

static bool CASstatus(word_t *ptr, word_t oldval, word_t newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

static bool CASword(word_t *ptr, word_t oldval, word_t newval)
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

static word_t SWAPword(word_t *ptr, word_t value)
{
    return __sync_lock_test_and_set(ptr, value);
}

/* This function atomically add value to the value pointed by ptr.
 */
static void FAA(int *ptr, int value)
{
    __sync_fetch_and_add(ptr, value);
}

#pragma mark global

#define NR_THREADS 2
#define NR_INDICES 2

int annBusy[NR_THREADS][NR_INDICES];
int annIndex[NR_THREADS];
word_t annReadAddr[NR_THREADS][NR_INDICES];

int alder_type_word()
{
//    descriptor_t b;
//    word_t a;
//    descriptor_t *desc = &b;
//    void *address = &b;
    
//    uint64_t addr_b = MAKE_REF(address);
    
//    descriptor_t *desc2 = GET_DESCRIPTOR(addr_b);
    
//    a = 0;
//    a.value = (uint64_t)&b;
//    a.word_type = 0;
//    word_t *address = &a;
//    descriptor_t *desc = &b;
//    
//    /* */
//    annReadAddr[0][0].value = (uint64_t)desc;
//    annReadAddr[0][0].word_type = 0;
//    
//    word_t ref = *address;
//    IS_REFERENCE(ref);
//    
//    descriptor_t *desc2 = GET_DESCRIPTOR(ref);
//    
//    
//    size_t s = sizeof(a);
//    a.word_type = 1;
//    a.word_type_dummy = 0;
//    a.word_type = 1;

    printf("END OF WORD FUNCTION\n");
    return 0;
}


#pragma mark CASN

/* This function returns the contents of the read memory word,
 * and it it was a reference it also returns a pointer to the
 * corresponding descriptor.
 *
 * 2: Choose first index >= 0 such that nnBusy[hreadID][index] == 0.
 * 3: Announce that I will use access the index.
 * 4: ...
 * 5: Read the content of the memory.
 * 6: Test if the value is a pointer to a descriptor.
 * 7: Fetch the descriptor of the reference.
 * 8: Increase the reference count.
 */
word_t WFRead(word_t *address, descriptor_t **desc)
{
    word_t ref;
    int idx = -1;
    int threadID = 0;
    /* 2 */
    for (int i = 0; i < NR_INDICES; i++) {
        if (annBusy[threadID][i] == 0) {
            idx = i;
            break;
        }
    }
    assert(idx >= 0);
    /* 3 */
    annIndex[threadID] = idx;
    /* 4 */
    annReadAddr[threadID][idx] = (word_t)address;
    /* 5 */
    ref = *address;
    /* 6 */
    if (IS_REFERENCE(ref)) {
        /* 7 */
        *desc = GET_DESCRIPTOR(ref);
        /* 8 */
        FAA(&(*desc)->refcount,1);
        /* 9 */
        word_t refRep = SWAPword(&annReadAddr[threadID][idx], 0);
        /* 10 */
        if (refRep != (word_t)address) {
            if (IS_REFERENCE(ref)) FAA(&(*desc)->refcount,-1);
            ref = refRep;
        }
        if (IS_REFERENCE(ref)) *desc = GET_DESCRIPTOR(ref);
    }
    return ref;
}

/* This function is called by CASN and by itself.
 *
 * 2: Retry if I need to.
 * 3: Intialize local variables.
 * 4: Keep trying ...
 * 5: Set the current status.
 * 6: Go back to retry if the status changed.
 * 7: STATUS_TRYING!
 * 8: desc2 is a so-called conflicting thread.
 * 9: Read the value at i-th entry (word) address. Retrieve the descriptor if
 *    the value is an address for a descriptor. 
 * 10: Check if the value at the address is an address for a descriptor.
 * 11: ALREADY LOCKED BY MYSELF!
 * 12: 
 * 13:
 * 14: Check if the index reached the last entry.
 * 15:
 * 16:
 * 17:
 * 18:
 * 19: ... NOT SURE WHAT THIS IS ABOUT.
 * 20:
 * 21:
 * 22: LOCKED BY SOMEONE ELSE!
 * 23: Get the other's status, or status2.
 * 24: Check if the other's status is SUCCESS or not.
 * 25: Get the old value of the other's if not.
 * 26: Get the new value if SUCCESS.
 * 27: Check if the other's value is the same as the current's value.
 * 28: FAILED if they are different.
 * 30: Check if the value at i-th entry is equal to the old value if the
 *     value is a "value" not an address for a descriptor.
 * 31: Change the status of the descriptor to FAILED.
 * 32: Case where the value at i-th entry is equal to the old value.
 * 33: Check if the other thread is in avoidList.
 * 34:
 * 35:
 * 36:
 * 37:
 * 38:
 * 39:
 */
void HelpCompareAndSwap(descriptor_t *desc, uint64_t avoidList)
{
    int index; 
    int retry;
    bool conflict;
    word_t oldStatus;
    /* 2 */
retry:
    /* 3 */
    index = 0;
    retry = 0;
    conflict = false;
    oldStatus = desc->status;
    /* 4 */
    while (true) {
        /* 5 */
        word_t status = desc->status;
        /* 6 */
        if (status != oldStatus) goto retry;
        /* 8 */
        descriptor_t *desc2 = NULL;
        /* 9 */
        word_t ref = WFRead(desc->words[index].address, &desc2);
        /* 7 */
        if (IS_STATUS_TRYING(status)) {
            /* 10 */
            if(IS_REFERENCE(ref)) {
                /* 11 */
                if(desc2 == desc) {
                    /* 12 */
                next_word:
                    /* 13 */
                    if((++index) == retry) retry = 0;
                    /* 14 */
                    if(index == desc->size) {
                        /* 15 */
                        if(retry == 0 && desc->status == status) {
                            /* 16 */
                            if(conflict) return;
                            /* 17 */
                            CASstatus(&desc->status, status, STATUS_SUCCESS);
                        }
                        /* 19 */
                        index = (retry?retry-1:0);
                    }
                }
                /* 22 */
                else {
                    word_t value2;
                    /* 23 */
                    word_t status2 = desc2->status;
                    /* 24 */
                    if(status2 != STATUS_SUCCESS) {
                        /* 25 */
                        value2 = desc2->words[GET_INDEX(ref)].old;
                    }
                    else {
                        /* 26 */
                        value2 = desc2->words[GET_INDEX(ref)].new;
                    }
                    /* 27 */
                    if(value2 != desc->words[index].old) {
                        /* 28 */
                        CASstatus(&desc->status, status, STATUS_FAILED);
                    }
                    /* 30 */
                    else if(status2 == STATUS_FAILED || status2 == STATUS_SUCCESS) {
                        if(CASword(desc->words[index].address,
                                   ref,
                                   MAKE_REF(desc,index))) {
                            goto next_word ;
                        }
                    }
                    /* 33 */
                    else if ((1LLU << desc2->thread) & avoidList) {
                        /* 34 */
                        conflict = true;
                        /* 35 */
                        goto next_word;
                    }
                    /* S1 */
                    else if(desc->thread < desc2->thread) {
                        if((index+1) != retry) { // Try all other words first
                            if(retry == 0) retry = index+1;
                            if(++index == desc->size) {
                                index = (retry?retry-1:0);
                            }
                        }
                        /* S7 */
                        else { // Try grabbing
                            /* S8 */
                            retry = 0;
                            /* S9 */
                            if(IS_STATUS_TRYING(status2) &&
                               /* S10 */
                               CASstatus(&desc2->status,
                                         status2,
                                         MAKE_STATUS_GIVE_DESCRIPTOR(desc))) {
                                   /* S11 */
                                   HelpGiveAway(desc,desc2);
                                   /* S11 */
                                   FAA(&desc2->seq,1);
                                   /* S13 */
                                   CASstatus(&desc2->status,
                                             MAKE_STATUS_GIVE_DESCRIPTOR(desc),
                                             MAKE_STATUS_TRYING_DESCRIPTOR(desc2));
                            }
                            else {
                                /* S15 */
                                avoidList = ADD_AVOIDLIST(avoidList, desc->thread);
                                HelpCompareAndSwap(desc2, avoidList);
                            }
                        }
                    }
                    /* S18 */
                    else {
                        /* S19 */
                        avoidList = ADD_AVOIDLIST(avoidList, desc->thread);
                        HelpCompareAndSwap(desc2, avoidList);
                        /* S20 */
                        word_t status2 = desc2->status;
                        if(IS_STATUS_TRYING(status2) &&
                           CASstatus(&desc->status, status,
                                     MAKE_STATUS_GIVE_DESCRIPTOR(desc2))) {
                               /* S22 */
                               HelpGiveAway(desc2,desc);
                               /* S23 */
                               FAA(&desc->seq,1);
                               /* S24 */
                               CASstatus(&desc->status,
                                         MAKE_STATUS_GIVE_DESCRIPTOR(desc2),
                                         MAKE_STATUS_TRYING_DESCRIPTOR(desc));
                               /* S25 */
                               avoidList = ADD_AVOIDLIST(avoidList, desc->thread);
                               HelpCompareAndSwap(desc2, avoidList);
                        }
                    }
                }
            }
            /* 30 */
            else if (ref != desc->words[index].old) {
                /* 31 */
                CASstatus(&desc->status, status, STATUS_FAILED);
            }
            /* 32 */
            else {
                /* 33 */
                if (CASword(desc->words[index].address, ref, MAKE_REF(desc, index))) {
                    /* 34 */
                    goto next_word;
                }
            }
        }
        /* S37 */
        else if (status == STATUS_FAILED) {
            for(int index = desc->size-1; index >= 0; index--) {
                if(*(desc->words[index].address) == MAKE_REF(desc, index)) {
                    CASword(desc->words[index].address, ref, desc->words[index].old);
                }
            }
            return;
        }
        /* S44 */
        else if(status == STATUS_SUCCESS) {
            for(int index = desc->size-1;index >= 0;index--) {
                if(*(desc->words[index].address) == MAKE_REF(desc, index))
                    CASword(desc->words[index].address, ref, desc->words[index].new);
            }
            return;
        }
        /* S51 */
        else if(IS_STATUS_GIVE(status)) {
            if(status == desc->status) HelpGiveAway(desc2,desc);
            FAA(&desc->seq,1);
            CASstatus(&desc->status, status, MAKE_STATUS_TRYING_DESCRIPTOR(desc));
            if (IS_EMPTY_AVOIDLIST(avoidList)) {
                return;
            }
        }
    }
}

/* This function
 *
 * 1:
 * 2:
 * 3:
 * 4:
 */
void HelpGiveAway(descriptor_t *descTo, descriptor_t *descFrom)
{
    word_t oldStatus;
    int idx;
    bool taken;
    word_t value2;
    word_t status;

    /* 2 */
    oldStatus = descTo->status;
    idx = 0;
    taken = false;
    /* 3 */
    while (true) {
        /* 4 */
        status = descTo->status;
        /* 5 */
        if (status != oldStatus) {
            /*  6 */
            idx = 0;
            oldStatus = status;
        }
        word_t ref = *(descTo->words[idx].address);
        /* 8 */
        if (IS_STATUS_TRYING(status)) {
            if(IS_REFERENCE(ref) && GET_DESCRIPTOR(ref) == descFrom) {
                word_t status2 = descFrom->status;
                if(status2 != STATUS_SUCCESS) {
                    value2 = descFrom->words[GET_INDEX(ref)].old;
                }
                else {
                    value2 = descFrom->words[GET_INDEX(ref)].new;
                }
                if(value2 != descTo->words[idx].old) {
                    CASstatus(&descTo->status, status, STATUS_FAILED);
                }
                /* 18 */
                else if(status2 == MAKE_STATUS_GIVE_DESCRIPTOR(descTo)) {
                    if(CASword(descTo->words[idx].address,
                               ref,
                               MAKE_REF(descTo,idx))) {
                        taken = true;
                    }
                }
            }
            /* 21 */
            else if(!IS_REFERENCE(ref) && ref != descTo->words[idx].old) {
                CASstatus(&descTo->status, status, STATUS_FAILED);
            }
            /* 23 */
            if(++idx == descTo->size) {
                if(descTo->status == status) return;
                idx = 0;
            }
        }
        /* 28 */
        else if(status == STATUS_FAILED && taken) {
            for(int index = descTo->size-1; index>= 0; index --) {
                if(*(descTo->words[index].address) == MAKE_REF(descTo,index))
                    CASword(descTo->words[index].address, ref, descTo->words[index].old);
            }
            return;
        }
        /* 35 */
        else if(status == STATUS_SUCCESS && taken) {
            for(int index = descTo->size-1;index>= 0; index--) {
                if (*(descTo->words[index].address) == MAKE_REF(descTo, index)) {
                    CASword(descTo->words[index].address, ref, descTo->words[index].new);
                }
            }
            return;
            
        }
        /* 42 */
        else {
            return;
        }
    }
}

/* This function atomically reads the value at an address.
 */
word_t Read(word_t *address)
{
    descriptor_t *desc = NULL;
    word_t ref = WFRead(address, &desc);
    if (IS_REFERENCE(ref)) {
        assert(desc != NULL);
        if (desc->status != STATUS_SUCCESS) {
            return GET_VALUE(desc->words[GET_INDEX(ref)].old);
        }
        else {
            return GET_VALUE(desc->words[GET_INDEX(ref)].new);
        }
    }
    return GET_VALUE(ref);
}

/* This function atomically tries to update values in addresses
 */
bool CASN(int N, word_t **address, word_t *oldvalue, word_t *newvalue)
{
    int threadID = -1;
    assert(threadID >= 0);
    //    descriptor_t desc= Allocate a new descriptor from the wait-free local memory pools for which
    //    there is no active reference count, otherwise perform same helping steps as in line 12
    descriptor_t *desc = NULL;
    assert(desc != NULL);
    for(int i = 0; i < N; i++) {
        desc->words[i].address = address[i];
        desc->words[i].old = MAKE_VALUE(oldvalue[i]);
        desc->words[i].new = MAKE_VALUE(newvalue[i]);
    }
    desc->status = STATUS_TRYING_N(0);
    desc->seq = 0;
    desc->thread = threadID;
    HelpCompareAndSwap(desc, 0);
    bool success = (desc->status == STATUS_SUCCESS);
    //        Set aside esc for possible reclamation according to the wait-free memory reclamation method and if no active
    //            reference count then try to help all threads with reading that have announced in nnReadAddr an address
    //            that matches with any of the descriptors addresses
    assert(desc == NULL || desc != NULL);
    return success;
}
