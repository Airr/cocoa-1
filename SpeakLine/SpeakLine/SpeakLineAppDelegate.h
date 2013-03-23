//
//  SpeakLineAppDelegate.h
//  SpeakLine
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SpeakLineAppDelegate : NSObject <NSApplicationDelegate, NSSpeechSynthesizerDelegate>
{
    NSArray* _voices;
    NSSpeechSynthesizer* _speechSynth;
}

@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSTextField *textField;
@property (weak) IBOutlet NSButton *stopButton;
@property (weak) IBOutlet NSButton *speakButton;
@property (weak) IBOutlet NSTableView *tableView;

- (IBAction)stopIt:(id)sender;
- (IBAction)sayIt:(id)sender;

@end
