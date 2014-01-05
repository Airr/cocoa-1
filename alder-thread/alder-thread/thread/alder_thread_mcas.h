/**
 * This file, alder_thread_mcas.h, is part of alder-thread.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-thread is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-thread is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-thread.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef alder_thread_alder_thread_mcas_h
#define alder_thread_alder_thread_mcas_h

#include <stdint.h>
#include <stdbool.h>

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

#define ALDER_THREAD_MCAS_DESCRIPTOR      0xC000000000000000
#define ALDER_THREAD_MCAS_DESCRIPTOR_MCAS 0x8000000000000000
#define ALDER_THREAD_MCAS_DESCRIPTOR_DCSS 0x4000000000000000
#define ALDER_THREAD_MCAS_MAKE_DESCRIPTOR 0x3FFFFFFFFFFFFFFF

enum {
    ALDER_THREAD_MCAS_SUCCEEDED,
    ALDER_THREAD_MCAS_FAILED,
    ALDER_THREAD_MCAS_UNDECIDED
};

typedef struct alder_thread_mcas_rdcss_struct {
    uint64_t *a1;
    uint64_t o1;
    uint64_t *a2;
    uint64_t o2;
    uint64_t n2;
} alder_thread_mcas_rdcss_t;

typedef struct alder_thread_mcas_struct {
    uint64_t status;
    uint64_t s;
    uint64_t *addr;
    uint64_t *old;
    uint64_t *n;
    alder_thread_mcas_rdcss_t *rdcss;
} alder_thread_mcas_t;


int alder_thread_mcas();
bool alder_thread_mcas_MCAS(alder_thread_mcas_t *cd);
uint64_t alder_thread_mcas_MCASRead(uint64_t *addr);
uint64_t alder_thread_mcas_RDCSS(alder_thread_mcas_rdcss_t *d);
uint64_t alder_thread_mcas_RDCSSRead(uint64_t *addr);


__END_DECLS


#endif /* alder_thread_alder_thread_mcas_h */
