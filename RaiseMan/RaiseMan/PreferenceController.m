//
//  PreferenceController.m
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "PreferenceController.h"

NSString * const BNRTableBgColorKey = @"BNRTableBackgroundColor";
NSString * const BNREmptyDocKey = @"BNREmptyDocumentFlag";
NSString * const BNRColorChangedNotification = @"BNRColorChanged";

@interface PreferenceController ()

@end

@implementation PreferenceController

- (id)init
{
    self = [super initWithWindowNibName:@"PreferenceController"];
    return self;
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
    [colorWell setColor:[PreferenceController preferenceTableBgColor]];
    [checkbox setState:[PreferenceController preferenceEmptyDoc]];
//    NSLog(@"Preferences Nib file is loaded");
}

- (IBAction)changeBackgroundColor:(id)sender
{
    NSColor *color = [colorWell color];
    [PreferenceController setPreferenceTableBgColor:color];
    
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    NSLog(@"Sending notification");
//    [nc postNotificationName:BNRColorChangedNotification object:self];
    
    NSDictionary *d = [NSDictionary dictionaryWithObject:color
                                                  forKey:@"color"];
    [nc postNotificationName:BNRColorChangedNotification
                      object:self
                    userInfo:d];
//    NSLog(@"Color changed: %@", color);
}

- (IBAction)changeNewEmptyDoc:(id)sender
{
    NSInteger state = [checkbox state];
    [PreferenceController setPreferenceEmptyDoc:state];
//    NSLog(@"Checkbox changed %ld", state);
}


+ (NSColor *)preferenceTableBgColor
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults]; NSData *colorAsData = [defaults objectForKey:BNRTableBgColorKey]; return [NSKeyedUnarchiver unarchiveObjectWithData:colorAsData];
}

+ (void)setPreferenceTableBgColor:(NSColor *)color
{
    NSData *colorAsData =
    [NSKeyedArchiver archivedDataWithRootObject:color];
    [[NSUserDefaults standardUserDefaults] setObject:colorAsData
                                              forKey:BNRTableBgColorKey];
}

+ (BOOL)preferenceEmptyDoc
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    return [defaults boolForKey:BNREmptyDocKey];
}

+ (void)setPreferenceEmptyDoc:(BOOL)emptyDoc
{
    [[NSUserDefaults standardUserDefaults] setBool:emptyDoc
                                            forKey:BNREmptyDocKey];
}


@end
