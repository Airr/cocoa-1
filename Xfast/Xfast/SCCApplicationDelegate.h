//
//  SCCApplicationDelegate.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/30/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SCCPreferencesWindowController;
@interface SCCApplicationDelegate : NSObject <NSApplicationDelegate>

@property (strong) IBOutlet SCCPreferencesWindowController *preferencesController;

@end
