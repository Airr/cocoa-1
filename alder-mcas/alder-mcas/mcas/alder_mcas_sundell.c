/**
 * This file, alder_mcas_sundell.c, is part of alder-mcas.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-mcas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-mcas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-mcas.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
//#include "alder_type.h"
#include "alder_atomic.h"
#include "alder_mcas_sundell.h"
#include "alder_memory_sundell.h"

#pragma mark shortcuts type
#define IS_REFERENCE alder_type_sundell_IS_REFERENCE
#define GET_DESCRIPTOR alder_type_sundell_GET_DESCRIPTOR
#define GET_VALUE alder_type_sundell_GET_VALUE
#define GET_INDEX alder_type_sundell_GET_INDEX
#define MAKE_VALUE alder_type_sundell_MAKE_VALUE
#define STATUS_TRYING_N alder_type_sundell_STATUS_TRYING_N
#define MAKE_REF alder_type_sundell_MAKE_REF
#define MAKE_STATUS_TRYING_DESCRIPTOR alder_type_sundell_MAKE_STATUS_TRYING_DESCRIPTOR
#define MAKE_STATUS_GIVE_DESCRIPTOR alder_type_sundell_MAKE_STATUS_GIVE_DESCRIPTOR
#define IS_EMPTY_AVOIDLIST alder_type_sundell_IS_EMPTY_AVOIDLIST
#define ADD_AVOIDLIST alder_type_sundell_ADD_AVOIDLIST
#define IS_IN_AVOIDLIST alder_type_sundell_IS_IN_AVOIDLIST

#pragma mark shortcuts memory
#define WFRead alder_memory_sundell_WFRead

#pragma mark shortcuts atomic
#define FAA alder_atomic_FAAint
#define SWAPword alder_atomic_SWAPword
#define CASword alder_atomic_CASword
#define CASstatus alder_atomic_CASstatus

typedef alder_type_sundell_descriptor_t descriptor_t;

#pragma mark static
static void HelpCompareAndSwap(descriptor_t *desc, uint64_t avoidList,
                               int threadId);
static void HelpGiveAway(descriptor_t *descTo, descriptor_t *descFrom);

#pragma mark function
/* This function atomically reads the value at an address.
 */
word_t alder_mcas_sundell_Read(word_t *address, int threadId)
{
    descriptor_t *desc = NULL;
    word_t ref = WFRead(address, &desc, threadId);
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
bool alder_mcas_sundell_CASN(int N, word_t **address,
                             word_t *oldvalue, word_t *newvalue,
                             int threadID)
{
    assert(threadID >= 0);
    // descriptor_t desc= Allocate a new descriptor from the wait-free local memory pools for which
    // there is no active reference count, otherwise perform same helping steps as in line 12
    descriptor_t *desc = alder_memory_sundell_AllocNode(threadID);
    assert(desc != NULL);
    for(int i = 0; i < N; i++) {
        desc->words[i].address = (*address + i);
        desc->words[i].old = MAKE_VALUE(oldvalue[i]);
        desc->words[i].new = MAKE_VALUE(newvalue[i]);
    }
    desc->status = STATUS_TRYING_N(0);
    desc->seq = 0;
    desc->thread = threadID;
    desc->size = N;
    HelpCompareAndSwap(desc, 0, threadID);
    bool success = (desc->status == STATUS_SUCCESS);
    alder_memory_sundell_HelpDeRef(&desc, threadID);
    alder_memory_sundell_ReleaseRef(desc, threadID);
    // Set aside desc for possible reclamation according to the wait-free memory reclamation method and if no active
    // reference count then try to help all threads with reading that have announced in annReadAddr an address
    // that matches with any of the descriptors addresses
    assert(desc == NULL || desc != NULL);
    return success;
}

#pragma mark static

/* This function is called by CASN and by itself.
 *
 * # Observations
 * 1. Try lock words (30-31 and S32-S33).
 * 2. STATUS will change to a FAIL if the old value and values in
 *    the conflicting thread (27-28).
 *
 * # Steps
 * 2: Retry if I need to.
 * 3: Intialize local variables.
 * 4: Keep trying ...
 * 5: Set the current status.
 * 6: Go back to retry if the status changed.
 * 7: STATUS_TRYING!
 * 8: desc2 is a so-called conflicting thread.
 * 9: Read the value at i-th entry (word) address. Retrieve the descriptor if
 *    the value is an address for a descriptor.
 *    Note that I increased the reference count in WFRead function.
 * 10: Check if the value at the address is an address for a descriptor.
 * 11: ALREADY LOCKED BY MYSELF!
 *     a) idx++
 *     b) status -> STATUS_SUCCESS
 *     c) THERE MUST BE OTHER THINGS GOING ON HERE.
 * 12: I DO NOT KNOW WHAT IS GOING ON IN THIS PART.
 *     Index value is incremented by 1. If the incremented index
 *     reached the retry value, the retry is reset to 0.
 *     If index reached the end of the words, I should
 *     exit the function. But, it does more than this with retry.
 *     WHAT IS RETRY ANYWAY?
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
 * 30: CONFLICTING THREAD IS DONE SO TRY TO LOCK.
 * 31: Try to lock the word.
 * 32: Go to a next word if the lock was successful.
 * 33: Test if the conflicting thread or desc2 belongs to avoidList.
 *     I skip this word, and go to a next word with conflict set on.
 *     This means that this call of of the procedure cannot decide
 *     the final status of the descriptor desc.
 *     WHY SHOULDN'T I STOP HERE?
 * 34:
 * 35:
 *
 * S1: HELPED THREAD HAS A LOWER ID THAN THE CONFLICTING THREAD.
 * S2~S5: I DO NOT KNOW WHAT IS GOING ON HERE.
 * S3:
 * S4:
 * S5:
 * S6:
 * S7:
 * S8:
 * S9: Test if conflicting thread is in STATUS_TRYING.
 * S10: Try to update the status of the conflicting descriptor from
 *      STATUS_TRYING<seq2> to STATUS_GIVE<helped descriptor>.
 * S11: Grab all of the words from the conflicting thread.
 *      desc happens to see desc2's ref in desc's address. 
 *      The desc (with a lower thread ID) replaces address's ref with desc's ref
 *      by calling HelpGiveAway(desc,desc2). This way, I resolve
 *      the conflicts between desc and desc2 by giving things to desc.
 *      Then, what would happen to desc2? SEE S12 and S13.
 *      So, desc2's status changes in S12.
 * S12: So, desc2's status changes in S12.
 * S13: Update the status of the conflicting descriptor to STATUS_TRYING<seq2+1>.
 *      The increased sequence number allows for the conflicting thread to know
 *      that words have been lost to some other thread.
 * S14: S9 failed.
 * S15: Call HelpCompareAndSwap for the conflicting descriptor with the
 *      currently helped thread added to avoidList.
 * S18: HELPED THREAD HAS HIGHER ID THAN THE CONFLICTING THREAD.
 * S19: Note that the helped thread's descriptor is desc.
 *      Call the HelpCompareAndSwap for the conflicting descriptor or
 *      desc2 with the currently helped thread (i.e., desc->thread) added
 *      to avoidList.
 * S20: 
 * S21: Try to change the status of the helped descriptor.
 * S22: Give away the required words to the conflicting thread.
 *
 * S30: Check if the value at i-th entry is equal to the old value if the
 *      value is a "value" not an address for a descriptor.
 *      If a word is found having the wrong value, the status is changed to
 *      STATUS_FAILED.
 * S31: Change the status of the descriptor to FAILED.
 * S32: Case where the value at i-th entry is equal to the old value.
 * S33: Lock the value at the i-entry with a reference.
 * S34: Go to a next word.
 * 35:
 * 36:
 * 37:
 * 38:
 * 39:
 *
 * local
 * idx - ?
 * retry - 0 or idx-1.
 * conflict -
 * oldStatus -
 * desc2 -
 *
 * DeRef:
 * 31
 */
static void HelpCompareAndSwap(descriptor_t *desc,
                               uint64_t avoidList,
                               int threadId)
{
    int idx;
    int tries;
    bool conflict;
    word_t oldStatus;
    descriptor_t *desc2;
    /* 2 */
retry:
    /* 3 */
    idx = 0;
    tries = 0;
    conflict = false;
    oldStatus = desc->status;
    assert(!IS_STATUS_GIVE(oldStatus));
    /* 8 */
    desc2 = NULL;
    /* 4 */
    while (true) {
        /* 5 */
        word_t status = desc->status;
        /* 6 */
        if (status != oldStatus) goto retry;
        /* 7 */
        if (IS_STATUS_TRYING(status)) {
            /* 9 */
            word_t ref = WFRead(desc->words[idx].address, &desc2, threadId);
            /* 10 */
            if(IS_REFERENCE(ref)) {
                /* 11 */
                if(desc2 == desc) {
                    /* 12 */
                next_word:
                    /* 13 */
                    if((++idx) == tries) tries = 0;
                    /* 14 */
                    if(idx == desc->size) {
                        /* 15 */
                        if(tries == 0 && desc->status == status) {
                            /* 16 */
                            if(conflict) return;
                            /* 17 */
                            CASstatus(&desc->status, status, STATUS_SUCCESS);
                        }
                        /* 19 */
                        idx = (tries?tries-1:0);
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
                    if(value2 != desc->words[idx].old) {
                        /* 28 */
                        CASstatus(&desc->status, status, STATUS_FAILED);
                    }
                    /* 30 */
                    else if(status2 == STATUS_FAILED || status2 == STATUS_SUCCESS) {
                        /* 31 */
                        if(CASword(desc->words[idx].address,
                                   ref,
                                   MAKE_REF(desc,idx))) {
                            /* 32 */
                            goto next_word ;
                        }
                    }
                    /* 33 */
                    else if (IS_IN_AVOIDLIST(desc2->thread, avoidList)) {
                        /* 34 */
                        conflict = true;
                        /* 35 */
                        goto next_word;
                    }
                    /* S1 */
                    else if(desc->thread < desc2->thread) {
                        if((idx+1) != tries) { // Try all other words first
                            if(tries == 0) tries = idx+1;
                            if(++idx == desc->size) {
                                idx = (tries?tries-1:0);
                            }
                        }
                        /* S7 */
                        else { // Try grabbing
                            /* S8 */
                            tries = 0;
                            /* S9 */
                            if(IS_STATUS_TRYING(status2) &&
                               /* S10 */
                               CASstatus(&desc2->status,
                                         status2,
                                         MAKE_STATUS_GIVE_DESCRIPTOR(desc))) {
                                   /* S11 */
                                   HelpGiveAway(desc,desc2);
                                   /* S12 */
                                   FAA(&desc2->seq,1);
                                   /* S13 */
                                   CASstatus(&desc2->status,
                                             MAKE_STATUS_GIVE_DESCRIPTOR(desc),
                                             MAKE_STATUS_TRYING_DESCRIPTOR(desc2));
                               }
                            /* S14 */
                            else {
                                /* S15 */
                                avoidList = ADD_AVOIDLIST(avoidList, desc->thread);
                                HelpCompareAndSwap(desc2, avoidList, threadId);
                            }
                        }
                    }
                    /* S18 */
                    else {
                        /* S19 */
                        avoidList = ADD_AVOIDLIST(avoidList, desc->thread);
                        HelpCompareAndSwap(desc2, avoidList, threadId);
                        /* S20 */
                        word_t status2 = desc2->status;
                        /* S21 */
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
                               HelpCompareAndSwap(desc2, avoidList, threadId);
                           }
                    }
                }
            }
            /* S30 */
            else if (ref != desc->words[idx].old) {
                /* S31 */
                CASstatus(&desc->status, status, STATUS_FAILED);
            }
            /* S32 */
            else {
                /* S33 */
                if (CASword(desc->words[idx].address, ref, MAKE_REF(desc, idx))) {
                    /* S34 */
                    goto next_word;
                }
            }
        }
        /* S37 */
        else if (status == STATUS_FAILED) {
            for(int index = desc->size-1; index >= 0; index--) {
                word_t ref = MAKE_REF(desc, index);
//                assert(*(desc->words[index].address) == ref);
                if(*(desc->words[index].address) == ref) {
                    CASword(desc->words[index].address, ref, desc->words[index].old);
                    // Decrement reference count.
//                    if (IS_REFERENCE(ref)) {
//                        descriptor_t *desc = GET_DESCRIPTOR(ref);
//                        FAA(&desc->mm_ref,-2);
//                    }
                }
            }
            return;
        }
        /* S44 */
        else if(status == STATUS_SUCCESS) {
            for(int index = desc->size-1;index >= 0;index--) {
                word_t ref = MAKE_REF(desc, index);
//                assert(*(desc->words[index].address) == ref);
                if(*(desc->words[index].address) == ref) {
                    CASword(desc->words[index].address, ref, desc->words[index].new);
                    // Decrement reference count.
//                    if (IS_REFERENCE(ref)) {
//                        descriptor_t *desc = GET_DESCRIPTOR(ref);
//                        FAA(&desc->mm_ref,-2);
//                    }
                }
            }
            return;
        }
        /* S51:????????????????????????????????????? */
        // When would this part be called?
        else {
            assert(desc2 != NULL);
            if(status == MAKE_STATUS_GIVE_DESCRIPTOR(desc2)) {
                if(status == desc->status) HelpGiveAway(desc2,desc);
                FAA(&desc->seq,1);
                CASstatus(&desc->status, status, MAKE_STATUS_TRYING_DESCRIPTOR(desc));
                if (IS_EMPTY_AVOIDLIST(avoidList)) {
                    return;
                }
            }
        }
    }
}

/* This function
 *
 * Give all of the values from descFrom to descTo.  
 * All of the word's address in descTo are replaced by descTo's ref.
 * Word's addresses in descTo used to be those of descFrom's ref.
 *
 * 1:
 * 2:
 * 3:
 * 4:
 */
static void HelpGiveAway(descriptor_t *descTo, descriptor_t *descFrom)
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
        /* 8 */
        if (IS_STATUS_TRYING(status)) {
            word_t ref = *(descTo->words[idx].address);
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
                word_t ref = MAKE_REF(descTo,index);
                if(*(descTo->words[index].address) == ref) {
                    CASword(descTo->words[index].address, ref,
                            descTo->words[index].old);
                }
            }
            return;
        }
        /* 35 */
        else if(status == STATUS_SUCCESS && taken) {
            for(int index = descTo->size-1;index>= 0; index--) {
                word_t ref = MAKE_REF(descTo,index);
                assert(*(descTo->words[index].address) == ref);
//                if (*(descTo->words[index].address) == MAKE_REF(descTo, index)) {
                    CASword(descTo->words[index].address, ref,
                            descTo->words[index].new);
//                }
            }
            return;
            
        }
        /* 42 */
        else {
            return;
        }
    }
}