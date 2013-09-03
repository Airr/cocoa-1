//
//  main.c
//  brass-ds-trie-list
//
//  Created by Sang Chul Choi on 9/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "alder_dictionary.h"

#define WORDSIZE 1000
void test_trie_tree_list()
{
    
    char randomWord[WORDSIZE + 1];
    randomWord[WORDSIZE] = '\0';
    alder_dictionary_t *dic = alder_dictionary_initialize();
    
    for (size_t i = 0; i < 100000; i++) {
        for (size_t j = 0; j < WORDSIZE; j++) {
            int k = rand();
            // 33-126: 126 - 32 = 94
            k = k%95+33;
            randomWord[j] = (char)k;
        }
        int64_t s = alder_dictionary_add(dic, randomWord);
        assert(s > 0);
    }
    alder_dictionary_finalize(dic);
}

int main(int argc, const char * argv[])
{
    for (size_t i = 0; i < 10; i++) {
        test_trie_tree_list();
    }
    return 0;
}

