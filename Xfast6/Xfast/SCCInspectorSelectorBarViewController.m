//
//  SCCInspectorSelectorBarViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/25/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCInspectorSelectorBarViewController.h"

@interface SCCInspectorSelectorBarViewController ()

@property (weak) IBOutlet NSButton *file;
@property (weak) IBOutlet NSButton *help;
@property (weak) IBOutlet NSButton *identity;
@property (weak) IBOutlet NSButton *attribute;

@end

@implementation SCCInspectorSelectorBarViewController

- (void)loadView {
    [super loadView];
    [_file setEnabled:NO];
}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCInspectorSelectorBarViewController" bundle:nil];
    if (self) {
    }
    return self;
}

- (IBAction)showFile:(id)sender {
    if ([[self file] state] == NSOnState) {
        [self willChangeValueForKey:@"file"];
        [[self file] setState:NSOnState];
        [[self help] setState:NSOffState];
        [[self identity] setState:NSOffState];
        [[self attribute] setState:NSOffState];
        
        [[self help] setEnabled:YES];
        [[self identity] setEnabled:YES];
        [[self attribute] setEnabled:YES];
        [self didChangeValueForKey:@"file"];
    } else {
        [[self file] setState:NSOnState];
        
    }
    [[self file] setEnabled:NO];
}

- (IBAction)showHelp:(id)sender {
    if ([[self help] state] == NSOnState) {
        [self willChangeValueForKey:@"help"];
        [[self file] setState:NSOffState];
        [[self help] setState:NSOnState];
        [[self identity] setState:NSOffState];
        [[self attribute] setState:NSOffState];
        
        [[self file] setEnabled:YES];
        [[self identity] setEnabled:YES];
        [[self attribute] setEnabled:YES];
        [self didChangeValueForKey:@"help"];
    } else {
        [[self help] setState:NSOnState];
    }
    [[self help] setEnabled:NO];
}

- (IBAction)showIdentity:(id)sender {
    if ([[self identity] state] == NSOnState) {
        [self willChangeValueForKey:@"identity"];
        [[self file] setState:NSOffState];
        [[self help] setState:NSOffState];
        [[self identity] setState:NSOnState];
        [[self attribute] setState:NSOffState];
        
        [[self file] setEnabled:YES];
        [[self help] setEnabled:YES];
        [[self attribute] setEnabled:YES];
        [self didChangeValueForKey:@"identity"];
    } else {
        [[self identity] setState:NSOnState];
        
    }
    [[self identity] setEnabled:NO];
}
- (IBAction)showAttribute:(id)sender {
    if ([[self attribute] state] == NSOnState) {
        [self willChangeValueForKey:@"attribute"];
        [[self file] setState:NSOffState];
        [[self help] setState:NSOffState];
        [[self identity] setState:NSOffState];
        [[self attribute] setState:NSOnState];
        
        [[self file] setEnabled:YES];
        [[self help] setEnabled:YES];
        [[self identity] setEnabled:YES];
        [self didChangeValueForKey:@"attribute"];
    } else {
        [[self attribute] setState:NSOnState];
        
    }
    [[self attribute] setEnabled:NO];
}

@end