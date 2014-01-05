//
//  wf_mcas_2_0_1.c
//  MCAS_V2
//  Created by Steven Feldman on 6/23/13.
//  Copyright (c) 2013 Steven FELDMAN. All rights reserved.
//

#ifndef alder_mcas_alder_mcas_wf_h
#define alder_mcas_alder_mcas_wf_h

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

#define BOOL int
#define true 1
#define false 0

// To use MSB
#define WFBIT 0x8000000000000000
#define WFBIT_LSB 0x0000000000000001
#define WFBIT_SHIFTLEFT 63
// To use LSB
//#define WFBIT 0x1
//#define WFBIT_LSB 0x1
//#define WFBIT_SHIFTLEFT 0

void mcas_init(int threads);
BOOL call_mcas(int count, void **address, void *old_v, void *new_v, ...);
BOOL call_mcas2(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA);
BOOL call_mcas3(int count, uint64_t *addressA, uint64_t *oldA, uint64_t *newA,
                uint64_t *addressB);
void * mcas_read_strong(void * address);

__END_DECLS


#endif /* alder_mcas_alder_mcas_wf_h */
