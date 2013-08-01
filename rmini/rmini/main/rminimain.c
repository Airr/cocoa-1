//
//  main.c
//  rmini
//
//  Created by Sang Chul Choi on 7/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h> /* avoid redefinition of extern in Defn.h */
#include <float.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//#define __MAIN__
#define R_USE_SIGNALS 1
#include "Defn.h"
#include <Internal.h>
#include "Rinterface.h"
//#include <R.h>
//#include <Rinternals.h>

int main(int argc, const char * argv[])
{
    int skrn = 1;
    SEXP sexpInt;
//    SEXP sexpStr;
//    SEXP sexpChar;
    int carSpeed[50] = {4,4,7,7,8,9,10,10,10,11,11,12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,16,16,17,17,17,18,18,18,18,19,19,19,20,20,20,20,20,22,23,24,24,24,24,25};
    int carDist[50] = {2,10,4,22,16,10,18,26,34,17,28,14,20,24,28,26,34,34,46,26,36,60,80,20,26,54,32,40,32,40,50,42,56,76,84,36,46,68,32,48,52,56,64,66,54,70,92,93,120,85};
    SEXP sexpCarSpeed, sexpCarDist;
    
    setup_Rmainloop();
    
    printf("Hello ... from rmini!\n");
    
//    sexpCarSpeed = allocVector(INTSXP,50);
//    INTEGER(sexpInt)[0] = 123;
    
    // asInteger
    
//    sexpStr = allocVector(STRSXP, 5);
//    SET_STRING_ELT(sexpStr, 0, mkChar("One"));
//    SET_STRING_ELT(sexpStr, 1, mkChar("Two"));
//    sexpChar = STRING_ELT(sexpStr, 0);
//    const char *v = CHAR(sexpChar);
    
    
    
    freeVector(sexpInt);
//    freeVector(sexpStr);
    
    finalize_Rmainloop();
    return skrn;
}
