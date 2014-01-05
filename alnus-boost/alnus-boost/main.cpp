//
//  main.cpp
//  alnus-boost
//
//  Created by Sang Chul Choi on 11/6/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include "cmdline.h"

using std::cout;
using std::endl;

int main(int argc, char **argv)
{
    gengetopt_args_info args_info;
    if (my_cmdline_parser (argc, argv, &args_info) != 0) exit(1) ;
    
    if (args_info.cite_flag) {
        cout << "Cite!" << endl;
    }
    
    auto x = 0.5;
    decltype(5) y;
    int* a = nullptr;
    y = 3.5;
    
    cout << x << endl;
    cout << y << endl;
    
    my_cmdline_parser_free (&args_info);
    return 0;
}

