//
//  AppDelegate.m
//  BookShelf
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "AppDelegate.h"
#import "BookShelfViewController.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    BookShelfViewController * bc = [[BookShelfViewController alloc]
                                initWithNibName:@"Bookshelf" bundle:nil];
    [self.window setContentSize:[bc.view bounds].size];
    self.window.contentView = bc.view;
}

@end
