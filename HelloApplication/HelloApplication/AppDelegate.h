//
//  AppDelegate.h
//  HelloApplication
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class ActivityController;
@class CurrentApp;

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet ActivityController *ac;
@property (retain) CurrentApp *currentApp;

@end
