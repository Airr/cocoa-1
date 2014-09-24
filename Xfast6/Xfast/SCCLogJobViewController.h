//
//  SCCLogJobViewController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/31/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCLogJobViewController : NSViewController

- (void)addData;

- (void)addEntryWithProject:(NSString *)project
                  withXfast:(NSString *)xfast
                 withTarget:(NSString *)target
                  withStart:(NSString *)start
                    withEnd:(NSString *)end
                 withStatus:(NSString *)status;

- (void)removeEntryWithProject:(NSString *)project
                     withXfast:(NSString *)xfast
                    withTarget:(NSString *)target
                     withStart:(NSString *)start
                       withEnd:(NSString *)end
                    withStatus:(NSString *)status;


@end
