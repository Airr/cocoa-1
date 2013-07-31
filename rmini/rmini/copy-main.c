//
//  main.c
//  rmini
//
//  Created by Sang Chul Choi on 7/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <R.h>
#include <Rdefines.h>
#include <dlfcn.h>

int main(int argc, const char * argv[])
{
    // Function signature: SEXP ksmooth_test(SEXP skrn, SEXP sbw)
    // Create an integer and a real.
    // Load the dynamic library called stats.so to call ksmooth_test.
    // Pass it to ksmooth_test.
    int skrn = 1;
    //    double sbw = 1.0;
    
    
    
    // Create an SEXP integer.
    SEXP skrnR;
    int *pRskrn;
    PROTECT(skrnR = NEW_INTEGER(1));
    pRskrn = INTEGER_POINTER(skrnR);
    pRskrn[0] = skrn;
    UNPROTECT(1);
    
    ////////////////////////////////////////////////////////////////////////////
    //
    char* lib_name = "/Users/goshng/Downloads/R-3.0.1/b/library/stats/libs/stats.so";
    void* lib_handle = dlopen(lib_name, RTLD_NOW);
    if (lib_handle) {
        printf("[%s] dlopen(\"%s\", RTLD_NOW): Successful\n", __FILE__, lib_name);
    }
    else {
        printf("[%s] Unable to open library: %s\n",
               __FILE__, dlerror());
        exit(EXIT_FAILURE);
    }
    
    // Get the symbol addresses.
    void (*add)(int) = dlsym(lib_handle, "ksmooth_test");
    if (add) {
        printf("[%s] dlsym(lib_handle, \"ksmooth_test\"): Successful\n", __FILE__);
    }
    else {
        printf("[%s] Unable to get symbol: %s\n",
               __FILE__, dlerror());
        exit(EXIT_FAILURE);
    }
    
    // Use the library.
    ksmooth_test(skrnR);
    
    // Close the library.
    if (dlclose(lib_handle) == 0) {
        printf("[%s] dlclose(lib_handle): Successful\n", __FILE__);
    }
    else {
        printf("[%s] Unable to open close: %s\n",
               __FILE__, dlerror());
    }
    
    printf("[end_test]\n\n");
    
    return 0;
}

