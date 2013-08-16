//
//  alder_logger.h
//  alder-align
//
//  Created by Sang Chul Choi on 8/11/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef alder_align_alder_logger_h
#define alder_align_alder_logger_h

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

#define MAIN_LOGGER 0x0011
#define ERR_TEST 0x00000001

int alder_logger_initialize ();
int alder_logger_finalize ();

__END_DECLS

#endif
