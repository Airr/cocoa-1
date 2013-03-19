//
//  main.m
//  ClassCluster
//
//  Created by Sang Chul Choi on 3/19/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Pair.h"

int main(int argc, const char * argv[])
{
    
    @autoreleasepool {
        
        Pair *floats = [[Pair alloc] initWithFloat:0.5 float:12.42];
        Pair *ints= [[Pair alloc] initWithInt:1984 int:2001];
        NSLog(@"Two floats: %@", floats);
        NSLog(@"Two ints: %@", ints);
        
    }
    return 0;
}

