//
//  SCCProjectWindowController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCProjectWindowController.h"
#import "SCCProjectNavigatorViewController.h"
#import "SCCLogNavigatorViewController.h"
#import "SCCFindNavigatorViewController.h"
#import "SCCReportNavigatorViewController.h"
#import "SCCContentOutlineViewController.h"
#import "SCCNavigationSelectorBarViewController.h"
#import "SCCContentSelectorBarViewController.h"
#import "SCCInspectorSelectorBarViewController.h"
#import "SCCContentNothingViewController.h"
#import "SCCContentProjectViewController.h"
#import "SCCContentProductViewController.h"
#import "SCCProjectKeys.h"
#import "SCCContentXfastViewController.h"
#import "SCCContentTextViewController.h"
#import "SCCLogJobViewController.h"
#import "SCCLibraryXfastViewController.h"
#import "SCCInspectorFileViewController.h"
#import "SCCCommand.h"
#import "SCCDefaultsKeys.h"


@interface SCCProjectWindowController ()
@property NSMutableArray *navigatorViewControllers;
@property NSViewController *navigatorViewController;
@property NSViewController *navigationSelectBarViewController;
@property NSViewController *contentSelectBarViewController;
@property NSViewController *inspectorSelectBarViewController;
@property SCCContentOutlineViewController *contentOutlineViewController;

@property NSViewController *contentViewController;
@property SCCContentNothingViewController *contentNothingViewController;
@property SCCContentProjectViewController *contentProjectViewController;
@property SCCContentProductViewController *contentProductViewController;
@property SCCContentXfastViewController   *contentXfastViewController;
@property SCCContentTextViewController    *contentTextViewController;
@property SCCLogJobViewController         *logJobViewController;
@property SCCLibraryXfastViewController   *libraryXfastViewController;

@property NSViewController *inspectorViewController;
@property SCCInspectorFileViewController  *inspectorFileViewController;

@property (weak) IBOutlet NSView           *navigatorView;
@property (weak) IBOutlet NSView           *navigationSelectorBarView;
@property (weak) IBOutlet NSView           *contentSelectorBarView;
@property (weak) IBOutlet NSView           *contentOutlinView;
@property (weak) IBOutlet NSView           *contentView;
@property (weak) IBOutlet NSView           *logView;
@property (weak) IBOutlet NSView           *inspectorSelectBarView;
@property (weak) IBOutlet NSView           *inspectorView;
@property (weak) IBOutlet NSView           *libraryView;


@property (weak) IBOutlet NSSplitView      *mainSplitView;
@property (weak) IBOutlet NSSplitView      *contentSplitView;
@property (weak) IBOutlet NSSplitView      *logSplitView;

//@property NSArray *navigationSelects;
//@property NSArray *inspectorSelects;
@property BOOL addFileReference;

@end

@implementation SCCProjectWindowController

static NSArray *navigationSelects;
static NSArray *inspectorSelects;
static NSArray *contentXfastMessages;

+ (NSArray *)navigationSelects
{
    if (nil == navigationSelects) {
        navigationSelects = [NSArray arrayWithObjects:
                             @"project",
                             @"find",
                             @"report",
                             @"log", nil];
    }
    return navigationSelects;
}



+ (NSArray *)inspectorSelects
{
    if (nil == inspectorSelects) {
        inspectorSelects = [NSArray arrayWithObjects:
                            @"file",
                            @"help",
                            @"identity",
                            @"attribute",
                            nil];
    }
    return inspectorSelects;
}

+ (NSArray *)contentXfastMessages
{
    if (nil == contentXfastMessages) {
        contentXfastMessages = [NSArray arrayWithObjects:
                                @"nodeIsSelected",
                                @"addFileReference",
                                nil];
    }
    return contentXfastMessages;
}

- (NSRect)rectCenterOfScreen {
    NSRect sizeWindow = NSMakeRect(200, 600, 800, 600);
    for (NSScreen *screen in [NSScreen screens]) {
        NSRect screenRect = [screen visibleFrame];
        sizeWindow.origin.x = screenRect.size.width / 2 - sizeWindow.size.width / 2;
        sizeWindow.origin.y = screenRect.size.height / 2 - sizeWindow.size.height / 2 + 200;
    }
    return sizeWindow;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    
    // Center the window.
    NSRect sizeWindow = [self rectCenterOfScreen];
    [[self window] setFrame:sizeWindow display:NO];
    
    // Main SplitView
    [[self mainSplitView] setPosition:170 ofDividerAtIndex:0];
    [[self mainSplitView] setPosition:650 ofDividerAtIndex:1];
    // Log SplitView
    [[self logSplitView] setPosition:450 ofDividerAtIndex:0];
    // Content SplitView
    [[self contentSplitView] setPosition:170 ofDividerAtIndex:0];
    
    NSView *view;
    
    // Content View
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    _contentViewController = _contentNothingViewController;
    view = [_contentNothingViewController view];
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
    
    // Navigator View
    view = [_navigatorViewController view];
    view.frame = _navigatorView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_navigatorView addSubview:view];
    
    // Navigation Select Bar View
    view = [_navigationSelectBarViewController view];
    view.frame = _navigationSelectorBarView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_navigationSelectorBarView addSubview:view];
    
    // Content Select Bar View
    view = [_contentSelectBarViewController view];
    view.frame = _contentSelectorBarView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentSelectorBarView addSubview:view];
    
    // Inspector Select Bar View
    view = [_inspectorSelectBarViewController view];
    view.frame = _inspectorSelectBarView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_inspectorSelectBarView addSubview:view];
    
    // Content Outline View
    view = [_contentOutlineViewController view];
    view.frame = _contentOutlinView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentOutlinView addSubview:view];
    

    
    // Log Job View
    view = [_logJobViewController view];
    view.frame = _logView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_logView addSubview:view];
    [_logJobViewController addData];
    
    // Library View
    view = [_libraryXfastViewController view];
    view.frame = _libraryView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_libraryView addSubview:view];
    [_libraryXfastViewController addData];
    
    // Inspector View
    _inspectorViewController = _inspectorFileViewController;
    view = [_inspectorFileViewController view];
    view.frame = _inspectorView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_inspectorView addSubview:view];
    
    // Connect the navigation select bar to self.
    
    for (NSString *navigationSelect in [SCCProjectWindowController navigationSelects]) {
        [[self navigationSelectBarViewController] addObserver:self
                                                   forKeyPath:navigationSelect
                                                      options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                                                      context:nil];
    }
    
    for (NSString *inspectorSelect in [SCCProjectWindowController inspectorSelects]) {
        [[self inspectorSelectBarViewController] addObserver:self
                                                  forKeyPath:inspectorSelect
                                                     options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                                                     context:nil];
    }
    
    
    // Display the content view.
}

- (void)dealloc
{
    for (NSString *navigationSelect in [SCCProjectWindowController navigationSelects]) {
        [[self navigationSelectBarViewController] removeObserver:self
                                                      forKeyPath:navigationSelect];
    }
    
    for (NSString *inspectorSelect in [SCCProjectWindowController inspectorSelects]) {
        [[self inspectorSelectBarViewController] removeObserver:self
                                                     forKeyPath:inspectorSelect];
    }
    
    SCCProjectNavigatorViewController *viewController = [[self navigatorViewControllers] objectAtIndex:0];
    [viewController removeObserver:self forKeyPath:@"contents"];
    [viewController removeObserver:self forKeyPath:@"nodeIsSelected"];
    
    
    for (NSString *contentXfastMessage in [SCCProjectWindowController contentXfastMessages]) {
        [_contentOutlineViewController removeObserver:self
                                           forKeyPath:contentXfastMessage];
    }
    
    [[NSNotificationCenter defaultCenter] removeObserver:kJobDidFinish];

}

- (id)init
{
    self = [super initWithWindowNibName:@"SCCProjectDocument"];
    if (self) {
        // Initialization code here.
        _navigatorViewController = [[SCCProjectNavigatorViewController alloc] init];
        _contentOutlineViewController = [[SCCContentOutlineViewController alloc] init];
        _navigationSelectBarViewController = [[SCCNavigationSelectorBarViewController alloc] init];
        _inspectorSelectBarViewController = [[SCCInspectorSelectorBarViewController alloc] init];
        _contentSelectBarViewController = [[SCCContentSelectorBarViewController alloc] init];
        _navigatorViewControllers = [NSMutableArray arrayWithObjects:
                                     _navigatorViewController,
                                     [NSNull null],
                                     [NSNull null],
                                     [NSNull null],
                                     nil];
    }
    return self;
}

- (id)initWithData:(NSMutableDictionary *)data
{
    self = [super initWithWindowNibName:@"SCCProjectDocument"];
    if (self) {
        // Initialization code here.
        NSMutableArray *contents = [data objectForKey:SCC_DATA_PROJECT_NAVIGATOR];
        
        _navigatorViewController = [[SCCProjectNavigatorViewController alloc]
                                    initWithContent:contents];
        
        _contentOutlineViewController = [[SCCContentOutlineViewController alloc] init];
        
        _contentXfastViewController = [[SCCContentXfastViewController alloc] init];
        _contentTextViewController = [[SCCContentTextViewController alloc] init];
        
        _logJobViewController = [[SCCLogJobViewController alloc] init];
        
        _libraryXfastViewController = [[SCCLibraryXfastViewController alloc] init];
        
        _inspectorFileViewController = [[SCCInspectorFileViewController alloc] init];
        _navigationSelectBarViewController = [[SCCNavigationSelectorBarViewController alloc] init];
        _inspectorSelectBarViewController = [[SCCInspectorSelectorBarViewController alloc] init];
        _contentSelectBarViewController = [[SCCContentSelectorBarViewController alloc] init];
        _navigatorViewControllers = [NSMutableArray arrayWithObjects:
                                     _navigatorViewController,
                                     [NSNull null],
                                     [NSNull null],
                                     [NSNull null],
                                     nil];
        
        _contentNothingViewController = [[SCCContentNothingViewController alloc] init];
        
        
        [_navigatorViewController addObserver:self
                                   forKeyPath:@"contents"
                                      options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                      context:nil];
        [_navigatorViewController addObserver:self
                                   forKeyPath:@"nodeIsSelected"
                                      options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                      context:nil];
        
        for (NSString *contentXfastMessage in [SCCProjectWindowController contentXfastMessages]) {
            [_contentOutlineViewController addObserver:self
                                            forKeyPath:contentXfastMessage
                                               options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                               context:nil];
        }
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(anyThread_handleJobDone:)
                                                     name:kJobDidFinish
                                                   object:nil];
        
        NSString *pathToDatabase = [[NSUserDefaults standardUserDefaults] stringForKey:DatabaseDirectory];
        NSLog(@"Database: %@", pathToDatabase);

    }
    return self;
}

- (void)mainThread_handleJobDone:(NSNotification *)note
{
    NSLog(@"mainThread");
    NSLog(@"mainThread2");
    NSDictionary *notifData = [note userInfo];
    NSLog(@"%@:%@:%@:%@:%@:%@",
          [notifData objectForKey:@"project"],
          [notifData objectForKey:@"xfast"],
          [notifData objectForKey:@"target"],
          [notifData objectForKey:@"start"],
          [notifData objectForKey:@"end"],
          [notifData objectForKey:@"status"]);
    [_logJobViewController removeEntryWithProject:[notifData objectForKey:@"project"]
                                        withXfast:[notifData objectForKey:@"xfast"]
                                       withTarget:[notifData objectForKey:@"target"]
                                        withStart:[notifData objectForKey:@"start"]
                                          withEnd:[notifData objectForKey:@"end"]
                                       withStatus:[notifData objectForKey:@"status"]];
    
    // Pending NSNotifications can possibly back up while waiting to be executed,
    // and if the user stops the queue, we may have left-over pending
    // notifications to process.
    //
    // So make sure we have "active" running NSOperations in the queue
    // if we are to continuously add found image files to the table view.
    // Otherwise, we let any remaining notifications drain out.
    //
//    NSDictionary *notifData = [note userInfo];
//
//    NSNumber *loadScanCountNum = [notifData valueForKey:kScanCountKey];
//    NSInteger loadScanCount = [loadScanCountNum integerValue];
//    
//    if ([myStopButton isEnabled])
//    {
//        // make sure the current scan matches the scan of our loaded image
//        if (scanCount == loadScanCount)
//        {
//            [tableRecords addObject:notifData];
//            [myTableView reloadData];
//            
//            // set the number of images found indicator string
//            [self updateCountIndicator];
//        }
//    }
}

- (void)anyThread_handleJobDone:(NSNotification *)note
{
    NSLog(@"anyThread");
    // update our table view on the main thread
    [self performSelectorOnMainThread:@selector(mainThread_handleJobDone:) withObject:note waitUntilDone:NO];
}

- (void)addEntryWithProject:(NSString *)project
                  withXfast:(NSString *)xfast
                 withTarget:(NSString *)target
                  withStart:(NSString *)start
                    withEnd:(NSString *)end
                 withStatus:(NSString *)status
{
    [_logJobViewController addEntryWithProject:project withXfast:xfast withTarget:target withStart:start withEnd:end withStatus:status];
}


- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    // has the array controller's "selectionIndexes" binding changed?
    if (object == [self navigationSelectBarViewController])
    {
        NSLog(@"navigation select bar: %@", keyPath);
        
        if ([self navigatorViewController]) {
            [[[self navigatorViewController] view] removeFromSuperview];
        }
        if ([keyPath compare:@"project"] == NSOrderedSame) {
            if ([[self navigatorViewControllers] objectAtIndex:0] == [NSNull null]) {
                _navigatorViewController = [[SCCLogNavigatorViewController alloc] init];
                [[self navigatorViewControllers] setObject:_navigatorViewController
                                        atIndexedSubscript:0];
            }
            _navigatorViewController = [[self navigatorViewControllers] objectAtIndex:0];
        } else if ([keyPath compare:@"find"] == NSOrderedSame) {
            if ([[self navigatorViewControllers] objectAtIndex:1] == [NSNull null]) {
                _navigatorViewController = [[SCCFindNavigatorViewController alloc] init];
                [[self navigatorViewControllers] setObject:_navigatorViewController
                                        atIndexedSubscript:1];
            }
            _navigatorViewController = [[self navigatorViewControllers] objectAtIndex:1];
        } else if ([keyPath compare:@"report"] == NSOrderedSame) {
            if ([[self navigatorViewControllers] objectAtIndex:2] == [NSNull null]) {
                _navigatorViewController = [[SCCReportNavigatorViewController alloc] init];
                [[self navigatorViewControllers] setObject:_navigatorViewController
                                        atIndexedSubscript:2];
            }
            _navigatorViewController = [[self navigatorViewControllers] objectAtIndex:2];
        } else {
            if ([[self navigatorViewControllers] objectAtIndex:3] == [NSNull null]) {
                _navigatorViewController = [[SCCLogNavigatorViewController alloc] init];
                [[self navigatorViewControllers] setObject:_navigatorViewController
                                        atIndexedSubscript:3];
            }
            _navigatorViewController = [[self navigatorViewControllers] objectAtIndex:3];
        }
        NSView *view = [_navigatorViewController view];
        view.frame = _navigatorView.bounds;
        [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [_navigatorView addSubview:view];
    }
    else if (object == [self inspectorSelectBarViewController])
    {
        NSLog(@"inspector select bar: %@", keyPath);
    }
    else if (object == [self navigatorViewController])
    {
        if ([keyPath compare:@"contents"] == NSOrderedSame) {
//            NSLog(@"Main Window: project navigator view");
            // Load the Xfast outline view.
            // Load the content view with a default selection of the Xfast outline view.
            // How can I know which one is selected in the navigator outline view?
            
//            [self showContentOfXfast];
//            [self willChangeValueForKey:@"navigatorViewController"];
//            [self didChangeValueForKey:@"navigatorViewController"];
        } else if ([keyPath compare:@"nodeIsSelected"] == NSOrderedSame) {
            NSLog(@"Main Window: project navigator view");
            // Load the Xfast outline view.
            // Load the content view with a default selection of the Xfast outline view.
            // How can I know which one is selected in the navigator outline view?
            
  
            [self willChangeValueForKey:@"navigatorViewController"];
            [self didChangeValueForKey:@"navigatorViewController"];
        }
    }
    else if (object == [self contentOutlineViewController])
    {
        if ([keyPath compare:@"nodeIsSelected"] == NSOrderedSame)
        {
            NSLog(@"Main Window: content outline view");
            // Load the Xfast outline view.
            // Load the content view with a default selection of the Xfast outline view.
            // How can I know which one is selected in the navigator outline view?
            [self willChangeValueForKey:@"contentOutlineViewController"];
            [self didChangeValueForKey:@"contentOutlineViewController"];
        }
        else if ([keyPath compare:@"addFileReference"] == NSOrderedSame)
        {
            NSLog(@"Main Window: content outline view - save - add a file reference");
            [self willChangeValueForKey:@"addFileReference"];
            [self didChangeValueForKey:@"addFileReference"];
        }
    }
    else
    {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

/**
 *  Calls the project navigator view controller to add an Xfast file.
 *
 *  @param type The Xfast type
 *  @param name The Xfast name
 */
- (void)addXfast:(NSString *)type withName:(NSString *)name withKey:(NSString *)key
{
    SCCProjectNavigatorViewController *viewController = [[self navigatorViewControllers] objectAtIndex:0];
    [viewController addXfast:type withName:name withKey:key];
}

- (void)showContentOfXfast:(NSMutableArray *)content
{
    [_contentOutlineViewController populateOutlineContentsWithArray:content];
}

- (NSString *)selectedNodeKeyOfProjectNavigator
{
    SCCProjectNavigatorViewController *viewController = [[self navigatorViewControllers] objectAtIndex:0];
    NSString *key = [viewController selectedKey];
    return key;
}

/**
 *  Displays a blank content.
 */
- (void)showContentOfNothing
{
    if (_contentNothingViewController == nil) {
        _contentNothingViewController = [[SCCContentNothingViewController alloc] init];
    }
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    _contentViewController = _contentNothingViewController;
    NSView *view = [_contentViewController view];
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
}

- (void)showContentOfProduct
{
    if (_contentProductViewController == nil) {
        _contentProductViewController = [[SCCContentProductViewController alloc] init];
    }
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    _contentViewController = _contentProductViewController;
     NSView *view = [_contentViewController view];
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
}

- (void)showContentOfProject
{
    if (_contentProjectViewController == nil) {
        _contentProjectViewController = [[SCCContentProjectViewController alloc] init];
    }
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    _contentViewController = _contentProjectViewController;
    NSView *view = [_contentViewController view];
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
}

- (NSMutableArray *)treeContentOfXfastOutline
{
    return [_contentOutlineViewController treeContent];
}


- (void)showXfastProjectSettings:(NSMutableDictionary *)settings
{
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    
    _contentViewController = _contentXfastViewController;
    NSView *view = [_contentViewController view];
    
    // Tip: The Nib file is loaded only after the view method is called.
    [_contentXfastViewController changeSettings:settings];
    
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
}


- (void)showXfastText:(NSString *)text withPath:(NSString *)path
{
    if ([self contentViewController]) {
        [[[self contentViewController] view] removeFromSuperview];
    }
    
    _contentViewController = _contentTextViewController;
    NSView *view = [_contentViewController view];
    
    // Tip: The Nib file is loaded only after the view method is called.
    [_contentTextViewController changeText:text withPath:path];
    
    view.frame = _contentView.bounds;
    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [_contentView addSubview:view];
}

@end



















































