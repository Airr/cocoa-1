//
//  AppDelegate.m
//  HellowWorldPro
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    [self generalize];
}

- (void)personalize {
    self.window.backgroundColor = [NSColor redColor];
    [self.textField setStringValue:[NSString stringWithFormat:@"Hello, %@!", NSFullUserName()]];
    [self.button setTitle:@"Generalize"];
    self.isPersonalized = NO;
}

- (void)generalize {
    self.window.backgroundColor = [NSColor greenColor];
    [self.textField setStringValue:@"Hello, World!"];
    [self.button setTitle:@"Personalize"];
    self.isPersonalized = NO;
    
}
- (IBAction)changeGreeting:(id)sender {
//    NSLog(@"%@",[[self.button title] lowercaseString]);
    [self performSelector:NSSelectorFromString([[self.button title] lowercaseString])];
//    if (self.isPersonalized) {
//        [self generalize];
//    } else {
//        [self personalize];
//    }
}
@end
