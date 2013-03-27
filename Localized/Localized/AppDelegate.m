//
//  AppDelegate.m
//  Localized
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    NSString* currentLanguage = [[[NSUserDefaults standardUserDefaults]
                                  objectForKey:@"AppleLanguages"] objectAtIndex:0];
    NSString* labelFormat = NSLocalizedString(@"main language: %@", nil);
    self.languagesLabel.stringValue = [NSString stringWithFormat:labelFormat,
                                      currentLanguage];
}

@end
