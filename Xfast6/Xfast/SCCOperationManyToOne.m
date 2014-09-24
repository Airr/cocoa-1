//
//  SCCOperationManyToOne.m
//  Xfast
//
//  Created by Sang Chul Choi on 9/4/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCOperationManyToOne.h"

@interface SCCOperationManyToOne ()

@property id<SCCCommand> command;

@property (nonatomic, strong) __block NSTask *buildTask;

@end



@implementation SCCOperationManyToOne

- (id)initWithCommand:(id<SCCCommand>)command
{
    self = [super init];
    if (self) {
        _command = command;
    }
    return self;
}


- (void)main
{
    if (![self isCancelled])
    {
        @try {
            self.buildTask            = [[NSTask alloc] init];
            self.buildTask.launchPath = [_command launchPath];
            
            self.buildTask.arguments  = [_command arguments];
            
            [self.buildTask launch];
            
            
            [self.buildTask waitUntilExit];
            self.buildTask = nil;
        }
        @catch (NSException *exception) {
            NSLog(@"Problem Running Task: %@", [exception description]);
        }
        @finally {
        }
        
        NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:
                              @"aaa", @"project",
                              @"bbb", @"xfast",
                              @"ccc", @"target",
                              @"start", @"start",
                              @"end", @"end",
                              @"started", @"status",
                              nil];
        if (![self isCancelled])
        {
            // for the purposes of this sample, we're just going to post the information
            // out there and let whoever might be interested receive it (in our case its MyWindowController).
            //
            [[NSNotificationCenter defaultCenter] postNotificationName:kJobDidFinish object:nil userInfo:info];
        }
        
    }
}

@end