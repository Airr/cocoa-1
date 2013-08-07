//
//  SCWindowController.m
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCWindowController.h"
#import "SCPerson.h"
#import "CustomImageViewController.h"
#import "CustomTableViewController.h"
#import "SCRunAnalysisViewController.h"
#import "SCStopAnalysisViewController.h"
#import "SCConfigurationViewController.h"


@interface SCWindowController ()
{
    IBOutlet NSView*	myTargetView;
    NSViewController*	myCurrentViewController;
    IBOutlet NSPanel*   theSheet;
}
@property NSMutableArray *personArray;
@property NSMutableArray *repositoryArray;
- (NSViewController*)viewController;
@end

@implementation SCWindowController

@synthesize personArray;
@synthesize repositoryArray;
@synthesize managedObjectContext;
enum	// popup tag choices
{
	kImageView = 0,
	kTableView,
    kRunAnalysisView,
    kStopAnalysisView,
    kConfigurationView,
};

NSString *const kViewTitle		= @"CustomImageView";
NSString *const kTableTitle		= @"CustomTableView";
NSString *const kRunAnalysisTitle		= @"SCRunAnalysisViewController";
NSString *const kStopAnalysisTitle		= @"SCStopAnalysisViewController";
NSString *const kConfigurationTitle		= @"SCConfigurationViewController";


- (id)init
{
    self = [super initWithWindowNibName:@"SCDocument"];
    if (self) {
        
    }
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
}



-(void)insertObject:(SCPerson *)p inPersonArrayAtIndex:(NSUInteger)index {
    [personArray insertObject:p atIndex:index];
}

-(void)removeObjectFromPersonArrayAtIndex:(NSUInteger)index {
    [personArray removeObjectAtIndex:index];
}

-(void)setPersonArray:(NSMutableArray *)a {
    personArray = a;
}

-(NSArray*)personArray {
    return personArray;
}

-(void)insertObject:(SCPerson *)p inRepositoryArrayAtIndex:(NSUInteger)index {
    [repositoryArray insertObject:p atIndex:index];
}

-(void)removeObjectFromRepositoryArrayAtIndex:(NSUInteger)index {
    [repositoryArray removeObjectAtIndex:index];
}

-(void)setRepositoryArray:(NSMutableArray *)a {
    repositoryArray = a;
}

-(NSArray*)repositoryArray {
    return repositoryArray;
}




- (void)changeViewController:(NSInteger)whichViewTag
{
	// we are about to change the current view controller,
	// this prepares our title's value binding to change with it
	[self willChangeValueForKey:@"viewController"];
	
	if ([myCurrentViewController view] != nil)
		[[myCurrentViewController view] removeFromSuperview];	// remove the current view
    
    //	if (myCurrentViewController != nil)
    //		[myCurrentViewController release];		// remove the current view controller
	
	switch (whichViewTag)
	{
		case 0:	// swap in the "CustomImageViewController - NSImageView"
		{
			CustomImageViewController* imageViewController =
            [[CustomImageViewController alloc] initWithNibName:kViewTitle bundle:nil];
            [imageViewController setManagedObjectContext:managedObjectContext];
			if (imageViewController != nil)
			{
				
				myCurrentViewController = imageViewController;	// keep track of the current view controller
				[myCurrentViewController setTitle:kViewTitle];
			}
			break;
		}
            
		case 1:	// swap in the "CustomTableViewController - NSTableView"
		{
			CustomTableViewController* tableViewController =
            [[CustomTableViewController alloc] initWithNibName:kTableTitle bundle:nil];
            [tableViewController setManagedObjectContext:managedObjectContext];
			if (tableViewController != nil)
			{
				myCurrentViewController = tableViewController;	// keep track of the current view controller
				[myCurrentViewController setTitle:kTableTitle];
			}
			break;
		}
		case kRunAnalysisView:	// swap in the "CustomImageViewController - NSImageView"
		{
			SCRunAnalysisViewController *imageViewController =
            [[SCRunAnalysisViewController alloc] initWithNibName:kRunAnalysisTitle bundle:nil];
            [imageViewController setManagedObjectContext:managedObjectContext];
			if (imageViewController != nil)
			{
				
				myCurrentViewController = imageViewController;	// keep track of the current view controller
				[myCurrentViewController setTitle:kRunAnalysisTitle];
			}
			break;
		}
		case kStopAnalysisView:	// swap in the "CustomImageViewController - NSImageView"
		{
			SCStopAnalysisViewController* imageViewController =
            [[SCStopAnalysisViewController alloc] initWithNibName:kStopAnalysisTitle bundle:nil];
            [imageViewController setManagedObjectContext:managedObjectContext];
			if (imageViewController != nil)
			{
				
				myCurrentViewController = imageViewController;	// keep track of the current view controller
				[myCurrentViewController setTitle:kStopAnalysisTitle];
			}
			break;
		}
        case kConfigurationView:
		{
			SCConfigurationViewController* imageViewController =
            [[SCConfigurationViewController alloc] initWithNibName:kConfigurationTitle bundle:nil];
            [imageViewController setManagedObjectContext:managedObjectContext];
			if (imageViewController != nil)
			{
				
				myCurrentViewController = imageViewController;	// keep track of the current view controller
				[myCurrentViewController setTitle:kConfigurationTitle];
			}
			break;
		}
	}
	
	// embed the current view to our host view
	[myTargetView addSubview: [myCurrentViewController view]];
	
	// make sure we automatically resize the controller's view to the current window size
	[[myCurrentViewController view] setFrame: [myTargetView bounds]];
	
	// set the view controller's represented object to the number of subviews in that controller
	// (our NSTextField's value binding will reflect this value)
	[myCurrentViewController setRepresentedObject: [NSNumber numberWithUnsignedLong: [[[myCurrentViewController view] subviews] count]]];
	
	[self didChangeValueForKey:@"viewController"];	// this will trigger the NSTextField's value binding to change
}


- (void)awakeFromNib {
    // Execution buttons on the left view.
    NSMutableArray *analysisArray = [NSMutableArray array];
    SCPerson *pmFastq = [[SCPerson alloc] init];
    pmFastq.name = @"Fastq";
    SCPerson *pmConfiguration = [[SCPerson alloc] init];
    pmConfiguration.name = @"Configuration";
    SCPerson *pmPreprocessing = [[SCPerson alloc] init];
    pmPreprocessing.name = @"Preprocessing";
    SCPerson *pmAlignment = [[SCPerson alloc] init];
    pmAlignment.name = @"Alignment";
    SCPerson *pmDE = [[SCPerson alloc] init];
    pmDE.name = @"DE";
//    [analysisArray addObject:pmFastq];
    [analysisArray addObject:pmConfiguration];
    [self setPersonArray:analysisArray];
    
    // Execution buttons on the repository panel view.
    NSMutableArray *repositoryAnalysisArray = [NSMutableArray array];
    [repositoryAnalysisArray addObject:pmPreprocessing];
//    [repositoryAnalysisArray addObject:pmConfiguration];
    [repositoryAnalysisArray addObject:pmAlignment];
    [repositoryAnalysisArray addObject:pmDE];
    [self setRepositoryArray:repositoryAnalysisArray];
    
    // Table on the right view.
    [self changeViewController: kImageView];
}

- (NSViewController*)viewController
{
	return myCurrentViewController;
}

- (IBAction)runAlignment:(id)sender
{
    if (![sender isKindOfClass:[NSButton class]])
        return;
    
    NSString *title = [(NSButton *)sender title];
    
    if ([title compare:@"Fastq"] == NSOrderedSame)
    {
        [self changeViewController:kTableView];
    }
    else if ([title compare:@"Configuration"] == NSOrderedSame)
    {
        [self changeViewController:kConfigurationView];
    }
    else if ([title compare:@"Preprocessing"] == NSOrderedSame)
    {
        [self changeViewController:kTableView];
    }
    else if ([title compare:@"Alignment"] == NSOrderedSame)
    {
        [self changeViewController:kTableView];
    }
    else if ([title compare:@"DE"] == NSOrderedSame)
    {
        [self changeViewController:kTableView];
    }
}

- (IBAction)runAnalysis:(id)sender
{
    [self changeViewController:kRunAnalysisView];
}

- (IBAction)stopAnalysis:(id)sender
{
    [self changeViewController:kStopAnalysisView];
    
}

- (IBAction)addAnalysis:(id)sender
{
    // Execution buttons on the left view.
    NSMutableArray *analysisArray = [NSMutableArray arrayWithArray:personArray];
    
    SCPerson *pmAlignment = [[SCPerson alloc] init];
    pmAlignment.name = @"More";
    [analysisArray addObject:pmAlignment];
    
    [self setPersonArray:analysisArray];
    
}

// Use isChecked to remove persons.
- (IBAction)removeAnalysis:(id)sender
{
    NSMutableIndexSet *indexes = [NSMutableIndexSet indexSet];
    NSMutableArray *sourceArray = [NSMutableArray arrayWithArray:personArray];
    NSUInteger numberOfPerson = [sourceArray count];
    for (NSUInteger i = 0; i < numberOfPerson; i++) {
        SCPerson *pm = [sourceArray objectAtIndex:i];
        if ([pm isChecked] == YES)
        {
            [indexes addIndex:i];
        }
    }
    NSArray *addArray = [sourceArray objectsAtIndexes:indexes];
    NSMutableArray *destArray = [NSMutableArray arrayWithArray:repositoryArray];
    [destArray addObjectsFromArray:addArray];
    [sourceArray removeObjectsAtIndexes:indexes];
    
    [self setPersonArray:sourceArray];
    [self setRepositoryArray:destArray];
}

- (IBAction)showTheSheet:(id)sender {
    
    // Execution buttons on the left view.
    NSMutableArray *analysisArray = [NSMutableArray arrayWithArray:repositoryArray];
    [self setRepositoryArray:analysisArray];
    
    [NSApp beginSheet:theSheet
       modalForWindow:[self window]
        modalDelegate:self
       didEndSelector:nil
          contextInfo:nil];
    
}

- (IBAction)endTheSheet:(id)sender {
    
    if ([[(NSButton *)sender title] compare:@"Add"] == NSOrderedSame)
    {
        NSMutableIndexSet *indexes = [NSMutableIndexSet indexSet];
        NSMutableArray *sourceArray = [NSMutableArray arrayWithArray:repositoryArray];
        NSUInteger numberOfPerson = [sourceArray count];
        for (NSUInteger i = 0; i < numberOfPerson; i++) {
            SCPerson *pm = [sourceArray objectAtIndex:i];
            if ([pm isChecked] == YES)
            {
                [indexes addIndex:i];
            }
        }
        NSArray *addArray = [sourceArray objectsAtIndexes:indexes];
        NSMutableArray *destArray = [NSMutableArray arrayWithArray:personArray];
        [destArray addObjectsFromArray:addArray];
        [sourceArray removeObjectsAtIndexes:indexes];
        
        [self setRepositoryArray:sourceArray];
        [self setPersonArray:destArray];
    }
    
    [NSApp endSheet:theSheet];
    [theSheet orderOut:sender];
    
}


@end
