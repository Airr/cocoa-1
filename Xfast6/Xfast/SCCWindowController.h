//
//  SCCWindowController.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/18/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class XCProject;

@interface SCCWindowController : NSWindowController <NSOutlineViewDelegate, NSOutlineViewDataSource>

- (id)initWithProject:(XCProject *)project;

#pragma mark Buttons

- (IBAction)addFiles:(id)sender;

- (IBAction)removeFiles:(id)sender;

/*
 * Adds a new target to the Xfast Project.
 */
- (IBAction)addTarget:(id)sender;

- (IBAction)submitTarget:(id)sender;

#pragma mark - Sheet of Add Xfast

/**
 * Adds a new Xfast to the current project.
 */
- (IBAction)addXfast:(id)sender;

//- (IBAction)finishAddXfast:(id)sender;

#pragma mark - Xfast

- (void)addXfastFile:(NSString *)xfastName;

@end
