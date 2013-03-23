//
//  SpeakLineAppDelegate.h
//  SpeakLine
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SpeakLineAppDelegate : NSObject <NSApplicationDelegate>
{
    NSSpeechSynthesizer* _speechSynth;
}

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSTextField *textField;

- (IBAction)stopIt:(id)sender;
- (IBAction)sayIt:(id)sender;

@end
