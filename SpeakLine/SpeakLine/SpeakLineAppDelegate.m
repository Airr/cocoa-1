//
//  SpeakLineAppDelegate.m
//  SpeakLine
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpeakLineAppDelegate.h"

@implementation SpeakLineAppDelegate

@synthesize window = _window;
@synthesize textField = _textField;

- (id)init
{
    self = [super init];
    if (self) {
        _speechSynth = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
    }
//    if (self)
//    {
//        NSLog(@"init");
//        _speechSynth = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
//        _speechSynth = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
//        ￼
//    }
    return self;
}



- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
}

- (IBAction)stopIt:(id)sender {
    NSLog(@"stopping");
    [_speechSynth stopSpeaking];
}

- (IBAction)sayIt:(id)sender {
    NSString *string = [_textField stringValue];
    // Is the string zero-length?
    if ([string length] == 0) {
        NSLog(@"string from %@ is of zero-length", _textField);
        return; }
    [_speechSynth startSpeakingString:string];
    NSLog(@"Have started to say: %@", string);
}
@end
