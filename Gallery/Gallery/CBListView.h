//
//  CBListView.h
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class CBMainWindow;

@interface CBListView : NSViewController

@property (assign) CBMainWindow* mainWindowController;
@property (retain) IBOutlet NSTableView* imagesTable;
@property (retain) IBOutlet NSArrayController* imagesArrayController;
- (IBAction) tableViewItemDoubleClicked:(id)sender;

@end
