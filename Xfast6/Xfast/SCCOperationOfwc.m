//
//  SCCOperationOfwc.m
//  Xfast
//
//  Created by Sang Chul Choi on 9/2/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCOperationOfwc.h"
#import "SCCCommand.h"


@interface SCCOperationOfwc ()

@property id<SCCCommand> command;

/**
 * NSTask
 */
@property (nonatomic, strong) __block NSTask *buildTask;

@end



@implementation SCCOperationOfwc

- (id)initWithCommand:(id<SCCCommand>)command
{
    self = [super init];
    if (self) {
        _command = command;
    }
    return self;
}


//        NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:
//                              [self.loadURL lastPathComponent], kNameKey,
//                              [self.loadURL absoluteString], kPathKey,
//                              modDateStr, kModifiedKey,
//                              [NSString stringWithFormat:@"%ld", [fileSize integerValue]], kSizeKey,
//                              [NSNumber numberWithInteger:ourScanCount], kScanCountKey,  // pass back to check if user cancelled/started a new scan
//                              nil];
- (void)main
{
    if (![self isCancelled])
    {
        @try {
            for (NSArray *arguments in [_command arrayOfArguments]) {
                
                self.buildTask            = [[NSTask alloc] init];
                self.buildTask.launchPath = [_command launchPath];
                
                self.buildTask.arguments  = arguments;
                
                [self.buildTask launch];
                
                [self.buildTask waitUntilExit];
                self.buildTask = nil;
            }
        }
        @catch (NSException *exception) {
            NSLog(@"Problem Running Task: %@", [exception description]);
        }
        @finally {
        }
        
        NSDictionary *info = nil;
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
