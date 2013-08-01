//
//  main_dummy.c
//  rmini
//
//  Created by Sang Chul Choi on 7/31/13.
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
#include <assert.h>

#define __MAIN__
#define R_USE_SIGNALS 1
#include "Defn.h"
#include <Internal.h>
#include "Rinterface.h"

void attribute_hidden BindDomain(char *R_Home)
{
    assert(0);
    return;
}