//
//  main.c
//  alder-kmer-hex256
//
//  Created by Sang Chul Choi on 10/12/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>

int retired_main(int argc, const char * argv[])
{
    //    for (uint8_t i = 1; i != 0; i++) {
    //        printf("%x, ",i);
    //    }
    
    printf("static uint8_t alder_bit_four2uc_matrix[4][4] = {\n");
    uint8_t x = 0x00; // 0000 0000
    printf("{");
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X", x);
    printf("},\n");
    
    x = 0x40; // 0100 0000
    printf("{");
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X", x);
    printf("},\n");
    
    x = 0x80; // 1000 0000
    printf("{");
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X", x);
    printf("},\n");
    
    x = 0xC0; // 1100 0000
    printf("{");
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X,", x); x >>= 2;
    printf("0x%02X", x);
    printf("},\n");
    printf("};\n");
    
    
    printf("static uint64_t alder_bit_kmer2number_matrix[32][4] = {\n");
    for (uint64_t i = 0; i < 32; i++) {
        printf("{");
        for (uint64_t j = 0; j < 4; j++) {
            uint64_t x = 1UL << (2*i);
            x *= j;
            printf("0x%016llX", x);
            if (j < 3) {
                printf(", ");
            }
        }
        printf("}");
        if (i < 31) {
            printf(",\n");
        }
    }
    printf("\n};\n");
    
    printf("static uint64_t alder_bit_1_array[64] = {\n");
    uint64_t x64 = 0x0000000000000001;
    for (uint64_t i = 0; i < 64; i++) {
        printf("0x%016llX", x64);
        x64 <<= 1;
        if (i < 63) {
            printf(",\n");
        }
    }
    printf("\n};\n");
    
    printf("static uint64_t alder_bit_right_1s_array[65] = {\n");
    x64 = 0x0000000000000000;
    printf("0x%016llX,\n", x64);
    x64 = 0x0000000000000001;
    for (uint64_t i = 0; i < 64; i++) {
        printf("0x%016llX", x64);
        x64 = (x64 << 1) | x64;
        if (i < 63) {
            printf(",\n");
        }
    }
    printf("\n};\n");
    return 0;
}


