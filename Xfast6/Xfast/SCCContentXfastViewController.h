//
//  SCCContentXfastViewController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/29/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCContentXfastViewController : NSViewController
<NSTableViewDataSource>

- (void)changeSettings:(NSMutableDictionary *)settings;

@end
