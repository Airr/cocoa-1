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

#define __MAIN__
#define R_USE_SIGNALS 1
#include "Defn.h"
#include <Internal.h>
#include "Rinterface.h"
//#include <R.h>
//#include <Rinternals.h>

int main(int argc, const char * argv[])
{
    int skrn = 1;
    
    
    return skrn;
}

void setup_Rmainloop(void)
{
    InitNames();
}