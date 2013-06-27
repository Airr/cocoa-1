//
//  AppController.m
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "AppController.h"
#import "PreferenceController.h"

@implementation AppController

- (IBAction)showPrefreencePanel:(id)sender
{
    if (!preferenceController) {
        preferenceController = [[PreferenceController alloc] init];
    }
    NSLog(@"showing %@", preferenceController);
    [preferenceController showWindow:self];
}

+ (void)initialize
{
    // Create a dictionary
    NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
    // Archive the color object
    NSData *colorAsData = [NSKeyedArchiver archivedDataWithRootObject:
                           [NSColor yellowColor]];
    // Put defaults in the dictionary
    [defaultValues setObject:colorAsData forKey:BNRTableBgColorKey];
    [defaultValues setObject:[NSNumber numberWithBool:YES] forKey:BNREmptyDocKey];
    // Register the dictionary of defaults
    [[NSUserDefaults standardUserDefaults] registerDefaults: defaultValues];
    NSLog(@"registered defaults: %@", defaultValues);
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
    NSLog(@"applicationShouldOpenUntitledFile:");
    return [PreferenceController preferenceEmptyDoc];
}

@end
