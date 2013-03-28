//
//  AppDelegate.h
//  Counter
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class Display;
@class Logger;
@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property IBOutlet Display *display;
@property IBOutlet Logger *logger;

@end
