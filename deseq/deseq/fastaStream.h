//
//  fastaStream.h
//  deseq
//
//  Created by Sang Chul Choi on 7/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef deseq_fastaStream_h
#define deseq_fastaStream_h

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

////////////////////////////////////////////////////////////////////////////////
// Usage
//    struct fastaFormat fsm;
//
//    int main(int argc, const char * argv[])
//    {
//        int isNext = 1;
//        FILE *fin = fopen("/Users/goshng/Library/Developer/Xcode/DerivedData/deseq-hcobcucskulqnzgasrjxhitcryvo/Build/Products/Debug/test.mRNA.fa", "r");
//        fastaStreamInitialize( &fsm, fin );
//        while ( isNext == 1 ) {
//            isNext = fastaStreamNext( &fsm, fin );
//            printf("%s\n", fsm.buf);
//        }
//        fastaStreamFinalize( &fsm );
//        return 0;
//    }

struct fastaFormat
{
	char *buf;
	unsigned int buflen;
};

int fastaStreamInitialize( struct fastaFormat *fsm, FILE *fin );
int fastaStreamNext( struct fastaFormat *fsm, FILE *fin );
int fastaStreamFinalize( struct fastaFormat *fsm );

__END_DECLS

#endif
