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
//        _xfastType = [[NSMutableString alloc] init];
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
    [[self window] setContentBorderThickness:10 forEdge:NSMinYEdge];
    
    // load our nib that contains the collection view
    [self willChangeValueForKey:@"viewController"];
    viewController = [[SCCViewControllerTarget alloc] initWithNibName:@"SCCViewControllerTarget" bundle:nil];
    [self didChangeValueForKey:@"viewController"];
    
    [[self myTargetView] addSubview:[viewController view]];
    
    // make sure we resize the viewController's view to match its super view
    
    [[viewController view] setFrame:[[self myTargetView] bounds]];
    [viewController setSortingMode:0];		// ascending sort order
    [viewController setAlternateColors:NO];	// no alternate background colors (initially use gradient background)
    
    
//    [[NSNotificationCenter defaultCenter] addObserver:self
//                                             selector:@selector(contentReceived:)
//                                                 name:kReceivedContentNotification object:nil];
}

#pragma mark - Actions

- (IBAction)create:(id)sender
{
    [self setXfastType:[viewController selectedName]];
    
//    [[self window] close];
}

- (IBAction)cancel:(id)sender
{
    [[self window] close];
}

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
    NSLog(@"A:The method was called: not called");
}

- (void)controlTextDidBeginEditing:(NSNotification *)aNotification
{
    NSLog(@"B:The method was called: not called");
    [[self xfastName] setEnabled:YES];
}

// Do not allow empty node names.
- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    if ([[fieldEditor string] length] == 0)
    {
        // don't allow empty node names
        return NO;
    }
    else
    {
        return YES;
    }
}

@end
