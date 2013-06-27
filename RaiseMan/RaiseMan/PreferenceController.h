//
//  PreferenceController.h
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString * const BNRTableBgColorKey;
extern NSString * const BNREmptyDocKey;
extern NSString * const BNRColorChangedNotification;

@interface PreferenceController : NSWindowController
{
    IBOutlet NSColorWell *colorWell;
    IBOutlet NSButton *checkbox;
}

+ (NSColor *)preferenceTableBgColor;
+ (void)setPreferenceTableBgColor:(NSColor *)color;
+ (BOOL)preferenceEmptyDoc;
+ (void)setPreferenceEmptyDoc:(BOOL)emptyDoc;

- (IBAction)changeBackgroundColor:(id)sender;
- (IBAction)changeNewEmptyDoc:(id)sender;
@end
