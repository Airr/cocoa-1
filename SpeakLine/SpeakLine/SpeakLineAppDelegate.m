//
//  SpeakLineAppDelegate.m
//  SpeakLine
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SpeakLineAppDelegate.h"

@implementation SpeakLineAppDelegate

@synthesize window = _window;
@synthesize textField = _textField;
@synthesize tableView = _tableView;
- (id)init
{
    self = [super init];
    if (self) {
        _speechSynth = [[NSSpeechSynthesizer alloc] initWithVoice:nil];
        [_speechSynth setDelegate:self];
        _voices = [NSSpeechSynthesizer availableVoices];
    }
    return self;
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender
        didFinishSpeaking:(BOOL)finishedSpeaking
{
    NSLog(@"finishedSpeaking = %d", finishedSpeaking);
    [_stopButton setEnabled:NO];
    [_speakButton setEnabled:YES];
    [_tableView setEnabled:YES];
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
    [_stopButton setEnabled:YES];
    [_speakButton setEnabled:NO];
    [_tableView setEnabled:NO];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
    return (NSInteger)[_voices count];
}

- (id)tableView:(NSTableView *)tv
objectValueForTableColumn:(NSTableColumn *)tableColumn
            row:(NSInteger)row
{
    NSString *v = [_voices objectAtIndex:row];
    NSDictionary *dict = [NSSpeechSynthesizer attributesForVoice:v];
    return [dict objectForKey:NSVoiceName];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    NSInteger row = [_tableView selectedRow];
    if (row == -1) {
        return;
    }
    NSString *selectedVoice = [_voices objectAtIndex:row];
    [_speechSynth setVoice:selectedVoice];
    NSLog(@"new voice = %@", selectedVoice);
}

- (void)awakeFromNib
{
    // When the table view appears on screen, the default voice
    // should be selected
    NSString *defaultVoice = [NSSpeechSynthesizer defaultVoice];
    NSInteger defaultRow = [_voices indexOfObject:defaultVoice];
    NSIndexSet *indices = [NSIndexSet indexSetWithIndex:defaultRow];
    [_tableView selectRowIndexes:indices byExtendingSelection:NO];
    [_tableView scrollRowToVisible:defaultRow];
}
@end
