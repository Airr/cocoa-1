//
//  main.m
//  Xfast-nstask
//
//  Created by Sang Chul Choi on 9/7/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");
    
        NSTask *buildTask = [[NSTask alloc] init];
        buildTask.launchPath = @"Xfast.app/Contents/Resources/cat.command";
        buildTask.arguments = [NSArray arrayWithObjects:
                               @"", @"\"1.txt 2.txt\"", @"o.txt", nil];
        [buildTask launch];
        
        [buildTask waitUntilExit];
    }
    return 0;
}
