//
//  AlderAppDelegate.h
//  alderg-kmer
//
//  Created by Sang Chul Choi on 1/2/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AlderAppDelegate : NSObject <NSApplicationDelegate>
{
    NSTask *unixTask;
    NSPipe *unixStandardOutputPipe;
    NSPipe *unixStandardErrorPipe;
    NSPipe *unixStandardInputPipe;
    NSFileHandle *fhOutput;
    NSFileHandle *fhError;
    NSData *standardOutputData;
    NSData *standardErrorData;
    NSString *runScriptPath;
    NSString *cmdScriptPath;
}
@property (assign) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSStepper *kmerSizeStepper;
@property (weak) IBOutlet NSTextField *kmerSize;
@property (weak) IBOutlet NSTextField *outfile;
@property (weak) IBOutlet NSProgressIndicator *unixProgressBar;
@property (unsafe_unretained) IBOutlet NSTextView *unixTaskStdOutput;
@property (weak) IBOutlet NSTextField *outdir;
@property (weak) IBOutlet NSTextField *availableDisk;
@property (weak) IBOutlet NSTextField *availableMemory;
@property (weak) IBOutlet NSTextField *allocatedDisk;
@property (weak) IBOutlet NSTextField *allocatedMemory;
@property (weak) IBOutlet NSSlider *rateDisk;
@property (weak) IBOutlet NSSlider *rateMemory;
@property (weak) IBOutlet NSTextField *infile;
@property (weak) IBOutlet NSTextField *infilesize;
@property (weak) IBOutlet NSTextField *totalMaxKmer;
@property (weak) IBOutlet NSTextField *n_ni;
@property (weak) IBOutlet NSTextField *n_np;
@property (weak) IBOutlet NSTextField *parfilesize;
@property (weak) IBOutlet NSTextField *tablefile;
@property (weak) IBOutlet NSTextField *tablefilesize;
@property (weak) IBOutlet NSTextField *kmercount;
@property (weak) IBOutlet NSTextField *querykmer;
@property (weak) IBOutlet NSTextField *valuekmer;
@property (weak) IBOutlet NSButton *quitbutton;
@property (weak) IBOutlet NSButton *countbutton;
@property (weak) IBOutlet NSButton *searchbutton;
@property (weak) IBOutlet NSTextField *ncpu;
@property (weak) IBOutlet NSStepper *ncpuStepper;
@property (weak) IBOutlet NSMenuItem *mnew;
@property (weak) IBOutlet NSMenuItem *mopen;
@property (weak) IBOutlet NSMenuItem *mopenrecent;
@property (weak) IBOutlet NSMenuItem *msave;
@property (weak) IBOutlet NSMenuItem *mload;
@property (weak) IBOutlet NSMenuItem *msearch;
@property (weak) IBOutlet NSMenuItem *mcount;

- (IBAction)save:(id)sender;
- (IBAction)open:(id)sender;
- (IBAction)load:(id)sender;
- (IBAction)count:(id)sender;
- (IBAction)search:(id)sender;
- (IBAction)outdir:(id)sender;
- (IBAction)adjustDisk:(id)sender;
- (IBAction)adjustMemory:(id)sender;
//- (IBAction)adjustCPU:(id)sender;

- (void)runCommand;
- (void)notifiedForStdOutput: (NSNotification *)notified;
- (void)notifiedForStdError: (NSNotification *)notified;
- (void)notifiedForComplete:(NSNotification *)anotification;

@end
