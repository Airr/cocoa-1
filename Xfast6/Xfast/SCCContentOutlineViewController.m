//
//  SCCContentOutlineViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/25/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCContentOutlineViewController.h"
#import "SCCProjectDocument.h"
#import "SCCChildNode.h"
#import "ImageAndTextCell.h"
#import "SeparatorCell.h"
#import "SCCTreeAdditionObj.h"
#import "XFKeyBuilder.h"

#define COLUMNID_NAME			@"NameColumn"	// the single column name in our outline view
#define INITIAL_INFODICT		@"outline"		// name of the dictionary file to populate our outline view

#define ICONVIEW_NIB_NAME		@"IconView"		// nib name for the icon view
#define FILEVIEW_NIB_NAME		@"FileView"		// nib name for the file view
#define CHILDEDIT_NAME			@"ChildEdit"	// nib name for the child edit window controller

#define UNTITLED_NAME			@"Untitled"		// default name for added folders and leafs

#define HTTP_PREFIX				@"http://"

// default folder titles
#define PLACES_NAME				@"PLACES"
#define BOOKMARKS_NAME			@"PROJECT"
#define PROJECT_NAME			@"XFAST"
#define TARGETS_NAME			@"TARGETS"
#define FILES_NAME			@"FILES"
//#define BOOKMARKS_NAME			@"BOOKMARKS"

// keys in our disk-based dictionary representing our outline view's data
#define KEY_NAME				@"name"
#define KEY_URL					@"url"
#define KEY_KEY					@"key"
#define KEY_PATH			    @"path"
#define KEY_SEPARATOR			@"separator"
#define KEY_GROUP				@"group"
#define KEY_FOLDER				@"folder"
#define KEY_ENTRIES				@"entries"

#define kMinOutlineViewSplit	120.0f

#define kIconImageSize          16.0

#define kNodesPBoardType		@"myNodesPBoardType"	// drag and drop pasteboard type



#pragma mark -

@interface SCCContentOutlineViewController ()
{
    IBOutlet NSView				*placeHolderView;
    IBOutlet NSSplitView		*splitView;
    IBOutlet NSProgressIndicator *progIndicator;
    IBOutlet NSButton			*removeButton;
    IBOutlet NSPopUpButton		*actionButton;
    IBOutlet NSTextField		*urlField;
    
    // cached images for generic folder and url document
    NSImage						*folderImage;
    NSImage						*urlImage;
    BOOL						retargetWebView;
    SeparatorCell				*separatorCell;	// the cell used to draw a separator line in the outline view
}

@property (strong) IBOutlet NSTreeController *treeController;
@property (weak) IBOutlet NSOutlineView *myOutlineView;
@property (weak) IBOutlet NSButton *addFolderButton;

@property NSArray *dragNodesArray;
@property NSMutableArray *contents;

@property NSMutableArray *projectContent;
@property NSMutableArray *targetContent;
@property NSMutableArray *fileContent;
@property NSMutableArray *keepContents;

@property BOOL nodeIsSelected;
@property BOOL addFileReference;
@property (weak) IBOutlet NSButton *fireButton;
@property (nonatomic, weak) SCCProjectDocument *document;

@end

#pragma mark -

@implementation SCCContentOutlineViewController

@synthesize treeController;
@synthesize myOutlineView;
@synthesize addFolderButton;

- (instancetype)initWithNibName:(NSString *)nibNameOrNil
                         bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCContentOutlineViewController" bundle:nil];
    if (self) {
        _contents = [[NSMutableArray alloc] init];
        _nodeIsSelected = NO;
        
        // cache the reused icon images
        folderImage = [[NSWorkspace sharedWorkspace]
                       iconForFileType:NSFileTypeForHFSTypeCode(kGenericFolderIcon)];
        [folderImage setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
        
        urlImage = [[NSWorkspace sharedWorkspace]
                    iconForFileType:NSFileTypeForHFSTypeCode(kGenericURLIcon)];
        [urlImage setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
        
    }
    return self;
}

- (void)loadView
{
    [super loadView];
}

- (SCCProjectDocument *)document
{
    if (_document == nil) {
        _document = [[[[self view] window] windowController] document];
    }
    return _document;
}

- (void)dealloc
{
    //    [[NSNotificationCenter defaultCenter] removeObserver:self
    //                                                    name:kReceivedContentNotification
    //                                                  object:nil];
}

- (void)awakeFromNib
{
    _nodeIsSelected = NO;
    

    // apply our custom ImageAndTextCell for rendering the first column's cells
    NSTableColumn *tableColumn = [myOutlineView tableColumnWithIdentifier:COLUMNID_NAME];
    ImageAndTextCell *imageAndTextCell = [[ImageAndTextCell alloc] init];
    [imageAndTextCell setEditable:YES];
    [tableColumn setDataCell:imageAndTextCell];
    
    separatorCell = [[SeparatorCell alloc] init];
    [separatorCell setEditable:NO];
    
    // add our content
    // see goshng
    [self populateOutlineContents];
    
    // scroll to the top in case the outline contents is very long
    [[[myOutlineView enclosingScrollView] verticalScroller] setFloatValue:0.0];
    [[[myOutlineView enclosingScrollView] contentView] scrollToPoint:NSMakePoint(0,0)];
    // make our outline view appear with gradient selection, and behave like the Finder, iTunes, etc.
    [myOutlineView setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleSourceList];
    
    // drag and drop support
    [myOutlineView registerForDraggedTypes:[NSArray arrayWithObjects:
                                            kNodesPBoardType,			// our internal drag type
                                            NSURLPboardType,			// single url from pasteboard
                                            NSFilenamesPboardType,		// from Safari or Finder
                                            NSFilesPromisePboardType,	// from Safari or Finder (multiple URLs)
                                            nil]];
    
    _nodeIsSelected = YES;
    
    NSUInteger indexArr[] = {0,0};
    NSIndexPath *indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2]; //
    [treeController setSelectionIndexPath:indexPath];
    
    //    [[NSNotificationCenter defaultCenter] addObserver:self
    //                                             selector:@selector(contentReceived:)
    //                                                 name:kReceivedContentNotification object:nil];
    
}




#pragma mark - Actions


/**
 *  Take the currently selected node and select its parent.
 *
 *  This is called several times in different places.
 */
- (void)selectParentFromSelection
{
    if ([[treeController selectedNodes] count] > 0)
    {
        NSTreeNode *firstSelectedNode = [[treeController selectedNodes] objectAtIndex:0];
        NSTreeNode *parentNode = [firstSelectedNode parentNode];
        if (parentNode)
        {
            // select the parent
            NSIndexPath *parentIndex = [parentNode indexPath];
            [treeController setSelectionIndexPath:parentIndex];
        }
        else
        {
            // no parent exists (we are at the top of tree), so make no selection in our outline
            NSArray *selectionIndexPaths = [treeController selectionIndexPaths];
            [treeController removeSelectionIndexPaths:selectionIndexPaths];
        }
    }
}

// -------------------------------------------------------------------------------
//	performAddFolder:treeAddition
// -------------------------------------------------------------------------------
- (void)performAddFolder:(SCCTreeAdditionObj *)treeAddition
{
    // NSTreeController inserts objects using NSIndexPath, so we need to calculate this
    NSIndexPath *indexPath = nil;
    
    _contents.count;
    // if there is no selection, we will add a new group to the end of the contents array
    if ([[treeController selectedObjects] count] == 0)
    {
        // there's no selection so add the folder to the top-level and at the end
        indexPath = [NSIndexPath indexPathWithIndex:self.contents.count];
    }
    else
    {
        // get the index of the currently selected node, then add the number its children to the path -
        // this will give us an index which will allow us to add a node to the end of the currently selected node's children array.
        //
        indexPath = [treeController selectionIndexPath];
        if ([[[treeController selectedObjects] objectAtIndex:0] isLeaf])
        {
            // user is trying to add a folder on a selected child,
            // so deselect child and select its parent for addition
            [self selectParentFromSelection];
        }
        else
        {
            indexPath = [indexPath indexPathByAddingIndex:[[[[treeController selectedObjects] objectAtIndex:0] children] count]];
        }
    }
    
    SCCChildNode *node = [[SCCChildNode alloc] init];
    node.nodeTitle = [treeAddition nodeName];
    node.key = [treeAddition nodeKey];
    
    // the user is adding a child node, tell the controller directly
    [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
    
}

- (void)addFolder:(NSString *)folderName withKey:(NSString *)key withPath:(NSString *)path
{
    SCCTreeAdditionObj *treeObjInfo = [[SCCTreeAdditionObj alloc]
                                       initWithURL:nil
                                       withName:folderName
                                       withKey:key
                                       withPathRelativeToParent:path
                                       selectItsParent:NO];
    [self performAddFolder:treeObjInfo];
}

// -------------------------------------------------------------------------------
//	addFolder:folderName
// -------------------------------------------------------------------------------
- (void)addFolder:(NSString *)folderName
{
    [self addFolder:folderName withKey:[[XFKeyBuilder createUnique] build] withPath:nil];
}

- (void)addFolder:(NSString *)folderName withKey:(NSString *)key
{
    [self addFolder:folderName withKey:key withPath:nil];
}

// -------------------------------------------------------------------------------
//	addFolderAction:sender:
// -------------------------------------------------------------------------------
- (IBAction)addFolderAction:(id)sender
{
    [self addFolder:UNTITLED_NAME];
}


#pragma mark - Target


/**
 *  Adds a new target with a name.
 *
 *  @param aName The target name.
 *
 *  @return YES
 */
- (BOOL)addTarget:(NSString *)aName
{
    // Check if the target already exists.
    NSString *key = [[self document] addTarget:aName];
    
    
    if (key) {
        // NSTreeController inserts objects using NSIndexPath, so we need to calculate this
        NSIndexPath *indexPath = nil;
        NSUInteger indexArr[] = {1,0};
        indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2]; // drop at the end of the top level
        SCCChildNode *node = [[SCCChildNode alloc] init];
        node.nodeTitle = [aName copy];
        node.key = [key copy];
        // the user is adding a child node, tell the controller directly
        [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
        
        NSUInteger indexArrIn[] = {1,0,0};
        NSIndexPath *indexPathIn = [NSIndexPath indexPathWithIndexes:indexArrIn length:3];
        SCCChildNode *nodeIn = [[SCCChildNode alloc] init];
        nodeIn.nodeTitle = @"in";
        nodeIn.key = [key copy];
        [treeController insertObject:nodeIn atArrangedObjectIndexPath:indexPathIn];
        
        NSUInteger indexArrOut[] = {1,0,1};
        NSIndexPath *indexPathOut = [NSIndexPath indexPathWithIndexes:indexArrOut length:3];
        SCCChildNode *nodeOut = [[SCCChildNode alloc] init];
        nodeOut.nodeTitle = @"out";
        nodeOut.key = [key copy];
        [treeController insertObject:nodeOut atArrangedObjectIndexPath:indexPathOut];
        
        // Add the output files that are in the out directory of the target.
        // When we add a new target in XFProject, I add any default out files.
        // So, get those files and add those files in the outline view.
        // Get an array of dictionaries for the output files.
        for (NSDictionary *fileInfo in [[self document] outFilesOfTargetWithKey:key]) {
            SCCChildNode *node = [[SCCChildNode alloc] init];
            
            node.isLeaf = YES;
            node.nodeTitle = [[fileInfo objectForKey:@"name"] copy];
            node.urlString = [[fileInfo objectForKey:@"path"] copy];
            node.key = [[fileInfo objectForKey:@"key"] copy];
            NSUInteger indexArrOutFile[] = {1,0,1,0};
            NSIndexPath *indexPathOutFile = [NSIndexPath indexPathWithIndexes:indexArrOutFile length:4];
            [treeController insertObject:node atArrangedObjectIndexPath:indexPathOutFile];
        }
        
        return YES;
    } else {
        return NO;
    }
}


- (NSString *)input: (NSString *)prompt defaultValue: (NSString *)defaultValue {
    NSAlert *alert = [NSAlert alertWithMessageText: prompt
                                     defaultButton:@"OK"
                                   alternateButton:@"Cancel"
                                       otherButton:nil
                         informativeTextWithFormat:@""];
    
    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 24)];
    [input setStringValue:defaultValue];
//    [input autorelease];
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

-(void)showSimpleCriticalAlert
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Alert"];
    [alert setInformativeText:@"NSCriticalAlertStyle\rPlease enter different target name."];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert beginSheetModalForWindow:[[self view] window] modalDelegate:self
                     didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    /*
     The following options are deprecated in 10.9. Use NSAlertFirstButtonReturn instead
     NSAlertDefaultReturn = 1,
     NSAlertAlternateReturn = 0,
     NSAlertOtherReturn = -1,
     NSAlertErrorReturn = -2
     NSOKButton = 1, // NSModalResponseOK should be used
     NSCancelButton = 0 // NSModalResponseCancel should be used
     */
    if (returnCode == NSOKButton)
    {
        NSLog(@"(returnCode == NSOKButton)");
    }
    else if (returnCode == NSCancelButton)
    {
        NSLog(@"(returnCode == NSCancelButton)");
    }
    else if(returnCode == NSAlertFirstButtonReturn)
    {
        NSLog(@"if (returnCode == NSAlertFirstButtonReturn)");
    }
    else if (returnCode == NSAlertSecondButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertSecondButtonReturn)");
    }
    else if (returnCode == NSAlertThirdButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertThirdButtonReturn)");
    }
    else
    {
        NSLog(@"All Other return code %ld", (long)returnCode);
    }
}

/**
 *  Adds a new target.
 *
 *  @param sender The button.
 */
- (IBAction)addTargetAction:(id)sender
{
    // A simple window for getting the name of the target.
    NSString *targetName = [self input:@"Target name" defaultValue:@""];
    if (targetName && [targetName length] > 0) {
        if ([self addTarget:targetName]) {
            // Added a target with the name.
        } else {
            // Failed to add the target because it is already in the Xfast.
            [self showSimpleCriticalAlert];
        }
    }
}

// -------------------------------------------------------------------------------
//	performAddChild:treeAddition
// -------------------------------------------------------------------------------
- (void)performAddChild:(SCCTreeAdditionObj *)treeAddition
{
    if ([[treeController selectedObjects] count] > 0)
    {
        // we have a selection
        if ([[[treeController selectedObjects] objectAtIndex:0] isLeaf])
        {
            // trying to add a child to a selected leaf node, so select its parent for add
            [self selectParentFromSelection];
        }
    }
    
    // find the selection to insert our node
    NSIndexPath *indexPath;
    if ([[treeController selectedObjects] count] > 0)
    {
        // we have a selection, insert at the end of the selection
        indexPath = [treeController selectionIndexPath];
        indexPath = [indexPath indexPathByAddingIndex:[[[[treeController selectedObjects] objectAtIndex:0] children] count]];
    }
    else
    {
        // no selection, just add the child to the end of the tree
        indexPath = [NSIndexPath indexPathWithIndex:self.contents.count];
    }
    
    // create a leaf node
    SCCChildNode *node = [[SCCChildNode alloc] initLeaf];
    node.urlString = [treeAddition nodeURL];
    node.key = [treeAddition nodeKey];
    
    if ([treeAddition nodeURL])
    {
        if ([[treeAddition nodeURL] length] > 0)
        {
            // the child to insert has a valid URL, use its display name as the node title
            if ([treeAddition nodeName])
                node.nodeTitle = [treeAddition nodeName];
            else
                node.nodeTitle = [[NSFileManager defaultManager] displayNameAtPath:[node urlString]];
        }
        else
        {
            // the child to insert will be an empty URL
            node.nodeTitle = UNTITLED_NAME;
            node.urlString = HTTP_PREFIX;
        }
    }
    
    // the user is adding a child node, tell the controller directly
    [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
    
    // adding a child automatically becomes selected by NSOutlineView, so keep its parent selected
    if ([treeAddition selectItsParent])
    {
        [self selectParentFromSelection];
    }
}



- (void)addSeparator
{
    [self addChild:nil withName:nil withKey:nil withPathRelativeToParent:nil selectParent:YES];
}

// -------------------------------------------------------------------------------
//	addChild:url:withName:selectParent
// -------------------------------------------------------------------------------
- (void)addChild:(NSString *)url
        withName:(NSString *)nameStr
         withKey:(NSString *)key
withPathRelativeToParent:(NSString *)path
    selectParent:(BOOL)select
{
    SCCTreeAdditionObj *treeObjInfo = [[SCCTreeAdditionObj alloc]
                                       initWithURL:url
                                       withName:nameStr
                                       withKey:key
                                       withPathRelativeToParent:nil
                                       selectItsParent:select];
    [self performAddChild:treeObjInfo];
}

// -------------------------------------------------------------------------------
//	addEntries:discloseParent:
// -------------------------------------------------------------------------------
- (void)addEntries:(NSDictionary *)entries
    discloseParent:(BOOL)discloseParent
{
    for (id entry in entries)
    {
        if ([entry isKindOfClass:[NSDictionary class]])
        {
            NSString *urlStr = [entry objectForKey:KEY_URL];
            NSString *keyStr = [entry objectForKey:KEY_KEY];
            NSString *pathStr = [entry objectForKey:KEY_PATH];
            
            if ([entry objectForKey:KEY_SEPARATOR])
            {
                // its a separator mark, we treat is as a leaf
                [self addSeparator];
            }
            else if ([entry objectForKey:KEY_FOLDER])
            {
                // we treat file system folders as a leaf and show its contents in the NSCollectionView
                NSString *folderName = [entry objectForKey:KEY_FOLDER];
                [self addChild:urlStr
                      withName:folderName
                       withKey:keyStr
      withPathRelativeToParent:pathStr
                  selectParent:YES];
            }
            else if ([entry objectForKey:KEY_URL])
            {
                // its a leaf item with a URL
                NSString *nameStr = [entry objectForKey:KEY_NAME];
                [self addChild:urlStr
                      withName:nameStr
                       withKey:keyStr
      withPathRelativeToParent:pathStr
                  selectParent:YES];            }
            else
            {
                // it's a generic container
                NSString *folderName = [entry objectForKey:KEY_GROUP];
                [self addFolder:folderName withKey:keyStr];
                
                // add its children
                NSDictionary *newChildren = [entry objectForKey:KEY_ENTRIES];
                [self addEntries:newChildren discloseParent:NO];
                
                [self selectParentFromSelection];
            }
        }
    }
    
    if (!discloseParent)
    {
        // inserting children automatically expands its parent, we want to close it
        if ([[treeController selectedNodes] count] > 0)
        {
            NSTreeNode *lastSelectedNode = [[treeController selectedNodes] objectAtIndex:0];
            [myOutlineView collapseItem:lastSelectedNode];
        }
    }
}

// -------------------------------------------------------------------------------
//	populateOutline
//
//	Populate the tree controller from disk-based dictionary (Outline.dict)
// -------------------------------------------------------------------------------
- (void)populateOutline
{
    // add the "Bookmarks" section
    [self addFolder:BOOKMARKS_NAME];
    
    NSDictionary *initData = [NSDictionary dictionaryWithContentsOfFile:
                              [[NSBundle mainBundle] pathForResource:INITIAL_INFODICT ofType:@"plist"]];
    NSDictionary *entries = [initData objectForKey:KEY_ENTRIES];
    [self addEntries:entries discloseParent:YES];
    
    [self selectParentFromSelection];
}

- (void)addProject
{
    NSDictionary *entries0 = [[self projectContent] copy];
    [[self projectContent] removeAllObjects];
    
    [self addFolder:PROJECT_NAME];
    [self addEntries:entries0 discloseParent:YES];
    [[self projectContent] addObjectsFromArray:[[_contents lastObject] children]];
    
    [self selectParentFromSelection];
}

- (void)addTargets
{
    NSDictionary *entries0 = [[self targetContent] copy];
    [[self targetContent] removeAllObjects];
    
    [self addFolder:TARGETS_NAME];
    [self addEntries:entries0 discloseParent:YES];
    [[self targetContent] addObjectsFromArray:[[_contents lastObject] children]];
    
    [self selectParentFromSelection];
}


// -------------------------------------------------------------------------------
//	populateOutlineContents
// -------------------------------------------------------------------------------
- (void)populateOutlineContents
{
    // hide the outline view - don't show it as we are building the content
    [myOutlineView setHidden:YES];
    
//    [self addProject];
//    [self addTargets];
//    [self addFiles];
    
    // remove the current selection
    NSArray *selection = [treeController selectionIndexPaths];
    [treeController removeSelectionIndexPaths:selection];
    
    [myOutlineView setHidden:NO];	// we are done populating the outline view content, show it again
}



- (void)addProject:(NSMutableArray *)content
{
    NSDictionary *entries0 = [content copy];
    [content removeAllObjects];
    
    [self addFolder:PROJECT_NAME];
    [self addEntries:entries0 discloseParent:YES];
    
    [self selectParentFromSelection];
}

- (void)addTargets:(NSMutableArray *)content
{
    NSDictionary *entries0 = [content copy];
    [content removeAllObjects];
    
    [self addFolder:TARGETS_NAME];
    [self addEntries:entries0 discloseParent:YES];
    
    [self selectParentFromSelection];
}

- (void)addFiles:(NSMutableArray *)content
{
    NSDictionary *entries0 = [content copy];
    [content removeAllObjects];
//    [[self contents] removeAllObjects];
    
    [self addFolder:FILES_NAME];
    [self addEntries:entries0 discloseParent:YES];
    
    [self selectParentFromSelection];
}


- (void)populateOutlineContentsWithArray:(NSMutableArray *)content
{
    _nodeIsSelected = NO;
    
    [treeController setContent:nil];
    [[self contents] removeAllObjects];
    
    // hide the outline view - don't show it as we are building the content
    [myOutlineView setHidden:YES];
    
    [self addProject:[content objectAtIndex:0]];
    [self addTargets:[content objectAtIndex:1]];
    [self addFiles:[content objectAtIndex:2]];
    
    [content removeAllObjects];
    [content arrayByAddingObjectsFromArray:[treeController content]];
    
    // remove the current selection
    NSArray *selection = [treeController selectionIndexPaths];
    [treeController removeSelectionIndexPaths:selection];
    
    _nodeIsSelected = YES;
    
    NSUInteger indexArr[] = {0,0};
    NSIndexPath *indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2]; //
    [treeController setSelectionIndexPath:indexPath];
    
    [myOutlineView setHidden:NO];	// we are done populating the outline view content, show it again
    
}

- (NSMutableArray *)treeContent
{
    return [treeController content];
}


#pragma mark - Node checks

// -------------------------------------------------------------------------------
//	isSeparator:node
// -------------------------------------------------------------------------------
- (BOOL)isSeparator:(SCCBaseNode *)node
{
    return ([node nodeIcon] == nil && [[node nodeTitle] length] == 0);
}

/**
 *  Used to check whether an item can be selected or not.
 *
 *  @param groupNode A base node
 *
 *  @return YES if they are not selectable, NO otherwise
 */
- (BOOL)isSpecialGroup:(SCCBaseNode *)groupNode
{
    return ([groupNode nodeIcon] == nil &&
            (
             [[groupNode nodeTitle] isEqualToString:PROJECT_NAME] ||
             [[groupNode nodeTitle] isEqualToString:TARGETS_NAME] ||
             [[groupNode nodeTitle] isEqualToString:FILES_NAME] ||
             [[groupNode nodeTitle] isEqualToString:PLACES_NAME]
             )
            );
}


#pragma mark - Managing Views

// -------------------------------------------------------------------------------
//  contentReceived:notif
//
//  Notification sent from IconViewController class,
//  indicating the file system content has been received
// -------------------------------------------------------------------------------
- (void)contentReceived:(NSNotification *)notif
{
    [progIndicator setHidden:YES];
    [progIndicator stopAnimation:self];
}

// -------------------------------------------------------------------------------
//	removeSubview
// -------------------------------------------------------------------------------
//- (void)removeSubview
//{
//    // empty selection
//    NSArray *subViews = [placeHolderView subviews];
//    if ([subViews count] > 0)
//    {
//        [[subViews objectAtIndex:0] removeFromSuperview];
//    }
//
//    [placeHolderView displayIfNeeded];	// we want the removed views to disappear right away
//}

// -------------------------------------------------------------------------------
//	changeItemView
// ------------------------------------------------------------------------------
//- (void)changeItemView
//{
//    NSArray	*selection = [treeController selectedNodes];
//    if ([selection count] > 0)
//    {
//        SCCBaseNode *node = [[selection objectAtIndex:0] representedObject];
//        NSString *urlStr = [node urlString];
//        if (urlStr)
//        {
//            if ([urlStr hasPrefix:HTTP_PREFIX])
//            {
//                // 1) the url is a web-based url
//                //
//                if (currentView != webView)
//                {
//                    // change to web view
//                    [self removeSubview];
//                    currentView = nil;
//                    [placeHolderView addSubview:webView];
//                    currentView = webView;
//                }
//
//                // this will tell our WebUIDelegate not to retarget first responder since some web pages force
//                // forus to their text fields - we want to keep our outline view in focus.
//                retargetWebView = YES;
//
//                [webView setMainFrameURL:urlStr];	// re-target to the new url
//            }
//            else
//            {
//                // 2) the url is file-system based (folder or file)
//                //
//                if (currentView != [fileViewController view] || currentView != [iconViewController view])
//                {
//                    NSURL *targetURL = [NSURL fileURLWithPath:urlStr];
//
//                    NSURL *url = [NSURL fileURLWithPath:[node urlString]];
//
//                    // detect if the url is a directory
//                    NSNumber *isDirectory = nil;
//
//                    [url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
//                    if ([isDirectory boolValue])
//                    {
//                        // avoid a flicker effect by not removing the icon view if it is already embedded
//                        if (!(currentView == [iconViewController view]))
//                        {
//                            // remove the old subview
//                            [self removeSubview];
//                            currentView = nil;
//                        }
//
//                        // change to icon view to display folder contents
//                        [placeHolderView addSubview:[iconViewController view]];
//                        currentView = [iconViewController view];
//
//                        // its a directory - show its contents using NSCollectionView
//                        iconViewController.url = targetURL;
//
//                        // add a spinning progress gear in case populating the icon view takes too long
//                        [progIndicator setHidden:NO];
//                        [progIndicator startAnimation:self];
//
//                        // note: we will be notifed back to stop our progress indicator
//                        // as soon as iconViewController is done fetching its content.
//                    }
//                    else
//                    {
//                        // 3) its a file, just show the item info
//                        //
//                        // remove the old subview
//                        [self removeSubview];
//                        currentView = nil;
//
//                        // change to file view
//                        [placeHolderView addSubview:[fileViewController view]];
//                        currentView = [fileViewController view];
//
//                        // update the file's info
//                        fileViewController.url = targetURL;
//                    }
//                }
//            }
//
//            NSRect newBounds;
//            newBounds.origin.x = 0;
//            newBounds.origin.y = 0;
//            newBounds.size.width = [[currentView superview] frame].size.width;
//            newBounds.size.height = [[currentView superview] frame].size.height;
//            [currentView setFrame:[[currentView superview] frame]];
//
//            // make sure our added subview is placed and resizes correctly
//            [currentView setFrameOrigin:NSMakePoint(0,0)];
//            [currentView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
//        }
//        else
//        {
//            // there's no url associated with this node
//            // so a container was selected - no view to display
//            [self removeSubview];
//            currentView = nil;
//        }
//    }
//}


#pragma mark - NSOutlineViewDelegate

- (BOOL)outlineView:(NSOutlineView *)outlineView
   shouldSelectItem:(id)item;
{
    // don't allow special group nodes (Devices and Places) to be selected
    SCCBaseNode *node = [item representedObject];
    return (![self isSpecialGroup:node] && ![self isSeparator:node]);
}

// Draw the separator.
- (NSCell *)outlineView:(NSOutlineView *)outlineView
 dataCellForTableColumn:(NSTableColumn *)tableColumn
                   item:(id)item
{
    NSCell *returnCell = [tableColumn dataCell];
    
    if ([[tableColumn identifier] isEqualToString:COLUMNID_NAME])
    {
        // we are being asked for the cell for the single and only column
        SCCBaseNode *node = [item representedObject];
        if ([self isSeparator:node])
            returnCell = separatorCell;
    }
    
    return returnCell;
}

// Do not allow empty node names.
- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    if ([[fieldEditor string] length] == 0)
    {
        // don't allow empty node names
        return NO;
    }
    else
    {
        return YES;
    }
}

//	Decide to allow the edit of the given outline view "item".
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn
               item:(id)item
{
    BOOL result = YES;
    
    item = [item representedObject];
    if ([self isSpecialGroup:item])
    {
        result = NO; // don't allow special group nodes to be renamed
    }
    else
    {
        if ([[item urlString] isAbsolutePath])
            result = NO; // don't allow file system objects to be renamed
    }
    
    return result;
}

// Set the image in the outline.
- (void)outlineView:(NSOutlineView *)olv
    willDisplayCell:(NSCell*)cell
     forTableColumn:(NSTableColumn *)tableColumn
               item:(id)item
{
    if ([[tableColumn identifier] isEqualToString:COLUMNID_NAME])
    {
        // we are displaying the single and only column
        if ([cell isKindOfClass:[ImageAndTextCell class]])
        {
            item = [item representedObject];
            if (item)
            {
                if ([item isLeaf])
                {
                    // does it have a URL string?
                    NSString *urlStr = [item urlString];
                    if (urlStr)
                    {
                        NSImage *iconImage;
                        if ([[item urlString] hasPrefix:HTTP_PREFIX])
                            iconImage = urlImage;
                        else
                            iconImage = [[NSWorkspace sharedWorkspace] iconForFile:urlStr];
                        [item setNodeIcon:iconImage];
                    }
                    else
                    {
                        // it's a separator, don't bother with the icon
                    }
                }
                else
                {
                    // check if it's a special folder (DEVICES or PLACES), we don't want it to have an icon
                    if ([self isSpecialGroup:item])
                    {
                        [item setNodeIcon:nil];
                    }
                    else
                    {
                        // it's a folder, use the folderImage as its icon
                        [item setNodeIcon:folderImage];
                    }
                }
            }
            
            // set the cell's image
            [[item nodeIcon] setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
            [(ImageAndTextCell*)cell setImage:[item nodeIcon]];
        }
    }
}

// Send the message to window controller.
- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    // ask the tree controller for the current selection
    NSArray *selection = [treeController selectedObjects];
    if ([selection count] > 1)
    {
        NSLog(@"Content outline view: multiple selection - clear the right side view");
        //        [self removeSubview];
        //        currentView = nil;
    }
    else
    {
        if ([selection count] == 1 && [self nodeIsSelected] == YES)
        {
            // single selection
            NSLog(@"Content outline view: an item selected.");
            [[self document] showXfastProjectSettings:[[selection firstObject] key]];
            
//            [self willChangeValueForKey:@"nodeIsSelected"];
//            [self didChangeValueForKey:@"nodeIsSelected"];
        }
        else
        {
//            NSLog(@"Content outline view: there is no current selection - no view to display");
            //            [self removeSubview];
            //            currentView = nil;
        }
    }
}

// Draw the special row.
- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item
{
    return ([self isSpecialGroup:[item representedObject]] ? YES : NO);
}

#pragma mark - NSOutlineViewDataSource

// ----------------------------------------------------------------------------------------
// outlineView:writeItems:toPasteboard
// ----------------------------------------------------------------------------------------
- (BOOL)outlineView:(NSOutlineView *)ov writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pboard
{
    [pboard declareTypes:[NSArray arrayWithObjects:kNodesPBoardType, nil] owner:self];
    
    // keep track of this nodes for drag feedback in "validateDrop"
    self.dragNodesArray = items;
    
    // Only leaf nodes can be moved.
    NSTreeNode *n = [items firstObject];
    SCCChildNode *sourceNode = [n representedObject];
    if ([sourceNode isLeaf]) {
        return YES;
    } else {
        return NO;
    }
}

#pragma mark - NSOutlineView drag and drop

#pragma mark - NSDraggingSource

// ----------------------------------------------------------------------------------------
// draggingSourceOperationMaskForLocal <NSDraggingSource override>
// ----------------------------------------------------------------------------------------
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
    return NSDragOperationMove;
}

#pragma mark - NSOutlineViewDataSource

// -------------------------------------------------------------------------------
//	outlineView:validateDrop:proposedItem:proposedChildrenIndex:
//
//	This method is used by NSOutlineView to determine a valid drop target.
// -------------------------------------------------------------------------------
- (NSDragOperation)outlineView:(NSOutlineView *)ov
                  validateDrop:(id <NSDraggingInfo>)info
                  proposedItem:(id)item
            proposedChildIndex:(NSInteger)index
{
    NSDragOperation result = NSDragOperationNone;
    
    if (!item)
    {
        // no item to drop on
        result = NSDragOperationGeneric;
    }
    else
    {
        if ([self isSpecialGroup:[item representedObject]])
        {
            // don't allow dragging into special grouped sections (i.e. Devices and Places)
            result = NSDragOperationMove;
//            result = NSDragOperationNone;
        }
        else
        {
            if (index == -1)
            {
                // don't allow dropping on a child
                result = NSDragOperationNone;
//                result = NSDragOperationMove;
            }
            else
            {
                // drop location is a container
                result = NSDragOperationMove;
            }
        }
    }
    
    return result;
}

// -------------------------------------------------------------------------------
//	handleWebURLDrops:pboard:withIndexPath:
//
//	The user is dragging URLs from Safari.
// -------------------------------------------------------------------------------
// goshng
- (void)handleWebURLDrops:(NSPasteboard *)pboard withIndexPath:(NSIndexPath *)indexPath
{
    NSArray *pbArray = [pboard propertyListForType:@"WebURLsWithTitlesPboardType"];
    NSArray *urlArray = [pbArray objectAtIndex:0];
    NSArray *nameArray = [pbArray objectAtIndex:1];
    
    NSInteger i;
    for (i = ([urlArray count] - 1); i >=0; i--)
    {
        SCCChildNode *node = [[SCCChildNode alloc] init];
        
        node.isLeaf = YES;
        
        node.nodeTitle = [nameArray objectAtIndex:i];
        
        node.urlString = [urlArray objectAtIndex:i];
        [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
        
    }
}

// -------------------------------------------------------------------------------
//	handleInternalDrops:pboard:withIndexPath:
//
//	The user is doing an intra-app drag within the outline view.
// -------------------------------------------------------------------------------
- (void)handleInternalDrops:(NSPasteboard *)pboard withIndexPath:(NSIndexPath *)indexPath
{
    // user is doing an intra app drag within the outline view:
    //
    NSArray* newNodes = self.dragNodesArray;
    
    //
    // Save all.
    //
    
    // move the items to their new place (we do this backwards, otherwise they will end up in reverse order)
    NSInteger idx;
    for (idx = ([newNodes count] - 1); idx >= 0; idx--)
    {
        [treeController moveNode:[newNodes objectAtIndex:idx] toIndexPath:indexPath];
    }
    
    // keep the moved nodes selected
    NSMutableArray *indexPathList = [NSMutableArray array];
    for (NSUInteger i = 0; i < [newNodes count]; i++)
    {
        [indexPathList addObject:[[newNodes objectAtIndex:i] indexPath]];
    }
    [treeController setSelectionIndexPaths: indexPathList];
}

/**
 *  Copies input files to the input of a Xfast target.
 *
 */
- (void)handleInternalDropsCopy:(NSPasteboard *)pboard
                  withIndexPath:(NSIndexPath *)indexPath
                   withTreeNode:(NSTreeNode *)targetItem
{
    // user is doing an intra app drag within the outline view:
    //
    NSMutableArray* newNodes = [self.dragNodesArray mutableCopy];
    
    //
    // Check the duplcates.
    // Add the source files and update output build files as well.
    //
    SCCChildNode *targetNode = [targetItem representedObject];
    [[self document] addSourceFiles:newNodes toTarget:[targetNode key]];
    
    
    
    // Add the output build files.
    // The Xfast has the output files are ready. Use them to update or add
    // output build files to the outline view.
    // Remove the output files in the outline view.
    // Add all the output build files.
    NSIndexPath *outputIndexPath = [[[indexPath indexPathByRemovingLastIndex]
                                    indexPathByRemovingLastIndex]
                                    indexPathByAddingIndex:1];
//    [treeController setSelectionIndexPath:outputIndexPath];
//    NSTreeNode *outputTreeNode = [[treeController selectedNodes] firstObject];
    [treeController removeObjectAtArrangedObjectIndexPath:outputIndexPath];
    SCCChildNode *outputNode = [[SCCChildNode alloc] init];
    outputNode.nodeTitle = @"out";
    outputNode.key = [targetNode key];
    [treeController insertObject:outputNode atArrangedObjectIndexPath:outputIndexPath];

    //
    // Get output build files as an array of dictionary.
    //
    NSArray *outputBuildFiles = [[self document] xfastOutputBuildFilesWithTargetKey:[targetNode key]];
    NSUInteger iFile = 0;
    for (NSDictionary *file in outputBuildFiles) {
        NSIndexPath *fileIndexPath = [outputIndexPath indexPathByAddingIndex:iFile];
        SCCChildNode *node = [[SCCChildNode alloc] initLeaf];
        node.urlString = [file objectForKey:@"path"];
        node.nodeTitle = [node.urlString lastPathComponent];
        node.key = [file objectForKey:@"key"];
        [treeController insertObject:node atArrangedObjectIndexPath:fileIndexPath];
        iFile++;
    }
    
    
    //
    // move the items to their new place (we do this backwards, otherwise they will end up in reverse order)
    //
    NSInteger idx;
    for (idx = ([newNodes count] - 1); idx >= 0; idx--)
    {
        
        NSTreeNode *n = [newNodes objectAtIndex:idx];
        SCCChildNode *sourceNode = [n representedObject];
        SCCChildNode *node = [[SCCChildNode alloc] init];
        
//        NSURL *url = [sourceNode urlString];
//        NSString *name = [[NSFileManager defaultManager] displayNameAtPath:[url path]];
        node.isLeaf = YES;
        node.nodeTitle = [[sourceNode nodeTitle] copy];
        node.urlString = [[sourceNode urlString] copy];
        node.key = [[sourceNode key] copy];
        
        [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
//        [treeController moveNode:[newNodes objectAtIndex:idx] toIndexPath:indexPath];
    }
    
    // keep the moved nodes selected
    NSMutableArray *indexPathList = [NSMutableArray array];
    for (NSUInteger i = 0; i < [newNodes count]; i++)
    {
        [indexPathList addObject:[[newNodes objectAtIndex:i] indexPath]];
    }
    [treeController setSelectionIndexPaths: indexPathList];
}

/**
 *  Copies input files to the input of a Xfast target in the operation of
 * ManyToOne. 
 *
 *  We create the input file in the in directory, and the output file is given.
 * The output file must have been created upon the Xfast creation.
 *
 */
- (void)handleInternalDropsCopyManyToOne:(NSPasteboard *)pboard
                           withIndexPath:(NSIndexPath *)indexPath
                            withTreeNode:(NSTreeNode *)targetItem
{
    // user is doing an intra app drag within the outline view:
    //
    NSMutableArray* newNodes = [self.dragNodesArray mutableCopy];
    
    //
    // Check the duplcates.
    // Add the source files and update output build files as well.
    //
    SCCChildNode *targetNode = [targetItem representedObject];
    [[self document] addSourceFiles:newNodes toTarget:[targetNode key]];
    
    //
    // move the items to their new place (we do this backwards, otherwise they will end up in reverse order)
    //
    NSInteger idx;
    for (idx = ([newNodes count] - 1); idx >= 0; idx--)
    {
        
        NSTreeNode *n = [newNodes objectAtIndex:idx];
        SCCChildNode *sourceNode = [n representedObject];
        SCCChildNode *node = [[SCCChildNode alloc] init];
        
//        NSURL *url = [sourceNode urlString];
//        NSString *name = [[NSFileManager defaultManager] displayNameAtPath:[url path]];
        node.isLeaf = YES;
        node.nodeTitle = [[sourceNode nodeTitle] copy];
        node.urlString = [[sourceNode urlString] copy];
        node.key = [[sourceNode key] copy];
        
        [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
//        [treeController moveNode:[newNodes objectAtIndex:idx] toIndexPath:indexPath];
    }
    
    // keep the moved nodes selected
    NSMutableArray *indexPathList = [NSMutableArray array];
    for (NSUInteger i = 0; i < [newNodes count]; i++)
    {
        [indexPathList addObject:[[newNodes objectAtIndex:i] indexPath]];
    }
    [treeController setSelectionIndexPaths: indexPathList];
}

// -------------------------------------------------------------------------------
//	handleFileBasedDrops:pboard:withIndexPath:
//
//	The user is dragging file-system based objects (probably from Finder)
// -------------------------------------------------------------------------------
- (void)handleFileBasedDrops:(NSPasteboard *)pboard withIndexPath:(NSIndexPath *)indexPath
{
    NSMutableArray *fileNames = [pboard propertyListForType:NSFilenamesPboardType];
    
    // Check if the files are already in the file reference.
    // Filter out duplicates.
    NSArray *keys = [[self document] addFileReference:fileNames];
    
    if ([fileNames count] > 0)
    {
        NSInteger i;
        NSInteger count = [fileNames count];
        
        for (i = (count - 1); i >=0; i--)
        {
            SCCChildNode *node = [[SCCChildNode alloc] init];
            
            NSURL *url = [NSURL fileURLWithPath:[fileNames objectAtIndex:i]];
            NSString *name = [[NSFileManager defaultManager] displayNameAtPath:[url path]];
            node.isLeaf = YES;
            
            node.nodeTitle = name;
            node.urlString = [url path];
            node.key = [[keys objectAtIndex:i] copy];
            
            [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
        }
    }
    
    // Update the Xfast file.
    
}

- (IBAction)deleteItem:(id)sender
{
    NSLog(@"ContentOutlineView: deleteItem");
    for (NSIndexPath *indexPath in [treeController selectionIndexPaths]) {
        [treeController removeObjectAtArrangedObjectIndexPath:indexPath];
    }
}

- (IBAction)aaaDocument
{
//    [[[[self view] window] firstResponder] setNextResponder:[[self view] window]];
    NSLog(@"aaaDocument");
    
}

- (IBAction)aaaDocument:(id)sender
{
    NSLog(@"aaaDocument: %@", sender);
}

// -------------------------------------------------------------------------------
//	handleURLBasedDrops:pboard:withIndexPath:
//
//	Handle dropping a raw URL.
// -------------------------------------------------------------------------------
- (void)handleURLBasedDrops:(NSPasteboard *)pboard withIndexPath:(NSIndexPath *)indexPath
{
    NSURL *url = [NSURL URLFromPasteboard:pboard];
    if (url)
    {
        SCCChildNode *node = [[SCCChildNode alloc] init];
        
        if ([url isFileURL])
        {
            // url is file-based, use it's display name
            NSString *name = [[NSFileManager defaultManager] displayNameAtPath:[url path]];
            node.nodeTitle = name;
            node.urlString = [url path];
        }
        else
        {
            // url is non-file based (probably from Safari)
            //
            // the url might not end with a valid component name, use the best possible title from the URL
            if ([[[url path] pathComponents] count] == 1)
            {
                if ([[url absoluteString] hasPrefix:HTTP_PREFIX])
                {
                    // use the url portion without the prefix
                    NSRange prefixRange = [[url absoluteString] rangeOfString:HTTP_PREFIX];
                    NSRange newRange = NSMakeRange(prefixRange.length, [[url absoluteString] length]- prefixRange.length - 1);
                    node.nodeTitle = [[url absoluteString] substringWithRange:newRange];
                }
                else
                {
                    // prefix unknown, just use the url as its title
                    node.nodeTitle = [url absoluteString];
                }
            }
            else
            {
                // use the last portion of the URL as its title
                node.nodeTitle = [[url path] lastPathComponent];
            }
            
            node.urlString = [url absoluteString];
        }
        node.isLeaf = YES;
        
        [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
        
    }
}

- (NSIndexPath *)indexPath:(NSInteger)index targetItem:(id)targetItem
{
    NSIndexPath *indexPath;
    if (targetItem)
    {
        // drop down inside the tree node:
        // feth the index path to insert our dropped node
        indexPath = [[targetItem indexPath] indexPathByAddingIndex:index];
    }
    else
    {
        // drop at the top root level
        if (index == -1) {	// drop area might be ambibuous (not at a particular location)
            NSUInteger indexArr[] = {2,0};
            NSMutableArray *c = [[self treeController] content];
            //            indexPath = [NSIndexPath indexPathWithIndex:self.contents.count]; // drop at the end of the top level
            indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2]; // drop at the end of the top level
        } else {
            indexPath = [NSIndexPath indexPathWithIndex:index]; // drop at a particular place at the top level
        }
    }
    return indexPath;
}

// -------------------------------------------------------------------------------
//	outlineView:acceptDrop:item:childIndex
//
//	This method is called when the mouse is released over an outline view that previously decided to allow a drop
//	via the validateDrop method. The data source should incorporate the data from the dragging pasteboard at this time.
//	'index' is the location to insert the data as a child of 'item', and are the values previously set in the validateDrop: method.
//
// -------------------------------------------------------------------------------
- (BOOL)outlineView:(NSOutlineView*)ov
         acceptDrop:(id <NSDraggingInfo>)info
               item:(id)targetItem
         childIndex:(NSInteger)index
{
    // note that "targetItem" is a NSTreeNode proxy
    //
    BOOL result = NO;
    // find the index path to insert our dropped object(s)
    NSIndexPath *indexPath;
    
    NSPasteboard *pboard = [info draggingPasteboard];	// get the pasteboard

    if ([pboard availableTypeFromArray:[NSArray arrayWithObject:NSFilenamesPboardType]])
    {
        NSUInteger indexArr[] = {2,0};
        NSMutableArray *c = [[self treeController] content];
        //            indexPath = [NSIndexPath indexPathWithIndex:self.contents.count]; // drop at the end of the top level
        indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2]; // drop at the end of the top level
    }
    else if ([pboard availableTypeFromArray:[NSArray arrayWithObject:kNodesPBoardType]])
    {
        // Allow or disallow file move within the outline.
        NSTreeNode *n = [self.dragNodesArray firstObject];
        NSIndexPath *sourecIndexPath = [n indexPath];
        
        if ([sourecIndexPath indexAtPosition:0] == 0) {
            // From project
            return NO;
        } else if ([sourecIndexPath indexAtPosition:0] == 1) {
            // From target
            indexPath = [self indexPath:index targetItem:targetItem];
            
            if ([indexPath length] < 2) {
                return NO;
            }
            if ([indexPath indexAtPosition:0] == 2) {
                return NO;
            } else if ([indexPath indexAtPosition:0] == 0) {
                return NO;
            } else {
                // Move
                if ([sourecIndexPath indexAtPosition:0] == [indexPath indexAtPosition:0] &&
                    [sourecIndexPath indexAtPosition:1] == [indexPath indexAtPosition:1] &&
                    [sourecIndexPath indexAtPosition:2] == [indexPath indexAtPosition:2] &&
                    [sourecIndexPath indexAtPosition:2] == [indexPath indexAtPosition:2]) {
                    [self handleInternalDrops:pboard withIndexPath:indexPath];
                    return YES;
                }
                return NO;
            }

        } else {
            // From file
            
            // Assume the source is in FILES.
//            SCCChildNode *targetNode = [targetItem representedObject];
            
            indexPath = [self indexPath:index targetItem:targetItem];
            
            if ([indexPath length] < 2) {
                return NO;
            }
            if ([indexPath indexAtPosition:0] == 2) {
                // Move
                [self handleInternalDrops:pboard withIndexPath:indexPath];
                return YES;
            } else if ([indexPath indexAtPosition:0] == 0) {
                return NO;
            } else {
                // Copy
                if ([indexPath length] > 3 && [indexPath indexAtPosition:2] == 0) {
                    NSString *operation = [[self document] operationOfXfast];
                    if ([operation compare:@"ManyToOne"] == NSOrderedSame) {
                        [self handleInternalDropsCopyManyToOne:pboard
                                                 withIndexPath:indexPath
                                                  withTreeNode:targetItem];
                    } else if ([operation compare:@"OneToOne"] == NSOrderedSame) {
                        [self handleInternalDropsCopy:pboard
                                        withIndexPath:indexPath
                                         withTreeNode:targetItem];
                    } else {
                        // Error!
                        assert(0);
                    }
                    return YES;
                } else {
                    return NO;
                }
            }
        }
        
    }
    else
    {
        indexPath = [self indexPath:index targetItem:targetItem];
    }

    
//    NSPasteboard *pboard = [info draggingPasteboard];	// get the pasteboard
    
    // check the dragging type -
    if ([pboard availableTypeFromArray:[NSArray arrayWithObject:kNodesPBoardType]])
    {
        // user is doing an intra-app drag within the outline view
        [self handleInternalDrops:pboard withIndexPath:indexPath];
        result = YES;
    }
    else if ([pboard availableTypeFromArray:[NSArray arrayWithObject:@"WebURLsWithTitlesPboardType"]])
    {
        // the user is dragging URLs from Safari
        [self handleWebURLDrops:pboard withIndexPath:indexPath];
        result = YES;
    }
    else if ([pboard availableTypeFromArray:[NSArray arrayWithObject:NSFilenamesPboardType]])
    {
        // the user is dragging file-system based objects (probably from Finder)
        [self handleFileBasedDrops:pboard withIndexPath:indexPath];
        result = YES;
    }
    else if ([pboard availableTypeFromArray:[NSArray arrayWithObject:NSURLPboardType]])
    {
        // handle dropping a raw URL
        [self handleURLBasedDrops:pboard withIndexPath:indexPath];
        result = YES;
    }
    
    NSMutableArray *c = [[self treeController] content];
    
    return result;
}


#pragma mark - NSSplitViewDelegate

// -------------------------------------------------------------------------------
//	splitView:constrainMinCoordinate:
//
//	What you really have to do to set the minimum size of both subviews to kMinOutlineViewSplit points.
// -------------------------------------------------------------------------------
//- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedCoordinate ofSubviewAt:(int)index
//{
//    return proposedCoordinate + kMinOutlineViewSplit;
//}

// -------------------------------------------------------------------------------
//	splitView:constrainMaxCoordinate:
// -------------------------------------------------------------------------------
//- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedCoordinate ofSubviewAt:(int)index
//{
//    return proposedCoordinate - kMinOutlineViewSplit;
//}

// -------------------------------------------------------------------------------
//	splitView:resizeSubviewsWithOldSize:
//
//	Keep the left split pane from resizing as the user moves the divider line.
// -------------------------------------------------------------------------------
//- (void)splitView:(NSSplitView *)sender resizeSubviewsWithOldSize:(NSSize)oldSize
//{
//    NSRect newFrame = [sender frame]; // get the new size of the whole splitView
//    NSView *left = [[sender subviews] objectAtIndex:0];
//    NSRect leftFrame = [left frame];
//    NSView *right = [[sender subviews] objectAtIndex:1];
//    NSRect rightFrame = [right frame];
//
//    CGFloat dividerThickness = [sender dividerThickness];
//
//    leftFrame.size.height = newFrame.size.height;
//
//    rightFrame.size.width = newFrame.size.width - leftFrame.size.width - dividerThickness;
//    rightFrame.size.height = newFrame.size.height;
//    rightFrame.origin.x = leftFrame.size.width + dividerThickness;
//
//    [left setFrame:leftFrame];
//    [right setFrame:rightFrame];
//}


@end
