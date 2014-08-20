//
//  SCCViewControllerModule.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/21/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SCCAddFilesWindowController.h"
@class XFProject;

@interface SCCViewControllerModule : NSViewController <NSOutlineViewDelegate,NSTableViewDelegate,NSTableViewDataSource>

@property (weak) IBOutlet NSOutlineView *configurationCollection;

@property (weak) IBOutlet NSTextField *version;
@property (weak) IBOutlet NSTextField *build;
@property (weak) IBOutlet NSTableView *fileTable;
@property (readonly) NSArray *configurations;
@property SCCAddFilesWindowController *addFilesWindowController;

@property (nonatomic) XFProject* xfastProject;

- (id)initWithXFProject:(XFProject *)xfastProject;


#pragma mark Buttons

- (IBAction)addFiles:(id)sender;

- (IBAction)removeFiles:(id)sender;

@end
