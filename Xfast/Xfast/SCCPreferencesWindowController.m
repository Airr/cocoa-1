//
//  SCCPreferencesWindowController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/12/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCPreferencesWindowController.h"

@interface SCCPreferencesWindowController ()

@end

@implementation SCCPreferencesWindowController

- (id)init {
    return [super initWithWindowNibName:@"SCCPreferencesWindowController"];
}

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
    NSWindow *window = [self window];
    [window setHidesOnDeactivate:NO];
    [window setExcludedFromWindowsMenu:YES];
    [window setIdentifier:@"SCCPreferencesWindowController"];
    [window setRestorationClass:[self class]];
}

@end
