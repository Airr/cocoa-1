//
//  wf_mcas_2_0_1.c
//  MCAS_V2
//  Created by Steven Feldman on 6/23/13.
//  Copyright (c) 2013 Steven FELDMAN. All rights reserved.
//

/* by Sang Chul Choi
 * How to use these functions?
 *
 * void mcas_init(int threads);
 * void * mcas_read_strong(void * address){
 * bool call_mcas(int count, void **address, void *old_v, void *new_v, ...){
 */
#include "alder_mcas_wf.h"
#define NDEBUG
#ifndef INLINE
#define INLINE static inline
#endif

//#define DEBUG2
#define helpDelay 1000

//#define DONTFREE -1
#ifndef MAX_MCAS_FAILURE
#define MAX_MCAS_FAILURE 1000
#endif

#define MCH_POOL_SIZE 64
/**
 Testing
 **/
#ifdef DEBUG
#include <pthread.h>
#endif
#include <setjmp.h>     /* jmp_buf, setjmp, longjmp */

#include <string.h> /* sangchul: memcpy */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

typedef struct CasRow_struct {
	void **address;
	void *expectedValue;
	void *newValue;
	void *mch;
} CasRow;

//struct CasRow ;
//struct MCASHelper;

typedef struct MCASHelper_struct {
	CasRow *cr_pointer;
	CasRow *endRow;
#ifdef DEBUG2
	void *replace;
	void *replacedBy;
#endif
    //Memory Management
	volatile int rc;
	struct MCASHelper_struct *next;
} MCASHelper;

INLINE BOOL shouldReplace(void *expectedValue, void *mch_m);

void	print_mcas(void *mcas_op);

BOOL perform_mcas(CasRow *mcas, CasRow *endRow);
INLINE void place_mcas_helper(CasRow * cr, CasRow *endRow, BOOL firstTime);
//inline void cleanup_mcas(CasRow* mcas, BOOL res, int count);
inline void cleanup_mcas(CasRow *mcas, BOOL passed, CasRow * lastRow);

INLINE void helpIfNeeded();


INLINE BOOL isMCASHelper(void * pointer);
INLINE void *markMCASHelper(void * pointer);
INLINE MCASHelper *unmarkMCASHelper(void * pointer);

INLINE void pointer_watch_rc_up  (MCASHelper *address);
INLINE void pointer_watch_rc_down(MCASHelper *address);
INLINE BOOL iswatched(MCASHelper *address);

INLINE void free_wrapper(void *address);
INLINE void safe_free_mcas(void ** address);
INLINE void free_mcas(void **address);
INLINE void safe_free_mch(MCASHelper *address);
INLINE BOOL is_freeable_mcas(void *address);

INLINE void *allocate_mcasHelpr(CasRow * cr, CasRow *endRow);

INLINE void *CAS_WRAPPER(void * address, void *expectedValue, void *newValue);


volatile int threadCount=0;

static int  nThreads=0;
__thread int threadID=0;
__thread int tl_helpID=0;
__thread int tl_Delay=0;
__thread int tl_depth=0;
__thread jmp_buf jumpBuffer;

void **pendingMCASTable;

__thread void** recycled_mcas = NULL;

__thread MCASHelper* recycled_mch=NULL;

#ifdef DEBUG
void dump_trace();
pthread_mutex_t dumplock;
FILE *fout_cas;
#endif


#ifdef DEBUG2
void searchMCH(void *v){
    MCASHelper *mch=unmarkMCASHelper(recycled_mch);
	
    while(mch!=NULL){
        if(mch->replacedBy== v || mch->replace ==v){
            printf("MCH: %p CR: %p ER: %p R: %p RB:%p\n",mch, mch->cr_pointer, mch->endRow, mch->replace, mch->replacedBy);
        }
        mch=unmarkMCASHelper(mch->next);
    }
	
};
#endif

void mcas_init(int threads){
	nThreads=threads;
	pendingMCASTable=(void **)calloc(threads,sizeof(void *));
	
#ifdef DEBUG
//	printf("Threads: %d\n", nThreads);
	pthread_mutex_init(&dumplock, NULL);
	fout_cas=fopen("CASRes.txt", "w");
	
#endif
}
int attach_mcas_thread(){
	threadID=__sync_fetch_and_add(&threadCount,1);
#ifdef DEBUG
	printf("Thread %d has attached\n", threadID);
#endif
	tl_helpID=0;
	
	int i;
	
	MCASHelper* mch = NULL;
	
	for(i=0; i < MCH_POOL_SIZE; i++){
		MCASHelper* temp=(MCASHelper*)malloc(sizeof(MCASHelper));
		temp->rc=0;
		temp->next=mch;
		mch=temp;
        
	}
	recycled_mch=mch;
	
	return threadID;
}


__thread MCASHelper * tl_mch;


BOOL perform_mcas(CasRow* mcas, CasRow *endRow){
	tl_depth=0;
	//Acquire address
	helpIfNeeded();
	
    //	CasRow* mcaso=mcas; // Commented out because of no use of it.
	
	BOOL res=true;
	
	place_mcas_helper(mcas++, endRow, true);
	for(mcas=mcas; mcas<=endRow; mcas++){
        //while(mcas[i].address != (void *)0x1){
        //do{
		void *erValue=endRow->mch;
		if( erValue == (void*)(~0x0)){
			res=false;
			break;
		}//End if not succefull
		else if( erValue != (void*)(0x0) ){
			res=true;
			break;
		}
		else{
			assert((mcas-1)->mch != (void *)0x0 || (mcas-1)->mch  != (void *)WFBIT);
            
			if(mcas->mch == NULL)
				place_mcas_helper(mcas, endRow, false);
            
			if(mcas->mch == (void *)~0x0){
				res=false;
				break;
			}
            
			assert((mcas-1)->mch != (void *)0x0 || (mcas-1)->mch  != (void *)WFBIT);
			assert(mcas->mch != (void *)0x0 || mcas->mch  != (void *)WFBIT);
		}
		
		
	}
	pendingMCASTable[threadID]=NULL;
    //}while( (mcas-1) != endRow);
	//End While
	assert(endRow->mch != (void *)0x0 || endRow->mch  != (void *)WFBIT);
	//Need Both Because of thread delay!
	//return ( endRow->mch != (void*)~(0x0) &&  mcas->mch != (void*)~(0x0) );
	/*#ifdef DEBUG
     if(res){
     for(mcas=mcaso; mcas<=endRow; mcas++){
     if(mcas->mch == (void *)0x0  || mcas->mch == (void *)(~0x0)  ){
     printf("True, %p but not fully associated! TID: %d\n", mcaso, threadID);
     break;
     }
     }
     for(mcas=(mcaso+1); mcas<=endRow; mcas++){
     if(mcaso->newValue != mcas->newValue ){
     printf("True, %p Deviated! TID: %d\n", mcaso, threadID);
     break;
     }
     }
     }
     #endif
     */
	
	tl_mch=NULL;
	return res;
}



BOOL help_perform_mcas(CasRow* mcas, CasRow *endRow){
#ifndef NDEBUG
	void *omcas=mcas;
#endif
	if(tl_depth==0){
		//set jump
        //		int val=setjmp(jumpBuffer); // val is deleted because of no use of val.
		setjmp(jumpBuffer);
		tl_depth=0;
        
	}
	
	tl_depth++;
	
	
	if(tl_depth > nThreads){
		//Jump back to original
#ifdef DEBUG2
		printf("Over Recur Tl depth %d!!!!\n", tl_depth);
#endif
		longjmp(jumpBuffer, 2);
		
	}
	
	BOOL res=true;
	void *erValue;
	
    //	void *mcasbmch, *mcasmch, *ermch; ermch is not used.
	void *mcasbmch, *mcasmch;
	for(mcas=mcas; mcas<=endRow; mcas++){
		
		assert(endRow >=mcas);
		
		erValue=endRow->mch;
		if( erValue == (void*)(~0x0)){
			res=false;
			break;
		}//End if not succefull
		else if( erValue != (void*)(0x0) ){
			assert(erValue != (void *)~0x0);
			res=true;
			break;
		}
		else{
			mcasbmch=(mcas-1)->mch;
			assert(mcasbmch != (void *)0x0 || mcasbmch  != (void *)WFBIT);
            
			if(mcas->mch == NULL)
				place_mcas_helper(mcas, endRow, false);
            
			mcasmch=mcas->mch;
			mcasbmch=(mcas-1)->mch;
			if(mcasmch == (void *)~0x0){
				res=false;
				break;
			}
            
			assert(mcasbmch != (void *)0x0 || mcasbmch  != (void *)WFBIT);
			assert(mcasmch  != (void *)0x0 || mcasmch   != (void *)WFBIT);
		}
		
		
	}
	tl_depth--;
    //}while( (mcas-1) != endRow);
	//End While
    /*
     ermch=endRow->mch;
     assert(ermch != (void *)0x0 || ermch  != (void *)0x1);
     //Need Both Because of thread delay!
     //return ( endRow->mch != (void*)~(0x0) &&  mcas->mch != (void*)~(0x0) );
     if(res){
     if(ermch == (void *)~0x0){
     assert(ermch != (void *)~0x0);
     }
     }
     else{
     if(ermch != (void *)~0x0){
     assert(ermch == (void *)~0x0);
     }
     }
     */
    //	printf("TID: %d helping: %p/%p \n", threadID, omcas,endRow);
    // SangChul: FIXME - Let's silence this assert for a while.
	assert(omcas <= endRow);
	return res;
	
}

//__thread int shoudlReplaceCounter=0;

//Must be Assoicated BEFORE ENTERING!--RC Counting Demands it!
INLINE BOOL shouldReplace(void *expectedValue, void *mch_m){
    //	shoudlReplaceCounter++;
    
	assert(nThreads !=1);
	assert(isMCASHelper(mch_m));
	
	MCASHelper *mch=unmarkMCASHelper(mch_m);
	CasRow *cr=mch->cr_pointer;
	CasRow *endRow=mch->endRow;
	void *cr_mch=CAS_WRAPPER(&(cr->mch), NULL, mch_m);
	
	if(cr->expectedValue != expectedValue && cr->newValue != expectedValue){
		return false ;
	}
	
	assert(mch->rc > 0);
	
    
	if(	(endRow->mch == (void *)~(0x0) ) ||
       (cr_mch == (void *)~(0x0)) ||
       (cr_mch != mch_m)	){
		if(cr->expectedValue == expectedValue){
			return true;
		}
		else
			return false;
	}
	//SD now we know it is associated with the CAS Row.
	BOOL res=help_perform_mcas(cr, endRow);
	
    //	printf("%d [%d] Helped! MCH: %p Cr %p LR %p\n",threadID, shoudlReplaceCounter, mch_m, cr, mch->endRow);
#ifdef DEBUG
	if(res){
		CasRow *lmcas = mch->cr_pointer;
		for(lmcas=lmcas; lmcas <= mch->endRow; lmcas++){
			if(lmcas->mch == 0x0 || lmcas->mch == (void **)(~0x0)){
				printf("True but failed marker!\n");
			}
			if(lmcas->newValue != mch->endRow->newValue ){
                //	printf("Ah! Mistmatch!\n");
                //  assert(false);
			}
		}
		
        
	}
    /*	else{
     void *erValue = mch->endRow->mch;
     if(  erValue != (void **)(~(0x0))  ){
     printf("Failed but NO failed marker!\n");
     }
     
     }*/
#endif
	
	if(res){
		if(cr->newValue == expectedValue){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if(cr->expectedValue == expectedValue){
			return true;
		}
		else{
			return false;
		}
	}
    
}


void cleanup_mcas(CasRow *mcas, BOOL passed, CasRow * lastRow){
	//Acquire address
    
	void **omcas=(void **)mcas;
	for(mcas=mcas; mcas <= lastRow; mcas++){
		void *mch=mcas->mch;
		if(mch == (void *)~(0x0)){
#ifdef DEBUG
			if(passed){
				assert(!passed);
			}
#endif
			return;//End of placed mchs
		}
#ifdef DEBUG
		else if(mch == (void *)(0x0) && passed){
            
			printf("%d NULL in cleanup at %p and Passed\n",threadID,mcas);
			print_mcas(omcas);
			assert(false);
			return;//End of placed mchs
		}
#endif
		else if (mch == (void *)(0x0)){
#ifdef DEBUG
			printf("%d NULL in cleanup at %p and failed\n",threadID,mcas);
			print_mcas(omcas);
			assert(false);
#endif
			return;
		}
		
		if(passed){
            /*	if(mcas->expectedValue != lastRow->expectedValue){
             #ifdef DEBUG2
             printf(" %p: MCHp: %p\n", omcas, omcas[-1]);
             print_mcas(omcas);
             searchMCH(mcas->expectedValue);
             searchMCH(lastRow->expectedValue);
             #endif
             assert(false);
             }*/
#ifdef DEBUG2
			void *res=CAS_WRAPPER(mcas->address, mch, (void *)mcas->newValue);
			if(res==(void *)mch)
				unmarkMCASHelper(mch)->replacedBy=(void *)mcas->newValue;
#else
			CAS_WRAPPER(mcas->address, mch, (void *)mcas->newValue);
#endif
		}
		else{
#ifdef DEBUG2
			void *res=CAS_WRAPPER(mcas->address, mch, (void *)mcas->expectedValue);
			if(res==(void *)mch)
				unmarkMCASHelper(mch)->replacedBy=(void *)mcas->expectedValue;
#else
			CAS_WRAPPER(mcas->address, mch, (void *)mcas->expectedValue);
#endif
			
		}
	}
}


/*TODO:
 Implement setJump,long jump to return to the threads own operation when the recursive depth > nThreads (ie no dependcy must exist) OR when Max MCAS failure has been reached
 Add setjump, __thread local values when helpIfNeeded another threads operation, treat as own
 */

INLINE void place_mcas_helper(CasRow * cr, CasRow *endRow, BOOL firstTime){
	int tries=0;
    
	void *address=cr->address;
	void *expectedValue=cr->expectedValue;
	void *mch=allocate_mcasHelpr(cr, endRow);
	void *currentValue=*((void * volatile *)address);
	
	BOOL freeMCH=true;
	if(firstTime){
		tl_mch=unmarkMCASHelper(mch);
		((void **)(cr))[-1]=mch;
		freeMCH=false;
	}
	
	while(cr->mch == NULL){
		if(tries++ == MAX_MCAS_FAILURE){
			if(firstTime){
				cr->mch=NULL;
				firstTime=false;
			}
			pendingMCASTable[threadID]=tl_mch;
			
			if(tl_depth > 0){
				longjmp(jumpBuffer, 3);
			}
		}//End Tries
		
		if (!isMCASHelper(currentValue)){
			
			if(firstTime){
				cr->mch=mch;
			}
			currentValue=CAS_WRAPPER(address, expectedValue, mch); //Internall checks if equal
			if(currentValue==expectedValue){
#ifdef DEBUG2
				unmarkMCASHelper(mch)->replace=(void *)currentValue;
#endif
				if(!firstTime){
					currentValue=CAS_WRAPPER(&(cr->mch), NULL, mch);
					if(currentValue!=NULL &&  currentValue != mch){
#ifdef DEBUG2
						void *res3=CAS_WRAPPER(address, mch,expectedValue);
						if(res3==mch)
							unmarkMCASHelper(mch)->replacedBy=(void *)expectedValue;
#endif
						
					}//end else its not a succefull assoc
					else{
						freeMCH=false;
					}
				}
				else{
					freeMCH=false;
				}
				
				break;
				
			}//end if succfull cas
			else{
				if(firstTime){
					cr->mch=NULL;
				}
			}
		}
		
		if (isMCASHelper(currentValue)){
			assert(nThreads != 1);
			MCASHelper *mch2=unmarkMCASHelper(currentValue);
            
			pointer_watch_rc_up(mch2);
			void *currentValue2=*((void * volatile *)address);
			if(currentValue != currentValue2){
				currentValue=currentValue2;
				pointer_watch_rc_down(mch2);
				continue;
			}//end rc  check
			
			
			
			CasRow *other_cr=mch2->cr_pointer;
			CasRow *other_er=mch2->endRow;
            
			if(endRow == other_er){
				currentValue2=CAS_WRAPPER(&(cr->mch), NULL, currentValue);
				if(currentValue2!=NULL && currentValue2 != currentValue){
#ifdef DEBUG2
					void *res3=CAS_WRAPPER(address, currentValue,expectedValue);
					if(res3==currentValue){
						mch2->replacedBy=(void *)expectedValue;
					}
#endif
				}//end else its not a succefull assoc
				
				pointer_watch_rc_down(mch2);
				break;
			}
			
			//Now We need to RC an associated MCH of the MCAS
			void *mch3= ((void **)(other_cr))[-1];//SHould be an mch
			if(mch3 == NULL || !isMCASHelper(mch3)){ //double check :)
				pointer_watch_rc_down(mch2);
				
				currentValue=*((void * volatile *)address);
				continue;//Reprocess!
				
				
			}
			MCASHelper * mch3um=unmarkMCASHelper(mch3);
			pointer_watch_rc_up(mch3um);
            
			currentValue2=*((void * volatile *)address);
			if(currentValue != currentValue2){
				//Man we have some HORRIBLE timing!
				pointer_watch_rc_down(mch2);
				
				pointer_watch_rc_down(mch3um);
				//Now if it has changed the mcas op it reference has been freed/reusued
				//So mch2 never associated with it! So it must now longer be at the address!
				currentValue=currentValue2;
				continue;//Reprocess!
			}
			//So we have a rc with a MCH that is associated with the MCAS operrat
			//Now we can operate on it.
			//First!
			
            
			if(	shouldReplace(expectedValue, currentValue)	){
                
				if(firstTime){
					cr->mch=mch;
				}
				
				currentValue2=CAS_WRAPPER(address, currentValue, mch);
				pointer_watch_rc_down(mch2);
				
				
				if(	currentValue==currentValue2	){
					
#ifdef DEBUG2
					unmarkMCASHelper(mch)->replace=mch2;
					mch2->replacedBy=mch;
#endif
					if(!firstTime){
						currentValue=CAS_WRAPPER(&(cr->mch), NULL, mch);
						if(currentValue!=NULL && currentValue != mch){
#ifdef DEBUG2
							void *res3=CAS_WRAPPER(address, mch,expectedValue);
							if(res3==mch){
								unmarkMCASHelper(mch)->replacedBy=(void *)expectedValue;
							}
#endif
							
						}//End Association failure
						else{
							freeMCH=false;
						}
						
					}
					else{
						freeMCH=false;
					}
					
					break;
				}//End if succfulll placed mach
				else{
					if(firstTime){
						cr->mch=NULL;
					}
					currentValue=currentValue2;
					continue;
				}//end else failed to place
                
			}//End Should Replace
			pointer_watch_rc_down(mch2);
			pointer_watch_rc_down(mch3um);
			
            
            
		}//End is MCH
        
		//No match found, set to failed
		assert(nThreads != 1);
		
		if(firstTime){
			cr->mch=(void*)~(0x0);
			endRow->mch=(void*)~(0x0);
		}
		else{
			void *res2, *res1=CAS_WRAPPER(	&(cr->mch), NULL, (void*)~(0x0)	);
			if(res1==NULL)
				res2=CAS_WRAPPER(	&(endRow->mch), NULL, (void*)~(0x0)	);
		}
		//assert(freeMCH);
		
		break;
	}//End While Loop
    
	assert(cr->mch != NULL);
	
	if(freeMCH)
	{
		safe_free_mch(unmarkMCASHelper(mch));//We allocated mch so we free
		assert(cr->mch != mch);
		assert(((void **)(cr))[-1]!=mch);
	}
	
	
	return;
    
    
};



void * mcas_read_strong(void * address){
    
    
	int tries=0;
    
	void *currentValue=*((void * volatile *)address);
    
    
	while(true){
		if(tries++ == MAX_MCAS_FAILURE){
			//Need to Implement (TODO)
		}//End Tries
        
		if (!isMCASHelper(currentValue)){
			return currentValue;
		}
		else{ //if (isMCASHelper(currentValue)){
			assert(nThreads != 1);
            
			MCASHelper *mch2=unmarkMCASHelper(currentValue);
            
			pointer_watch_rc_up(mch2);
			void *currentValue2=*((void * volatile *)address);
			if(currentValue != currentValue2){
				currentValue=currentValue2;
				pointer_watch_rc_down(mch2);
				continue;
			}//end rc  check
			
			
			
			CasRow *other_cr=mch2->cr_pointer;
			CasRow *other_er=mch2->endRow;
            
			//Now We need to RC an associated MCH of the MCAS
			void *mch3= ((void **)(other_cr))[-1];//SHould be an mch
			if(mch3 == NULL || !isMCASHelper(mch3)){ //double check :)
				pointer_watch_rc_down(mch2);
				currentValue=*((void * volatile *)address);
				continue;//Reprocess!
			}
			
			
			MCASHelper * mch3um=unmarkMCASHelper(mch3);
			pointer_watch_rc_up(mch3um);
			
			currentValue2=*((void * volatile *)address);
			if(currentValue != currentValue2){
				//Man we have some HORRIBLE timing!
				pointer_watch_rc_down(mch2);
				pointer_watch_rc_down(mch3um);
				//Now if it has changed the mcas op it reference has been freed/reusued
				//So mch2 never associated with it! So it must now longer be at the address!
				currentValue=currentValue2;
				continue;//Reprocess!
			}
			//So we have a rc with a MCH that is associated with the MCAS operrat
			//Now we can operate on it.
			//First!
			
			
			void *cr_mch=CAS_WRAPPER(&(other_cr->mch), NULL, currentValue);
			
			if(		(cr_mch  != currentValue)	||
               (cr_mch  == (void *)~(0x0))	||
               (other_er->mch == (void *)~(0x0))
               )
			{
				currentValue = other_cr->expectedValue;
			}
			else{
				//SD now we know it is associated with the CAS Row.
				BOOL res=help_perform_mcas(other_cr, other_er);
                
				if(res){
					currentValue =  other_cr->newValue;
				}
				else{
					currentValue = other_cr->expectedValue;
				}
			}
			
			pointer_watch_rc_down(mch2);
			pointer_watch_rc_down(mch3um);
			return currentValue;
			
		}
		
	}//End While Loop
	
}


INLINE void helpIfNeeded(){
	
	if(tl_Delay++ != helpDelay){
		return;
	}
	else{
		tl_Delay=0;
	}
	
    
	tl_helpID=(tl_helpID+1 )%nThreads;
	MCASHelper *mch=(MCASHelper *)pendingMCASTable[tl_helpID];
	if(mch==NULL)
		return;
	
    //	printf ("Help if needed????%d %p\n", threadID, mch);
	assert(nThreads !=1);
    
	pointer_watch_rc_up(mch);
	void *res2=pendingMCASTable[tl_helpID];
	if(mch==res2){
		tl_mch=mch;
		tl_depth=0;
		CasRow *cr=mch->cr_pointer;
        
		help_perform_mcas(cr,mch->endRow);
	}
	pointer_watch_rc_down(mch);
    
}

INLINE void * CAS_WRAPPER(void * address, void *expectedValue, void *newValue){
	
	
	void *currentValue=(void *)(*(void **)address);
	if(currentValue==expectedValue){
		void *res= __sync_val_compare_and_swap((void *volatile *)address, expectedValue, newValue);
		return res;
	}
	else{
		assert(nThreads != 1);
		return currentValue;
	}
}

INLINE BOOL isMCASHelper(void * pointer){
	return  (	(	(unsigned long)(pointer)	>> WFBIT_SHIFTLEFT) & WFBIT_LSB);

}

INLINE void *markMCASHelper(void * pointer){
	return  (void *)( ( (unsigned long)pointer) | WFBIT);
}
INLINE MCASHelper * unmarkMCASHelper(void * pointer){
	return (MCASHelper *) ( ( (unsigned long)((void *)pointer)) & ~(WFBIT));
}


INLINE void pointer_watch_rc_up(MCASHelper *address){
	__sync_fetch_and_add(&(address->rc),1);
}
INLINE void pointer_watch_rc_down(MCASHelper *address){
#ifndef NDEBUG
	int res=__sync_fetch_and_add(&(address->rc),-1);
	assert(res >=0);
#else
	__sync_fetch_and_add(&(address->rc),-1);
#endif
}

inline int containsAddressInMCAS(CasRow *mcas,  void *c_address, CasRow *lastRow){
    
	while(mcas != lastRow){
		if(mcas->address==c_address){
			return true;
		}
		else{
			mcas++;
		}
	}
    
	return false;
};


BOOL call_mcas(int count, void **address, void *old_v, void *new_v, ...){
    
	const int extra_words=3;
    
	int mem_to_allocate= (count*sizeof(CasRow))+extra_words*sizeof(void *);
	void **allocatedChunk=(void **)calloc(mem_to_allocate,sizeof(void **));
	CasRow *mcas_op=(CasRow *)(&(allocatedChunk[2]));//First Word is a Next Pointer for Memory Reuse!
	
	
	//Todo Check to make sure lower 2 bits are zero!
    
	int i=0;
	mcas_op[i].address=address;
	mcas_op[i].expectedValue=old_v;
	mcas_op[i].newValue=new_v;
	mcas_op[i].mch=NULL;
    
    
	va_list arguments;
	va_start ( arguments, new_v );
	for(i=1; i<count; i++){
        
		mcas_op[i].address=va_arg ( arguments, void **);;
		mcas_op[i].expectedValue=va_arg ( arguments, void *);;
		mcas_op[i].newValue=va_arg ( arguments, void *);;
		mcas_op[i].mch=NULL;
		
	}
	va_end(arguments);
    
	//Sort the address
	for ( i = 0; i < count; i++  )
	{
		int j;
		for ( j = i+1; j<count; j++)
		{
	 		if(mcas_op[i].address > mcas_op[j].address){
				continue;
			}
			else if(mcas_op[i].address < mcas_op[j].address){
                
                
				void ** address=mcas_op[i].address;
				void * expected=mcas_op[i].expectedValue;
				void * new_value=mcas_op[i].newValue;
                
				mcas_op[i].address=mcas_op[j].address;
				mcas_op[i].expectedValue=mcas_op[j].expectedValue;
				mcas_op[i].newValue=mcas_op[j].newValue;
                
				mcas_op[j].address=address;
				mcas_op[j].expectedValue=expected;
				mcas_op[j].newValue=new_value;
                
			}
			else{
				free_wrapper(mcas_op);
				return -2;
			}
            
		}
        
	}
	mcas_op[count].address=(void **)(WFBIT);
    //	mcas_op[count].address=(void **)(0x1);
    
	//	print_mcas((CasRow*)mcas_op);
	BOOL res= perform_mcas((CasRow *)mcas_op, (CasRow *)(&(mcas_op[count-1]))	);
	//BOOL res= perform_mcas(mcas_op, &(mcas_op[count-1])	);
    //	print_mcas(mcas_op);
	// if(res){
    // 		int i;
    // 		for(i=1; i<count; i++){
    // 			if(mcas_op[i].newValue != mcas_op[i-1].newValue){
    // 				print_mcas(mcas_op);
    //
    // 				dump_trace();
    // 				break;
    // 			}
    // 		}
    // 	}
	//print_mcas((CasRow*)mcas_op);
	cleanup_mcas((CasRow*)mcas_op, res, (CasRow *)(&(mcas_op[count-1]))	);
    
	safe_free_mcas(allocatedChunk);
    
	return res;
    
}

/* This function is a modified version of call_mcas. 
 * call_mcas takes variable number of argements. I want to use a fixed number
 * of array arguments.
 */
BOOL call_mcas2(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA)
{
//    void **address, void *old_v, void *new_v){
    
	const int extra_words=3;
    
	int mem_to_allocate= (count*sizeof(CasRow))+extra_words*sizeof(void *);
	void **allocatedChunk=(void **)calloc(mem_to_allocate,sizeof(void **));
	CasRow *mcas_op=(CasRow *)(&(allocatedChunk[2]));//First Word is a Next Pointer for Memory Reuse!
	
	
	//Todo Check to make sure lower 2 bits are zero!
    
	int i=0;
	mcas_op[i].address=(void**)(addressA);
	mcas_op[i].expectedValue=(void*)(*oldA);
	mcas_op[i].newValue=(void*)(*newA);
	mcas_op[i].mch=NULL;
    
    
//	va_list arguments;
//	va_start ( arguments, new_v );
	for (i = 1; i < count; i++){
        
		mcas_op[i].address=(void**)(addressA + i);
		mcas_op[i].expectedValue=(void*)(*(oldA + i));
		mcas_op[i].newValue=(void*)(*(newA + i));
		mcas_op[i].mch=NULL;
		
	}
//	va_end(arguments);
    
//	int i=0;
//	mcas_op[i].address=address;
//	mcas_op[i].expectedValue=old_v;
//	mcas_op[i].newValue=new_v;
//	mcas_op[i].mch=NULL;
//    
//    
//	va_list arguments;
//	va_start ( arguments, new_v );
//	for(i=1; i<count; i++){
//        
//		mcas_op[i].address=va_arg ( arguments, void **);;
//		mcas_op[i].expectedValue=va_arg ( arguments, void *);;
//		mcas_op[i].newValue=va_arg ( arguments, void *);;
//		mcas_op[i].mch=NULL;
//		
//	}
//	va_end(arguments);
    
	//Sort the address
	for ( i = 0; i < count; i++  )
	{
		int j;
		for ( j = i+1; j<count; j++)
		{
	 		if(mcas_op[i].address > mcas_op[j].address){
				continue;
			}
			else if(mcas_op[i].address < mcas_op[j].address){
                
                
				void ** address=mcas_op[i].address;
				void * expected=mcas_op[i].expectedValue;
				void * new_value=mcas_op[i].newValue;
                
				mcas_op[i].address=mcas_op[j].address;
				mcas_op[i].expectedValue=mcas_op[j].expectedValue;
				mcas_op[i].newValue=mcas_op[j].newValue;
                
				mcas_op[j].address=address;
				mcas_op[j].expectedValue=expected;
				mcas_op[j].newValue=new_value;
                
			}
			else{
				free_wrapper(mcas_op);
				return -2;
			}
            
		}
        
	}
	mcas_op[count].address=(void **)(WFBIT);
    //	mcas_op[count].address=(void **)(0x1);
    
	//	print_mcas((CasRow*)mcas_op);
	BOOL res= perform_mcas((CasRow *)mcas_op, (CasRow *)(&(mcas_op[count-1]))	);
	//BOOL res= perform_mcas(mcas_op, &(mcas_op[count-1])	);
    //	print_mcas(mcas_op);
	// if(res){
    // 		int i;
    // 		for(i=1; i<count; i++){
    // 			if(mcas_op[i].newValue != mcas_op[i-1].newValue){
    // 				print_mcas(mcas_op);
    //
    // 				dump_trace();
    // 				break;
    // 			}
    // 		}
    // 	}
	//print_mcas((CasRow*)mcas_op);
	cleanup_mcas((CasRow*)mcas_op, res, (CasRow *)(&(mcas_op[count-1]))	);
    
	safe_free_mcas(allocatedChunk);
    
	return res;
    
}

/* This function is a modified version of call_mcas. 
 * call_mcas takes variable number of argements. I want to use a fixed number
 * of array arguments.
 */
BOOL call_mcas3(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA,
                uint64_t *addressB)
{
//    void **address, void *old_v, void *new_v){
    if (count == 1) {
        *addressB = __sync_val_compare_and_swap(addressA, (uint64_t)*oldA, (uint64_t)*newA);
        if (*addressB == *oldA) {
            return 1;
        } else {
            return 0;
        }
    }
    
	const int extra_words=3;
    
	int mem_to_allocate= (count*sizeof(CasRow))+extra_words*sizeof(void *);
	void **allocatedChunk=(void **)calloc(mem_to_allocate,sizeof(void **));
	CasRow *mcas_op=(CasRow *)(&(allocatedChunk[2]));//First Word is a Next Pointer for Memory Reuse!
	
	
	//Todo Check to make sure lower 2 bits are zero!
    
	int i=0;
	mcas_op[i].address=(void**)(addressA);
	mcas_op[i].expectedValue=(void*)(*oldA);
	mcas_op[i].newValue=(void*)(*newA);
	mcas_op[i].mch=NULL;
    
    
//	va_list arguments;
//	va_start ( arguments, new_v );
	for (i = 1; i < count; i++){
        
		mcas_op[i].address=(void**)(addressA + i);
		mcas_op[i].expectedValue=(void*)(*(oldA + i));
		mcas_op[i].newValue=(void*)(*(newA + i));
		mcas_op[i].mch=NULL;
		
	}
//	va_end(arguments);
    
//	int i=0;
//	mcas_op[i].address=address;
//	mcas_op[i].expectedValue=old_v;
//	mcas_op[i].newValue=new_v;
//	mcas_op[i].mch=NULL;
//    
//    
//	va_list arguments;
//	va_start ( arguments, new_v );
//	for(i=1; i<count; i++){
//        
//		mcas_op[i].address=va_arg ( arguments, void **);;
//		mcas_op[i].expectedValue=va_arg ( arguments, void *);;
//		mcas_op[i].newValue=va_arg ( arguments, void *);;
//		mcas_op[i].mch=NULL;
//		
//	}
//	va_end(arguments);
    
	//Sort the address
	for ( i = 0; i < count; i++  )
	{
		int j;
		for ( j = i+1; j<count; j++)
		{
	 		if(mcas_op[i].address > mcas_op[j].address){
				continue;
			}
			else if(mcas_op[i].address < mcas_op[j].address){
                
                
				void ** address=mcas_op[i].address;
				void * expected=mcas_op[i].expectedValue;
				void * new_value=mcas_op[i].newValue;
                
				mcas_op[i].address=mcas_op[j].address;
				mcas_op[i].expectedValue=mcas_op[j].expectedValue;
				mcas_op[i].newValue=mcas_op[j].newValue;
                
				mcas_op[j].address=address;
				mcas_op[j].expectedValue=expected;
				mcas_op[j].newValue=new_value;
                
			}
			else{
				free_wrapper(mcas_op);
				return -2;
			}
            
		}
        
	}
	mcas_op[count].address=(void **)(WFBIT);
    //	mcas_op[count].address=(void **)(0x1);
    
	//	print_mcas((CasRow*)mcas_op);
	BOOL res= perform_mcas((CasRow *)mcas_op, (CasRow *)(&(mcas_op[count-1]))	);
    
    if (res == 1) {
        memcpy(addressB, oldA, sizeof(*addressB) * count);
    } else {
        // I'd want the value at the memory.
    }
	//BOOL res= perform_mcas(mcas_op, &(mcas_op[count-1])	);
    //	print_mcas(mcas_op);
	// if(res){
    // 		int i;
    // 		for(i=1; i<count; i++){
    // 			if(mcas_op[i].newValue != mcas_op[i-1].newValue){
    // 				print_mcas(mcas_op);
    //
    // 				dump_trace();
    // 				break;
    // 			}
    // 		}
    // 	}
	//print_mcas((CasRow*)mcas_op);
	cleanup_mcas((CasRow*)mcas_op, res, (CasRow *)(&(mcas_op[count-1]))	);
    
	safe_free_mcas(allocatedChunk);
    
    
	return res;
    
}

#ifdef DEBUG

void print_recyled_mcas(){
	void **list=recycled_mcas;
	
	while(list != NULL){
		void **mcas=&(list[1]);
		print_mcas((void *)mcas);
		list=(void **)list[0];
	}
	
}

void print_mcas(void *address){
	CasRow *mcas= (CasRow *)( address);
    
	int i=0;
	printf("============Thread: %d==========================\n", threadID);
	while(mcas->address != (void *)WFBIT){
        
		printf("[%d]\t[%d] A: %p Ev: %p Nv: %p, MCH:%p\n",threadID,i , mcas->address, mcas->expectedValue, mcas->newValue, mcas->mch);
		mcas++;
		i++;
	}
    printf("======================================\n\n\n");
}


void fprint_mcas(FILE *fout, int tid, void *address){
	CasRow *mcas= (CasRow *)( address);
    
	int i=0;
	fprintf(fout, "============Thread: %d==========================\n", tid);
	while(mcas[i].address != (void *)WFBIT){
        fprintf(fout,"[%d]\t[%d] A: %p Ev: %p Nv: %p, MCH:%p ",threadID,i , mcas[i].address, mcas[i].expectedValue, mcas[i].newValue, mcas[i].mch);
        
		// if(mcas[i].mch != NULL && mcas[i].mch != (void *)(~0x0)){
		// 	MCASHelper *mch=unmarkMCASHelper((void *)mcas[i].mch);
		// 	fprintf(fout," MCH R1: %p MCH R2: %p",mch->replaced1, mch->replaced2);
		// }
		fprintf(fout,"\n");
        
        
		i++;
	}
	fprintf(fout,"======================================\n\n\n");
}


#endif

INLINE void free_wrapper(void *address){
#ifdef DONTFREE
	return;
#else
	free(address);
#endif
}

INLINE void safe_free_mch(MCASHelper *address){
    
	assert(pendingMCASTable[threadID] != address);
	assert(address != recycled_mch);
	address->next=recycled_mch;
	recycled_mch=address;
}

INLINE void safe_free_mcas(void ** address){
	CasRow *mcas=(CasRow *)(&(address[2]));
	
	BOOL freeable=is_freeable_mcas(mcas);
	BOOL wasFreed=false;
    
#ifdef DONTFREE
	freeable=false;
	
	address[0]=recycled_mcas;
	recycled_mcas=address;
	return;
#endif
	
    
	if(freeable) {
		free_mcas(address);
		wasFreed=true;
	}
	else{
		//Check Stack
		void **address_cur=recycled_mcas;
		void **address_par=NULL;
		CasRow *mcas_al;
		while(address_cur != NULL){
			mcas_al=(CasRow *)(&(address_cur[2]));
			
			void **address_next=(void **)(address_cur[0]);
			
			if( is_freeable_mcas(mcas_al)){
				
				
				if(address_par == NULL){
					recycled_mcas=address_next; //Next pointer
				}
				else{
					address_par[0]=address_next;
				}
                
				free_mcas(address_cur);
				address_cur=address_next;
			}
			else{
				address_par=address_cur;
				address_cur=address_next;
			}
			
			
		}
        
		if(!wasFreed){
			if(address_par==NULL){
				recycled_mcas=address;
			}
			else{
				address_par[0]=address;
			}
		}
        
	}
}

INLINE void free_mcas(void **address){
	CasRow *mcas=(CasRow *)(&(address[2]));
	
	if(address[1]!=NULL && address[1] != mcas[0].mch){
		safe_free_mch(unmarkMCASHelper(address[1]));
	}
    
	//Acquire address
	int i=0;
	while(mcas[i].address != (void *)WFBIT){
		void *value=mcas[i].mch;
		if(value == (void *)~(0x0)){
			break;
		}
		else if(value == (void *)(0x0)){
			//assert(false);
			break;
		}
		else{
			safe_free_mch(unmarkMCASHelper(value));
		}
        
		i++;
	}
	free_wrapper(address);
    
}
INLINE BOOL is_freeable_mcas(void *address){
	CasRow *mcas= (CasRow *)( address);
	
	BOOL freeable=true;
	
	if( ((void **)address)[-1] != NULL){
		freeable= !(	iswatched(	unmarkMCASHelper(((void **)address)[-1])	)	);
	}
    
	
	//Acquire address
	int i=0;
	while(freeable && (mcas[i].address != (void *)WFBIT)	){
		void *value=mcas[i].mch;
		if(value == (void *)~(0x0)){
			break;
		}
		else if(value == (void *)(0x0)){
			//assert(false);
			
			break;
		}
		else{
			freeable= !(	iswatched(	unmarkMCASHelper(value)	)	);
		}
        
		i++;
	}
	return freeable;
}



INLINE BOOL iswatched(MCASHelper *mch){
	//int res=__sync_fetch_and_add(&(mch->rc),0);
	int res=mch->rc;
	return (res > 0);
}
INLINE void * allocate_mcasHelpr(CasRow * cr, CasRow *endRow){
	MCASHelper *mch, *mchp=NULL;
    
    
#ifdef DONTFREE
	mch=(MCASHelper *)malloc(sizeof(MCASHelper));
	mch->rc=0;
	mch->next = NULL;
#else
    
	mch = recycled_mch;
	assert(mch ==NULL || mch != mch->next);
	while(mch != NULL && mch->rc > 0){
		mchp=mch;
		mch=mch->next;
		assert(mch!=mchp);
		
	}
	if(mch == NULL){
		mch=(MCASHelper *)malloc(sizeof(MCASHelper));
		mch->rc=0;
		mch->next = NULL;
	}
	else{
		if(mch == recycled_mch){
			assert(recycled_mch != recycled_mch->next);
			recycled_mch = recycled_mch->next;
		}
		else{
			assert(mchp->next !=mch->next);
			mchp->next=mch->next;
		}
	}
    
    
#endif	
	
	mch->cr_pointer=cr;
	mch->endRow=endRow;
	
#ifdef DEBUG2
	mch->replace=NULL;
	mch->replacedBy=NULL;
#endif
	
	assert(mch != recycled_mch);
	return markMCASHelper(mch);
	
	
}

