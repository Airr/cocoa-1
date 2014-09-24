   
//  SCCWindowController.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/18/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCWindowController.h"
#import "SCCViewControllerText.h"
#import "SCCViewControllerModule.h"
#import "SCCViewControllerSettings.h"
#import "SCCViewControllerBuildSettings.h"
#import "XcodeEditor.h"
#import "XfastEditor.h"
#import "SCCWelcomeOperation.h"
#import "MyWindowController.h"
#import "SCCTargetWindowController.h"


@interface SCCWindowController ()
{
    NSViewController *_editorViewController;
}

@property (weak) IBOutlet NSOutlineView *outlineView;
@property (weak) IBOutlet NSOutlineView *xfastOutlineView;
@property (weak) IBOutlet NSView *editorView;

@property CGFloat positionContentSplitViewDivider;
@property (weak) IBOutlet NSSplitView *contentSplitView;
@property (weak) IBOutlet NSView *navigatorView;
@property (weak) IBOutlet NSView *xfastView;

@property CGFloat positionXfastSplitViewDivider;
@property (weak) IBOutlet NSSplitView *xfastSplitView;

@property CGFloat positionLogSplitViewDivider;
@property (weak) IBOutlet NSSplitView *logSplitView;

@property (weak) IBOutlet NSView *logView;

@property CGFloat positionContentSplitViewDivider2;
@property (weak) IBOutlet NSView *detailView;




@property MyWindowController *myWindowController;
@property SCCTargetWindowController *targetWindowController;

@property (nonatomic) XCProject* project;
@property (nonatomic) XFProject* xfastProject;
@property NSOperationQueue *queue;

- (NSString *)input: (NSString *)prompt defaultValue: (NSString *)defaultValue;

@end

@implementation SCCWindowController

- (id)init
{
    self = [super initWithWindowNibName:@"SCCDocument"];
    if (self) {
        // Initialization code here.
//        _project = [[XCProject alloc] initWithFilePath:@"caribou/caribou.xfastproj"];
        _project = [[XCProject alloc] initWithFilePath:@"/Users/goshng/Library/Developer/Xcode/DerivedData/Xfast-bfmvadqhoomsezcbprmkuxaxecjy/Build/Products/Debug/caribou/caribou.xfastproj"];
    }
    return self;
}

- (id)initWithProject:(XCProject *)project
{
    self = [super initWithWindowNibName:@"SCCDocument"];
    if (self) {
        // Initialization code here.
        _project = project;
        _queue = [[NSOperationQueue alloc] init];
        [_queue setMaxConcurrentOperationCount:1];
//        _contents = [[NSMutableArray alloc] init];
    }
    return self;
    
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    [_outlineView expandItem:nil expandChildren:YES];
}

#pragma mark *** Overrides of outlineView Protocols ***

- (NSInteger)outlineView:(NSOutlineView *)outlineView
  numberOfChildrenOfItem:(id)item
{
    if (outlineView == _outlineView) {
        if (item == nil) {
            XCGroup *group = [_project rootGroup];
            return [[group members] count];
        } else {
            return [((XCGroup *)item).members count];
        }
    } else {
        if (item == nil) {
            XFGroup *group = [_xfastProject rootGroup];
            return [[group members] count];
        } else {
            NSLog(@"group: %@", (XFGroup*)item);
            return [((XFGroup *)item).members count];
        }
    }
}

- (id)outlineView:(NSOutlineView *)outlineView
            child:(NSInteger)index
           ofItem:(id)item
{
    if (outlineView == _outlineView) {
        if (item == nil) {
            XCGroup *group = [_project rootGroup];
            return [[group members] objectAtIndex:index];
        } else {
            return [((XCGroup *)item).members objectAtIndex:index];
        }
    } else {
        if (item == nil) {
            XFGroup *group = [_xfastProject rootGroup];
            return [[group members] objectAtIndex:index];
        } else {
            return [((XFGroup *)item).members objectAtIndex:index];
        }
    }
    return NULL;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    if (outlineView == _outlineView) {
        if (item == nil) {
            return YES;
        } else {
            XcodeMemberType type = [(XCGroup *)item groupMemberType];
            if (type == PBXGroupType) {
                return YES;
            } else {
                return NO;
            }
        }
    } else {
        if (item == nil) {
            return YES;
        } else {
            XcodeMemberType type = [(XFGroup *)item groupMemberType];
            if (type == XFPBXGroupType) {
                return YES;
            } else {
                return NO;
            }
        }
    }
}
//
//- (NSView *)outlineView:(NSOutlineView *)outlineView
//     viewForTableColumn:(NSTableColumn *)tableColumn
//                   item:(id)item
//{
//    if (outlineView == _outlineView) {
//        if (item == nil) {
//            XCGroup *group = [_project rootGroup];
////            return [[group members] objectAtIndex:index];
//            NSView *result = [outlineView makeViewWithIdentifier:[tableColumn identifier] owner:self];
//            if ([result isKindOfClass:[NSTableCellView class]]) {
//                NSTableCellView *cellView = (NSTableCellView *)result;
//                // setup the color; we can't do this in bindings
//                cellView.imageView.objectValue = @"NSHomeTemplate";
//            }
//            return result;
//        } else {
////            return [((XCGroup *)item).members objectAtIndex:index];
//        }
//
//     
//    }
//
//    return item;
//}

- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
           byItem:(id)item
{
    if (outlineView == _outlineView) {
        NSLog(@"%@\n", item);
        
        return item;
    } else {
        NSLog(@"XfastOutline - %@\n", item);
        
        return item;
    }
}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    NSOutlineView *theOutline =[notification object];
    
    if (theOutline == _outlineView) {
        
        if ([_outlineView selectedRow] != -1) {
            
            XCGroup *item = [_outlineView itemAtRow:[_outlineView selectedRow]];
            NSString *filePath = [[[_project filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[item pathRelativeToProjectRoot]];
            
            //        NSLog(@"selected outline view 1 %@: %@/%@", [item displayName], [[_project filePath] stringByDeletingLastPathComponent], [item pathRelativeToProjectRoot]);
            NSLog(@"selected outline view 1 %@: %@", [item displayName], filePath);
            NSLog(@"Extension is %@", [[item displayName] pathExtension]);
            
            if ([[[item displayName] pathExtension] compare:@"xfast"] == NSOrderedSame) {
                NSString *baseDirectory = [[[_project filePath] stringByDeletingLastPathComponent]
                                           stringByAppendingPathComponent:@"xfastfolder"];
                [_xfastOutlineView reloadData];
                _xfastProject = [[XFProject alloc] initWithFilePath:filePath
                                                      baseDirectory:baseDirectory];
                [_xfastOutlineView reloadData];
                [_xfastOutlineView expandItem:nil expandChildren:YES];
            }

        }
    } else {
        
        if ([theOutline selectedRow] != -1) {
            XFGroup *item = [theOutline itemAtRow:[theOutline selectedRow]];
            
            
            NSString *filePath = [[[_xfastProject filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[item pathRelativeToProjectRoot]];
            
            NSLog(@"selected outline view 2 %@: %@", [item displayName], filePath);
            NSLog(@"Extension is %@", [[item displayName] pathExtension]);
            if ([item groupMemberType] == XFPBXFileReferenceType) {
                NSLog(@"Absolute Path: %@", [item absolutePath]);
            }
            
            // remove the original view
            if (_editorViewController) {
                [[_editorViewController view] removeFromSuperview];
            }
            
            if ([item groupMemberType] == XFPBXNativeTargetType) {
                [_xfastProject setCurrentTargetKey:[item key]];
                SCCViewControllerSettings *viewControllerText = [[SCCViewControllerSettings alloc] initWithXFProject:_xfastProject];
                
                if (viewControllerText != nil)
                {
                    _editorViewController = viewControllerText;
                    NSView *view = [_editorViewController view];
                    view.frame = _editorView.bounds;
                    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
                    [_editorView addSubview:view];
                }

            }
            else if ([[[item displayName] pathExtension] compare:@""] == NSOrderedSame)
            {
                // create a setting view
                SCCViewControllerModule *viewControllerText = [[SCCViewControllerModule alloc] initWithXFProject:_xfastProject];
                
                if (viewControllerText != nil)
                {
                    _editorViewController = viewControllerText;
                    NSView *view = [_editorViewController view];
                    view.frame = _editorView.bounds;
                    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
                    [_editorView addSubview:view];
                }
            } else if (NO) {
                
                // create a textview
                SCCViewControllerText *viewControllerText = [[SCCViewControllerText alloc] initWithFilePath:filePath];
                
                if (viewControllerText != nil)
                {
                    _editorViewController = viewControllerText;
                    NSView *view = [_editorViewController view];
                    view.frame = _editorView.bounds;
                    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
                    [_editorView addSubview:view];
                }
            } else {
                
                // create a textview
                SCCViewControllerBuildSettings *viewControllerText = [[SCCViewControllerBuildSettings alloc] initWithFilePath:filePath];
                
                if (viewControllerText != nil)
                {
                    _editorViewController = viewControllerText;
                    NSView *view = [_editorViewController view];
                    view.frame = _editorView.bounds;
                    [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
                    [_editorView addSubview:view];
                }
            }
            
        }
    }
}

#pragma mark Buttons

- (IBAction)addFiles:(id)sender
{
    int i; // Loop counter.
    
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:YES];
    [openDlg setAllowsMultipleSelection:YES];
    NSInteger result = [openDlg runModal];
    if (result == NSOKButton)
    {
        [_xfastOutlineView reloadData];
        NSString *groupFilePath = [[_xfastProject name] stringByAppendingPathComponent:@"Files"];
        XFGroup *groupXfast = [_xfastProject groupWithPathFromRoot:groupFilePath];
        
        // Get an array containing the full filenames of all
        // files and directories selected.
        NSArray* urls = [openDlg URLs];
        
        // Loop through all the files and process them.
        for( i = 0; i < [urls count]; i++ )
        {
            NSURL *fileURL = [urls objectAtIndex:i];
            NSString *filePath = [fileURL path];
//            NSString *filePath = [fileURL absoluteString];
            
            // Do something with the filename.
            NSLog(@"%d - fileURL [%@], filePath [%@]", i, fileURL, filePath);
            
            XFSourceFileDefinition *sourceDefinition =
            [XFSourceFileDefinition sourceDefinitionWithName:filePath
                                                        data:nil
                                                        type:XFTEXT];
            [sourceDefinition setFileOperationType:XFFileOperationTypeReferenceOnly];
            [groupXfast addFile:sourceDefinition];
            
//            XFFrameworkDefinition* frameworkDefinition =
//            [[XFFrameworkDefinition alloc] initWithFilePath:filePath copyToDestination:NO];
//            [groupXfast addFramework:frameworkDefinition];
        }
        [_xfastProject save];
        [_xfastOutlineView reloadData];
        [_xfastOutlineView expandItem:nil expandChildren:YES];
    }
}

- (IBAction)removeFiles:(id)sender
{
    NSLog(@"Button - removeFiles");
}

- (IBAction)addTarget:(id)sender
{
    NSString *targetName = [self input:@"Enter your target name:" defaultValue:@"Your Target"];
    [_xfastOutlineView reloadData];
    NSString *groupFilePath = [[_xfastProject name] stringByAppendingPathComponent:@"Targets"];
    XFGroup *groupXfast = [_xfastProject groupWithPathFromRoot:groupFilePath];

    XFSourceFileDefinition *sourceDefinition =
    [XFSourceFileDefinition sourceDefinitionWithName:targetName
                                                data:nil
                                                type:XfastTarget];
    [sourceDefinition setFileOperationType:XFFileOperationTypeReferenceOnly];
    [groupXfast addTarget:sourceDefinition];
    
    
    [_xfastProject save];
    [_xfastOutlineView reloadData];
    [_xfastOutlineView expandItem:nil expandChildren:YES];
}

- (NSString *)input: (NSString *)prompt defaultValue: (NSString *)defaultValue
{
    NSAlert *alert = [NSAlert alertWithMessageText: prompt
                                     defaultButton:@"OK"
                                   alternateButton:@"Cancel"
                                       otherButton:nil
                         informativeTextWithFormat:@""];
    
    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 24)];
    [input setStringValue:defaultValue];
    [alert setAccessoryView:input];
    NSInteger button = [alert runModal];
    if (button == NSAlertDefaultReturn) {
        [input validateEditing];
        return [input stringValue];
    } else if (button == NSAlertAlternateReturn) {
        return nil;
    } else {
        NSAssert1(NO, @"Invalid input dialog button %ld", button);
        return nil;
    }
}

/**
 *
 *
 */
- (IBAction)submitTarget:(id)sender
{
    // 1. Use the current target and send it to a quene.
    // 2. When the job was finished, get notified and change the state.
    // Use NSOperation and NSOperationQueue.
//    NSBlockOperation* theOp = [NSBlockOperation blockOperationWithBlock: ^{
//        NSLog(@"Beginning operation.\n");
//        // Do some work.
//        sleep(10);
//        NSLog(@"Ending operation");
//    }];
    
    // 1. Find the current xfast and its current target.
    // 2. Construct the command line for the target.
    //
    //id object = [[NSClassFromString(@"NameofClass") alloc] init];

    
    
    SCCWelcomeOperation *theOp = [[SCCWelcomeOperation alloc] initWithXFProject:_xfastProject];
    [theOp setCompletionBlock:^{
        
        NSLog(@"- Done");
    }];
    
    [_queue addOperation:theOp];
}

- (IBAction)addXfast:(id)sender
{
//    if (_sheet == nil) {
//        [[NSBundle mainBundle] loadNibNamed:@"Panel" owner:self topLevelObjects:nil];
//    }
//    [[self window] beginSheet:_sheet completionHandler:^(NSModalResponse returnCode) {
//        NSLog(@"finishAddXfast code: %ld", (long)returnCode);
//    }];
    //	[self.myWindowController showWindow:self];
//    
//    if (_myWindowController == nil) {
//        _myWindowController = [[MyWindowController alloc] initWithWindowNibName:@"MainWindow"];
//        [_myWindowController addObserver:self
//                              forKeyPath:@"xfastName"
//                                 options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
//                                 context:NULL];
//
//    }
//    
//    [[self window] beginSheet:[_myWindowController window] completionHandler:^(NSModalResponse returnCode)
//     {
//         NSLog(@"END: add files complete handler");
//     }];
//    NSLog(@"END: add files");
    
    if (_targetWindowController == nil) {
        _targetWindowController = [[SCCTargetWindowController alloc] initWithWindowNibName:@"SCCTargetWindowController"];
        [_targetWindowController addObserver:self
                                  forKeyPath:@"xfastName"
                                     options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                                     context:NULL];
        
    }
    
    [[self window] beginSheet:[_targetWindowController window] completionHandler:^(NSModalResponse returnCode)
     {
         NSLog(@"END: add files complete handler");
     }];
    NSLog(@"END: add files");
    
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
    
    if ([keyPath isEqual:@"xfastName"]) {
        NSLog(@"KVO: xfastName is %@", [_targetWindowController xfastName]);
        // Add the Xfast project.
        [_outlineView reloadData];
        [self addXfastFile:[_targetWindowController xfastName]];
        [_outlineView reloadData];
        [_outlineView expandItem:nil expandChildren:YES];

        
        
//        [_fileTable reloadData];
    }
    /*
     Be sure to call the superclass's implementation *if it implements it*.
     NSObject does not implement the method.
     */
    //    [super observeValueForKeyPath:keyPath
    //                         ofObject:object
    //                           change:change
    //                          context:context];
}

#pragma mark - Toolbar

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return YES;
}

- (void)splitViewDidResizeSubviews:(NSNotification *)aNotification
{
    
    
}

- (BOOL)splitView:(NSSplitView *)splitView shouldCollapseSubview:(NSView *)subview forDoubleClickOnDividerAtIndex:(NSInteger)dividerIndex
{
    // yes, if you can collapse you should collapse it
    return YES;
}


- (IBAction)showHideNavigator:(id)sender {
    
    if ([[self contentSplitView] isSubviewCollapsed:[self navigatorView]]) {
        [[self navigatorView] setHidden:NO];
        [[self contentSplitView] setPosition:[self positionContentSplitViewDivider]
                            ofDividerAtIndex:0];
    } else {
        [self setPositionContentSplitViewDivider:[[self navigatorView] frame].size.width];
        [[self contentSplitView] setPosition:0 ofDividerAtIndex:0];
        [[self navigatorView] setHidden:YES];
    }
    [[self contentSplitView] adjustSubviews];

}

- (IBAction)showHideXfastView:(id)sender {
    if ([[self xfastSplitView] isSubviewCollapsed:[self xfastView]]) {
        [[self xfastView] setHidden:NO];
        [[self xfastSplitView] setPosition:[self positionXfastSplitViewDivider]
                          ofDividerAtIndex:0];
    } else {
        [self setPositionXfastSplitViewDivider:[[self xfastView] frame].size.width];
        [[self xfastSplitView] setPosition:0 ofDividerAtIndex:0];
        [[self xfastView] setHidden:YES];
    }
    [[self xfastSplitView] adjustSubviews];
}

- (IBAction)showHideLogView:(id)sender {
    if ([[self logSplitView] isSubviewCollapsed:[self logView]]) {
        [[self logView] setHidden:NO];
        [[self logSplitView] setPosition:_positionLogSplitViewDivider ofDividerAtIndex:0];
    } else {
        _positionLogSplitViewDivider = [[self xfastSplitView] frame].size.height;
//        NSLog(@"max log splitview: %f", [[self logSplitView] maxPossiblePositionOfDividerAtIndex:0]);
//        NSRect a = [[self logSplitView] frame];
//        NSLog(@"height log splitview: %f", a.size.height);
        [[self logSplitView] setPosition:([[self logSplitView] maxPossiblePositionOfDividerAtIndex:0] + 1)
                        ofDividerAtIndex:0];
        [[self logView] setHidden:YES];

        
    }

    [[self logSplitView] adjustSubviews];
}

- (IBAction)showHideDetailView:(id)sender {
    if ([[self contentSplitView] isSubviewCollapsed:[self detailView]]) {
        [[self detailView] setHidden:NO];
        [[self contentSplitView] setPosition:[self positionContentSplitViewDivider2]
                            ofDividerAtIndex:1];
    } else {
//        CGFloat a = [[self navigatorView] frame].size.width + [[self xfastSplitView] frame].size.width;
        CGFloat a = [[self contentSplitView] maxPossiblePositionOfDividerAtIndex:0];
        [self setPositionContentSplitViewDivider2:a];
        [[self contentSplitView] setPosition:[[self contentSplitView] maxPossiblePositionOfDividerAtIndex:1]
                            ofDividerAtIndex:1];
        [[self detailView] setHidden:YES];
    }
    [[self contentSplitView] adjustSubviews];
}

//- (IBAction)finishAddXfast:(id)sender
//{
//    [[self window] endSheet:_sheet returnCode:1];
//}

#pragma mark - Xfast

/**
 *
 */
- (void)addXfastFile:(NSString *)xfastName
{
    
    NSString *projectBasePath = [[[self project] filePath] stringByDeletingLastPathComponent];
    
    NSString *projectName = [projectBasePath lastPathComponent];
    NSString *projectMainPath = [projectBasePath stringByAppendingPathComponent:projectName];
    NSString *projectXfastPath = [projectBasePath stringByAppendingPathComponent:@"xfastfolder"];
    NSString *projectPath = [projectMainPath stringByAppendingPathExtension:@"xfastproj"];
//    NSURL *projectURL = [NSURL fileURLWithPath:projectPath isDirectory:YES];
    
//    XCProject *project = [[XCProject alloc] initWithNewFilePath:projectBasePath];
    
    XCGroup *group = [[self project] rootGroup];
    
    XCGroup *group1 = [group memberWithDisplayName:projectName];
    XCGroup *group11 = [group1 memberWithDisplayName:@"Jobs"];
    
//    [group1 addGroupWithPath:@"Products"];
//    [project save];
    
    /* Welcome xfast */
    NSString *xfastFilename = [NSString stringWithFormat:@"%@.xfast", xfastName];
    NSString *xfastPath = [projectPath stringByAppendingPathComponent:xfastName];
    
    assert(0);
    XFProject *xfast = [XFProject projectWithNewFilePath:xfastPath withTemplate:projectXfastPath];
//                                           baseDirectory:projectXfastPath];
    
    NSString *xfastPBXProj = [NSString stringWithFormat:@"%@.xfast/project.pbxproj", xfastName];
    NSString *xfastGroupPath = [xfastPath stringByAppendingPathComponent:xfastPBXProj];
    XFGroup *groupXfast = [xfast rootGroup];
    XFGroup *groupXfast1 = [groupXfast addGroupWithPath:xfastName];
    [groupXfast1 addGroupWithPath:@"Targets"];
    [groupXfast1 addGroupWithPath:@"Files"];
    [xfast saveToPBXProj];
    
    
    NSData *xfastData = [NSData dataWithContentsOfFile:xfastGroupPath];
    XCSourceFileDefinition* header = [[XCSourceFileDefinition alloc]
                                      initWithName:xfastFilename
                                      data:xfastData
                                      type:SourceCodeXfast];
    [group11 addSourceFile:header];
    [[self project] save];
}

@end















