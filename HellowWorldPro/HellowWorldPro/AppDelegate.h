//
//  AppDelegate.h
//  HellowWorldPro
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSTextField *textField;
@property (assign) BOOL isPersonalized;
- (IBAction)changeGreeting:(id)sender;
@property (weak) IBOutlet NSButton *button;
- (void)personalize;
- (void)generalize;

@end
