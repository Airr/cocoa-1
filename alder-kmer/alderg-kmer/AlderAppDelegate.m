//
//  AlderAppDelegate.m
//  alderg-kmer
//
//  Created by Sang Chul Choi on 1/2/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#include <stdlib.h>
#include "alder_cmacro.h"
#include "alder_file_availablediskspace.h"
#include "alder_file_availablememory.h"
#include "alder_file_size.h"
#include "alder_lcfg_kmer.h"
#include "alder_lcfg_read.h"
#import "AlderAppDelegate.h"

@implementation AlderAppDelegate

/**
 *  This function updates disk space and available memory in the computer.
 */
- (void)update_disk_and_memory
{
    unsigned long disk_space = 0;
    alder_file_availablediskspace("/", &disk_space);
    size_t available_diskspace = (size_t)(disk_space/ALDER_GB2BYTE);
    if (1 < available_diskspace) {
        [_availableDisk setStringValue:[NSString stringWithFormat:@"%zuGB",
                                        available_diskspace]];
    } else {
        [_availableDisk setStringValue:[NSString stringWithFormat:@"<1GB"]];
    }
    [_rateDisk setIntValue:50];
    size_t allocated_diskspace = available_diskspace * 50/100;
    if (1 < allocated_diskspace) {
        [_allocatedDisk setStringValue:[NSString stringWithFormat:@"%zuGB",
                                        allocated_diskspace]];
    } else {
        [_allocatedDisk setStringValue:[NSString stringWithFormat:@"<1GB"]];
    }
    
    int64_t free_memory = 0;
    int64_t used_memory = 0;
    alder_file_availablememory2(&free_memory, &used_memory);
    size_t available_memory = (size_t)(free_memory/ALDER_MB2BYTE);
    if (1 < available_memory) {
        [_availableMemory setStringValue:[NSString stringWithFormat:@"%zuMB",
                                          available_memory]];
    } else {
        [_availableMemory setStringValue:[NSString stringWithFormat:@"<1MB"]];
    }
    [_rateMemory setIntValue:50];
    size_t allocated_memory = available_memory * 50/100;
    if (1 < allocated_memory) {
        [_allocatedMemory setStringValue:[NSString stringWithFormat:@"%zuMB",
                                        allocated_memory]];
    } else {
        [_allocatedMemory setStringValue:[NSString stringWithFormat:@"<1MB"]];
    }
}

- (void)update_ncpu
{
    NSUInteger ncpu = [[NSProcessInfo processInfo] processorCount];
    [_ncpuStepper setMinValue:1];
    [_ncpuStepper setMaxValue:ncpu];
    [_ncpuStepper setStringValue:[NSString stringWithFormat:@"%lu", (unsigned long)ncpu]];
    [_ncpu setStringValue:[NSString stringWithFormat:@"%lu", (unsigned long)ncpu]];
}

- (void)update_kmersize
{
    [_kmerSizeStepper setMinValue:1];
    [_kmerSizeStepper setMaxValue:1024];
    [_kmerSizeStepper setStringValue:@"1"];
    [_kmerSize setStringValue:@"1"];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    [_countbutton setEnabled:NO];
    [_searchbutton setEnabled:NO];
    [_mcount setEnabled:NO];
    [_msearch setEnabled:NO];
    
    [self update_kmersize];
    [self update_ncpu];
    [self update_disk_and_memory];
    
#if !defined(NDEBUG)
    runScriptPath = [NSString stringWithFormat:@"/Users/goshng/Dropbox/Documents/Projects/cocoa/alder-kmer/alderg-kmer"];
    cmdScriptPath = [NSString stringWithFormat:@"/Users/goshng/Library/Developer/Xcode/DerivedData/alder-kmer-eucwazklbwkhuhcgfzvgeahyxwnn/Build/Products/Debug"];
#else
    runScriptPath = [NSString stringWithFormat:@"/Applications/alder"];
    cmdScriptPath = [NSString stringWithFormat:@"/Applications/alder"];
#endif
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    return [menuItem isEnabled];
}

- (IBAction)new:(id)sender {
//    const char *fn = "/Users/goshng/Downloads/a/outfile.kmr";
//    [self openFromURL:fn];
   
    [_countbutton setEnabled:NO];
    [_searchbutton setEnabled:NO];
    [_mcount setEnabled:NO];
    [_msearch setEnabled:NO];
    
    [self update_kmersize];
    [self update_ncpu];
    [self update_disk_and_memory];
    
    [_kmerSize setStringValue:[NSString stringWithFormat:@"1"]];
    [_outdir setStringValue:[NSString stringWithFormat:@"./"]];
    [_outfile setStringValue:[NSString stringWithFormat:@"outfile"]];
    [_infile setStringValue:[NSString stringWithFormat:@"Choose a sequenc file (fasta,fastq)"]];
    [_infilesize setStringValue:[NSString stringWithFormat:@""]];
    [_totalMaxKmer setStringValue:[NSString stringWithFormat:@""]];
    [_n_ni setStringValue:[NSString stringWithFormat:@""]];
    [_n_np setStringValue:[NSString stringWithFormat:@""]];
    [_parfilesize setStringValue:[NSString stringWithFormat:@""]];
    [_tablefile setStringValue:[NSString stringWithFormat:@""]];
    [_tablefilesize setStringValue:[NSString stringWithFormat:@""]];
    [_kmercount setStringValue:[NSString stringWithFormat:@""]];
    [_querykmer setStringValue:[NSString stringWithFormat:@""]];
    [_valuekmer setStringValue:[NSString stringWithFormat:@""]];
}

- (void)openFromURL:(const char *)fn {
    int bsSize = 1000;
    bstring bs = bfromcstralloc(bsSize, "");
    
    alder_lcfg_base_read_char(fn, "disk", (char**)&bs->data, bsSize);
    [_allocatedDisk setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "memory", (char**)&bs->data, bsSize);
    [_allocatedMemory setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "ncpu", (char**)&bs->data, bsSize);
    [_ncpu setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "kmersize", (char**)&bs->data, bsSize);
    [_kmerSize setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "outdir", (char**)&bs->data, bsSize);
    [_outdir setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "outfile", (char**)&bs->data, bsSize);
    [_outfile setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "infile", (char**)&bs->data, bsSize);
    [_infile setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "infilesize", (char**)&bs->data, bsSize);
    [_infilesize setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "totalmaxkmer", (char**)&bs->data, bsSize);
    [_totalMaxKmer setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "n_ni", (char**)&bs->data, bsSize);
    [_n_ni setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "n_np", (char**)&bs->data, bsSize);
    [_n_np setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "parfilesize", (char**)&bs->data, bsSize);
    [_parfilesize setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "tablefile", (char**)&bs->data, bsSize);
    [_tablefile setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "tablefilesize", (char**)&bs->data, bsSize);
    [_tablefilesize setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "kmercount", (char**)&bs->data, bsSize);
    [_kmercount setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "querykmer", (char**)&bs->data, bsSize);
    [_querykmer setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    alder_lcfg_base_read_char(fn, "valuekmer", (char**)&bs->data, bsSize);
    [_valuekmer setStringValue:[NSString stringWithFormat:@"%s", bs->data]];
    
    bdestroy(bs);
}

- (IBAction)open:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setAllowedFileTypes:@[@"kmr"]];
    
    // This method displays the panel and returns immediately.
    // The completion handler is called when the user selects an
    // item or cancels the panel.
    [panel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSURL* theDoc = [[panel URLs] objectAtIndex:0];
            // Open  the document.
            NSString* fileName = [theDoc path];
            NSLog(@"%@", fileName);
            const char *fn = [fileName cStringUsingEncoding:NSASCIIStringEncoding];
            
            [self openFromURL:fn];
            [self enableAction];
        }
    }];
}

- (void)saveToURL:(NSURL *)theDoc {
    // Write the content to a file
//    int allocatedDiskValue = [[[[_allocatedDisk stringValue] componentsSeparatedByCharactersInSet:
//                                [[NSCharacterSet decimalDigitCharacterSet] invertedSet]]
//                               componentsJoinedByString:@""] intValue];
//    allocatedDiskValue <<= 10;
    
    NSString *data = [NSString stringWithFormat:
                      @"disk = \"%@\"\n"
                      @"memory = \"%@\"\n"
                      @"ncpu = \"%@\"\n"
                      @"kmersize = \"%@\"\n"
                      @"outdir = \"%@\"\n"
                      @"outfile = \"%@\"\n"
                      @"infile = \"%@\"\n"
                      @"infilesize = \"%@\"\n"
                      @"totalmaxkmer = \"%@\"\n"
                      @"n_ni = \"%@\"\n"
                      @"n_np = \"%@\"\n"
                      @"parfilesize = \"%@\"\n"
                      @"tablefile = \"%@\"\n"
                      @"tablefilesize = \"%@\"\n"
                      @"kmercount = \"%@\"\n"
                      @"querykmer = \"%@\"\n"
                      @"valuekmer = \"%@\"\n",
                      [_allocatedDisk stringValue],
                      [_allocatedMemory stringValue],
                      [_ncpu stringValue],
                      [_kmerSize stringValue],
                      [_outdir stringValue],
                      [_outfile stringValue],
                      [_infile stringValue],
                      [_infilesize stringValue],
                      [_totalMaxKmer stringValue],
                      [_n_ni stringValue],
                      [_n_np stringValue],
                      [_parfilesize stringValue],
                      [_tablefile stringValue],
                      [_tablefilesize stringValue],
                      [_kmercount stringValue],
                      [_querykmer stringValue],
                      [_valuekmer stringValue]];
    
    NSError *error;
    BOOL ok = [data writeToURL:theDoc atomically:YES
                      encoding:NSASCIIStringEncoding error:&error];
    if (!ok) {
        // an error occurred
        NSLog(@"Error writing file at %@\n%@",
              theDoc, [error localizedFailureReason]);
        // implementation continues ...
    }
}

- (IBAction)save:(id)sender {
    NSSavePanel * savePanel = [NSSavePanel savePanel];
    // Restrict the file type to whatever you like
    [savePanel setAllowedFileTypes:@[@"kmr"]];
    // Set the starting directory
//    [savePanel setDirectoryURL:someURL];
    // Perform other setup
    // Use a completion handler -- this is a block which takes one argument
    // which corresponds to the button that was clicked
    [savePanel beginSheetModalForWindow:_window completionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            // Close panel before handling errors
            [savePanel orderOut:self];
            // Do what you need to do with the selected path
            NSURL* theDoc = [savePanel URL];
            
            NSString* fileName = [theDoc path];
            NSLog(@"%@", fileName);
            
            [self saveToURL:theDoc];
            
        }
    }];
}

- (IBAction)load:(id)sender {
    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    
    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:YES];
    [openDlg setAllowedFileTypes:@[@"fq",@"fq.gz",@"fastq",@"fastq.gz",@"fa",@"fa.gz",@"fasta",@"fasta.gz"]];
    
    if ( [openDlg runModal] == NSOKButton )
    {
        // Get an array containing the full filenames of all
        // files and directories selected.
        NSArray* files = [openDlg URLs];
        
        // Loop through all the files and process them.
        for( int i = 0; i < [files count]; i++ )
        {
            NSURL* url = [files objectAtIndex:i];
            
            NSString* fileName = [url path];
            
            unsigned long long fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:fileName error:nil] fileSize];
            
            fileSize >>= 20;
            [_infilesize setStringValue:[NSString stringWithFormat:@"%lluMB",
                                         fileSize]];
            
            // Do something with the filename
            
            // Compute the size of the file.
//            size_t fileSize = 0;
//            const char *f = [fileName cStringUsingEncoding:NSASCIIStringEncoding];
//            alder_file_size(f, &fileSize);
//            NSLog(@"%zu", fileSize);
//            fileSize >>= 20;
//            [_infilesize setStringValue:[NSString stringWithFormat:@"%zuMB",
//                                         fileSize]];
            
            [_infile setStringValue:fileName];
        }
        [_countbutton setEnabled:YES];
        [_mcount setEnabled:YES];
        
    }
}

- (IBAction)outdir:(id)sender {
    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    
    // Enable the selection of files in the dialog.
    [openDlg setCanChooseDirectories:YES];
    [openDlg setCanChooseFiles:NO];
    
    if ( [openDlg runModal] == NSOKButton )
    {
        // Get an array containing the full filenames of all
        // files and directories selected.
        NSArray* files = [openDlg URLs];
        
        // Loop through all the files and process them.
        for( int i = 0; i < [files count]; i++ )
        {
            NSURL* url = [files objectAtIndex:i];
            
            NSString* fileName = [url path];
            [_outdir setStringValue:fileName];
            // Do something with the filename
//            [customButtonImg setImage:[NSImage imageNamed:fileName]];
        }
    }
}

- (IBAction)adjustDisk:(id)sender {
    unsigned long disk_space = 0;
    alder_file_availablediskspace("/", &disk_space);
    size_t available_diskspace = (size_t)(disk_space/ALDER_GB2BYTE);
    if (1 < available_diskspace) {
        [_availableDisk setStringValue:[NSString stringWithFormat:@"%zuGB",
                                        available_diskspace]];
    } else {
        [_availableDisk setStringValue:[NSString stringWithFormat:@"<1GB"]];
    }
    int rate = [_rateDisk intValue];
    size_t allocated_diskspace = available_diskspace * rate/100;
    if (1 < allocated_diskspace) {
        [_allocatedDisk setStringValue:[NSString stringWithFormat:@"%zuGB",
                                        allocated_diskspace]];
    } else {
        [_allocatedDisk setStringValue:[NSString stringWithFormat:@"<1GB"]];
    }
}

- (IBAction)adjustMemory:(id)sender {
    int64_t free_memory = 0;
    int64_t used_memory = 0;
    alder_file_availablememory2(&free_memory, &used_memory);
    size_t available_memory = (size_t)(free_memory/ALDER_MB2BYTE);
    if (1 < available_memory) {
        [_availableMemory setStringValue:[NSString stringWithFormat:@"%zuMB",
                                          available_memory]];
    } else {
        [_availableMemory setStringValue:[NSString stringWithFormat:@"<1MB"]];
    }
    int rate = [_rateMemory intValue];
    size_t allocated_memory = available_memory * rate/100;
    if (1 < allocated_memory) {
        [_allocatedMemory setStringValue:[NSString stringWithFormat:@"%zuMB",
                                        allocated_memory]];
    } else {
        [_allocatedMemory setStringValue:[NSString stringWithFormat:@"<1MB"]];
    }
}

// To set the minimum and maximum values for a stepper, use setMinValue: and setMaxValue:. To choose how much the stepper changes its value when an arrow is pressed, use setIncrement:. Pressing the upper arrow increments the value by increment. Pressing the bottom arrow decrements the value by increment.

//To choose what happens when the user presses the mouse button and holds it down, use setAutoRepeat:. If autoRepeat is YES, the stepper changes its value at a regular interval. If autoRepeat is NO, the stepper changes its value once after the mouse button is released.

//To choose what happens when the user continues to press the stepper after its value has reached its minimum or maximum value, use setValueWraps:. If valueWraps is YES, the stepper wraps around to the minimum or maximum value. If valueWraps is NO, the stepper doesn’t change its value.
//- (IBAction)adjustCPU:(id)sender {
//    [sender setMinValue:1];
//    [sender incrementBy:1];
//    [_ncpuStepper ]
//}

- (IBAction)count:(id)sender {
    [_unixTaskStdOutput setString:@"" ];
    [_unixProgressBar startAnimation:sender];
    
    [self runCommand];
}

- (void)update_summary {
    // Read outfile.cfg.
    NSString *nfncfg = [[_outdir stringValue] stringByAppendingFormat:@"/%@.cfg",
                        [_outfile stringValue]];
    
    const char *fncfg = [nfncfg cStringUsingEncoding:NSASCIIStringEncoding];
    int kmersize = 0;
    alder_lcfg_kmer_readKmersize(fncfg, &kmersize);
    [_kmerSize setStringValue:[NSString stringWithFormat:@"%d", kmersize]];
    uint64 totalkmer = 0;
    alder_lcfg_kmer_readTotalkmer(fncfg, &totalkmer);
    [_totalMaxKmer setStringValue:[NSString stringWithFormat:@"%llu", totalkmer]];
    uint64_t ni = 0;
    alder_lcfg_kmer_readNi(fncfg, &ni);
    [_n_ni setStringValue:[NSString stringWithFormat:@"%llu", ni]];
    uint64_t np = 0;
    alder_lcfg_kmer_readNp(fncfg, &np);
    [_n_np setStringValue:[NSString stringWithFormat:@"%llu", np]];
    uint64_t parfilesize = 0;
    alder_lcfg_kmer_readParfilesize(fncfg, &parfilesize);
    parfilesize >>= 20;
    [_parfilesize setStringValue:[NSString stringWithFormat:@"%lluMB", parfilesize]];
    
    bstring btabfn = bfromcstralloc(100, "");
    alder_lcfg_kmer_readTablefilename(fncfg, (char **)&btabfn->data, 99);
    [_tablefile setStringValue:[NSString stringWithFormat:@"%s", btabfn->data]];
    bdestroy(btabfn);
    
    size_t tabfilesize = 0;
    alder_lcfg_kmer_readTablefilesize(fncfg, &tabfilesize);
    tabfilesize >>= 20;
    if (1 < tabfilesize) {
        [_tablefilesize setStringValue:[NSString stringWithFormat:@"%zuMB", tabfilesize]];
    } else {
        [_tablefilesize setStringValue:[NSString stringWithFormat:@"<1MB"]];
    }
    
    size_t kmercount = 0;
    alder_lcfg_kmer_readNh(fncfg, &kmercount);
    [_kmercount setStringValue:[NSString stringWithFormat:@"%zu", kmercount]];
}

- (IBAction)search:(id)sender {
    [_unixTaskStdOutput setString:@"" ];
    [_unixProgressBar setIndeterminate:YES];
    [_unixProgressBar startAnimation:sender];
    
    [self searchCommand];
}

- (void)disableActionWhileRunning
{
    [_countbutton setEnabled:NO];
    [_searchbutton setEnabled:NO];
    [_mnew setEnabled:NO];
    [_mopen setEnabled:NO];
    [_mopenrecent setEnabled:NO];
    [_msave setEnabled:NO];
    [_mload setEnabled:NO];
    [_mcount setEnabled:NO];
    [_msearch setEnabled:NO];
}

- (void)runCommand
{
    //setup system pipes and filehandles to process output data
    unixStandardOutputPipe = [[NSPipe alloc] init];
    unixStandardErrorPipe =  [[NSPipe alloc] init];
    
    fhOutput = [unixStandardOutputPipe fileHandleForReading];
    fhError =  [unixStandardErrorPipe fileHandleForReading];
    
    //setup notification alerts
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    
    [nc addObserver:self selector:@selector(notifiedForStdOutput:) name:NSFileHandleReadCompletionNotification object:fhOutput];
    [nc addObserver:self selector:@selector(notifiedForStdError:)  name:NSFileHandleReadCompletionNotification object:fhError];
    [nc addObserver:self selector:@selector(notifiedForComplete:)  name:NSTaskDidTerminateNotification object:unixTask];
    
    int allocatedDiskValue = [[[[_allocatedDisk stringValue] componentsSeparatedByCharactersInSet:
                                [[NSCharacterSet decimalDigitCharacterSet] invertedSet]]
                               componentsJoinedByString:@""] intValue];
    allocatedDiskValue <<= 10;
    int allocatedMemoryValue = [[[[_allocatedMemory stringValue] componentsSeparatedByCharactersInSet:
                                  [[NSCharacterSet decimalDigitCharacterSet] invertedSet]]
                                 componentsJoinedByString:@""] intValue];
    
    
    
    NSMutableArray *commandLine = [NSMutableArray new];
    [commandLine addObject:[NSString stringWithFormat:@"%@/alderg-kmer-run.sh", runScriptPath]];
    [commandLine addObject:cmdScriptPath];
    [commandLine addObject:[_kmerSize stringValue]];
    [commandLine addObject:[NSString stringWithFormat:@"%d", allocatedDiskValue]];
    [commandLine addObject:[NSString stringWithFormat:@"%d", allocatedMemoryValue]];
    [commandLine addObject:[_outdir stringValue]];
    [commandLine addObject:[_outfile stringValue]];
    [commandLine addObject:[_infile stringValue]];
    [commandLine addObject:[_ncpu stringValue]];
    
    unixTask = [[NSTask alloc] init];
    [unixTask setLaunchPath:@"/bin/bash"];
    [unixTask setArguments:commandLine];
    [unixTask setStandardOutput:unixStandardOutputPipe];
    [unixTask setStandardError:unixStandardErrorPipe];
    [unixTask setStandardInput:[NSPipe pipe]];
    [unixTask launch];
    
    //note we are calling the file handle not the pipe
    [fhOutput readInBackgroundAndNotify];
    [fhError readInBackgroundAndNotify];
    
    [self disableActionWhileRunning];
}

-(void) notifiedForStdOutput: (NSNotification *)notified
{
    
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard data ready %ld bytes",data.length);
    
    if ([data length]){
        
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSTextStorage *ts = [_unixTaskStdOutput textStorage];
        [ts replaceCharactersInRange:NSMakeRange([ts length], 0)
                          withString:outputString];
    }
    
    if (unixTask != nil) {
        
        [fhOutput readInBackgroundAndNotify];
    }
    
}

-(void) notifiedForStdError: (NSNotification *)notified
{
    
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard error ready %ld bytes",data.length);
    
    if ([data length]) {
        
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [_unixProgressBar setDoubleValue:[outputString doubleValue]];
    }
    
    if (unixTask != nil) {
        
        [fhError readInBackgroundAndNotify];
    }
    
}

- (void)enableAction
{
    [_countbutton setEnabled:YES];
    [_searchbutton setEnabled:YES];
    [_mnew setEnabled:YES];
    [_mopen setEnabled:YES];
    [_mopenrecent setEnabled:YES];
    [_msave setEnabled:YES];
    [_mload setEnabled:YES];
    [_mcount setEnabled:YES];
    [_msearch setEnabled:YES];
}

-(void) notifiedForComplete:(NSNotification *)anotification
{
    
    NSLog(@"task completed or was stopped with exit code %d",[unixTask terminationStatus]);
    unixTask = nil;
    
    [_unixProgressBar stopAnimation:self];
    [_unixProgressBar viewDidHide];
    
    if ([unixTask terminationStatus] == 0) {
        [_countbutton setEnabled:YES];
        [_searchbutton setEnabled:YES];
        
        [self update_summary];
        [self enableAction];
    }
    else {
//        [_unixProgressUpdate setStringValue:@"Terminated with non-zero exit code"];
    }
}

//NSString *runCommand(NSString *commandToRun)
//{
//    NSTask *task;
//    task = [[NSTask alloc] init];
//    [task setLaunchPath: @"/bin/sh"];
//    
//    NSArray *arguments = [NSArray arrayWithObjects:
//                          @"-c" ,
//                          [NSString stringWithFormat:@"%@", commandToRun],
//                          nil];
//    NSLog(@"run command: %@",commandToRun);
//    [task setArguments: arguments];
//    
//    NSPipe *pipe;
//    pipe = [NSPipe pipe];
//    [task setStandardOutput: pipe];
//    
//    NSFileHandle *file;
//    file = [pipe fileHandleForReading];
//    
//    [task launch];
//    
//    NSData *data;
//    data = [file readDataToEndOfFile];
//    
//    NSString *output;
//    output = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
//    return output;
//}

- (void)searchCommand
{
    //setup system pipes and filehandles to process output data
    unixStandardOutputPipe = [[NSPipe alloc] init];
    unixStandardErrorPipe =  [[NSPipe alloc] init];
    
    fhOutput = [unixStandardOutputPipe fileHandleForReading];
    fhError =  [unixStandardErrorPipe fileHandleForReading];
    
    //setup notification alerts
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    
    [nc addObserver:self selector:@selector(searchNotifiedForStdOutput:) name:NSFileHandleReadCompletionNotification object:fhOutput];
    [nc addObserver:self selector:@selector(searchNotifiedForStdError:)  name:NSFileHandleReadCompletionNotification object:fhError];
    [nc addObserver:self selector:@selector(searchNotifiedForComplete:)  name:NSTaskDidTerminateNotification object:unixTask];
    
    NSMutableArray *commandLine = [NSMutableArray new];
    [commandLine addObject:[NSString stringWithFormat:@"%@/alderg-kmer-search.sh", runScriptPath]];
    [commandLine addObject:cmdScriptPath];
    [commandLine addObject:[_tablefile stringValue]];
    [commandLine addObject:[_querykmer stringValue]];
    
    unixTask = [[NSTask alloc] init];
    [unixTask setLaunchPath:@"/bin/bash"];
    [unixTask setArguments:commandLine];
    [unixTask setStandardOutput:unixStandardOutputPipe];
    [unixTask setStandardError:unixStandardErrorPipe];
    [unixTask setStandardInput:[NSPipe pipe]];
    [unixTask launch];
    
    //note we are calling the file handle not the pipe
    [fhOutput readInBackgroundAndNotify];
    [fhError readInBackgroundAndNotify];
    
    [_searchbutton setEnabled:NO];
}

-(void) searchNotifiedForStdOutput: (NSNotification *)notified
{
    
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard data ready %ld bytes",data.length);
    
    if ([data length]){
        
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        NSTextStorage *ts = [_unixTaskStdOutput textStorage];
        [ts replaceCharactersInRange:NSMakeRange([ts length], 0)
                          withString:outputString];
    }
    
    if (unixTask != nil) {
        
        [fhOutput readInBackgroundAndNotify];
    }
}

-(void) searchNotifiedForStdError: (NSNotification *)notified
{
    
    NSData * data = [[notified userInfo] valueForKey:NSFileHandleNotificationDataItem];
    NSLog(@"standard error ready %ld bytes",data.length);
    
    if ([data length]) {
        
        NSString * outputString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if ([outputString intValue] < 0) {
            [_valuekmer setStringValue:[NSString stringWithFormat:@"No Found!"]];
        } else {
            [_valuekmer setStringValue:outputString];
        }
    }
    
    if (unixTask != nil) {
        
        [fhError readInBackgroundAndNotify];
    }
    
}

-(void) searchNotifiedForComplete:(NSNotification *)anotification
{
    
    NSLog(@"task completed or was stopped with exit code %d",[unixTask terminationStatus]);
    unixTask = nil;
    
    [_unixProgressBar stopAnimation:self];
    [_unixProgressBar viewDidHide];
    [_unixProgressBar setIndeterminate:NO];
    
    if ([unixTask terminationStatus] == 0) {
        [_searchbutton setEnabled:YES];
    }
    else {
//        [_unixProgressUpdate setStringValue:@"Terminated with non-zero exit code"];
    }
}

@end
