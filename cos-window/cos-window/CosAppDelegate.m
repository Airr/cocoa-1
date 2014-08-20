//
//  CosAppDelegate.m
//  cos-window
//
//  Created by Sang Chul Choi on 7/10/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "CosAppDelegate.h"

@implementation CosAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    
}

- (IBAction)center:(id)sender {
    [[NSApp mainWindow] center];
    [[NSApp mainWindow] setTitle:@"Centered"];
    NSLog(@"Center");
}

@end
