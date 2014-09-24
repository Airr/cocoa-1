//
//  SCCContentTextViewController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/30/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCContentTextViewController : NSViewController

- (void)changeText:(NSString *)text;

- (void)changeText:(NSString *)text withPath:(NSString *)path;

@end
