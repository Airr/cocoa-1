//
//  SCCTargetWindowController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/14/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCTargetWindowController.h"

@interface SCCTargetWindowController ()

@property (weak) IBOutlet NSView *myTargetView;
@property (weak) IBOutlet NSTextField *selectionField;



@end

@implementation SCCTargetWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}


// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
    [[self window] setAutorecalculatesContentBorderThickness:YES forEdge:NSMinYEdge];
    [[self window] setContentBorderThickness:30 forEdge:NSMinYEdge];
    
    // load our nib that contains the collection view
    [self willChangeValueForKey:@"viewController"];
    viewController = [[SCCViewControllerTarget alloc] initWithNibName:@"SCCViewControllerTarget" bundle:nil];
    [self didChangeValueForKey:@"viewController"];
    
    [[self myTargetView] addSubview:[viewController view]];
    
    // make sure we resize the viewController's view to match its super view
    
    [[viewController view] setFrame:[[self myTargetView] bounds]];
    [viewController setSortingMode:0];		// ascending sort order
    [viewController setAlternateColors:NO];	// no alternate background colors (initially use gradient background)
    
}

#pragma mark - Actions

- (IBAction)create:(id)sender
{
    [self setXfastName:[viewController selectedName]];
    [[self window] close];
}

- (IBAction)cancel:(id)sender
{
    [[self window] close];
}

@end
