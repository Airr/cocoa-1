//
//  SCCAddFilesWindowController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/9/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class XFProject;

@interface SCCAddFilesWindowController : NSWindowController <NSOutlineViewDelegate, NSOutlineViewDataSource>

@property (nonatomic) XFProject *xfastProject;

- (IBAction)add:(id)sender;
- (IBAction)cancel:(id)sender;

@end
