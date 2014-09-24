//
//  SCCProjectNavigatorViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

// We wish to call one of the methods of SCCProjectDocument class.
// view -> window -> windon controller -> document:
// I am not sure whether this is a good way to change the data.
//#import "SCCProjectDocument.h"

// Change the outline image in this delegate method.
// Set the image in the outline.
//- (void)outlineView:(NSOutlineView *)olv
//willDisplayCell:(NSCell*)cell
//forTableColumn:(NSTableColumn *)tableColumn
//item:(id)item

#import "XcodeEditor.h"
#import "SCCTreeAdditionObj.h"
#import "SCCProjectNavigatorViewController.h"
#import "SCCChildNode.h"
#import "ImageAndTextCell.h"
#import "SeparatorCell.h"
#import "SCCProjectDocument.h"

#define COLUMNID_NAME			@"NameColumn"	// the single column name in our outline view
#define INITIAL_INFODICT		@"outline"		// name of the dictionary file to populate our outline view

#define ICONVIEW_NIB_NAME		@"IconView"		// nib name for the icon view
#define FILEVIEW_NIB_NAME		@"FileView"		// nib name for the file view
#define CHILDEDIT_NAME			@"ChildEdit"	// nib name for the child edit window controller

#define UNTITLED_NAME			@"Untitled"		// default name for added folders and leafs

#define HTTP_PREFIX				@"http://"

// default folder titles
#define PROJECT_NAME			@"PROJECT"

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


@interface SCCProjectNavigatorViewController ()
{
    // cached images for generic folder and url document
    NSImage						*projectImage;
    NSImage						*folderImage;
    NSImage						*urlImage;
    SeparatorCell				*separatorCell;	// the cell used to draw a separator line in the outline view
}

@property (nonatomic,weak) SCCProjectDocument *document;

@property (strong) IBOutlet NSTreeController *treeController;
@property (weak) IBOutlet NSOutlineView *myOutlineView;
@property (weak) IBOutlet NSButton *addFolderButton;

@property NSArray *dragNodesArray;
@property (strong) NSMutableArray *contents;

// We first create a copy of the NSMutableArray from the group structure,
// which is used to populate the Navigator outline. We wanted the TreeController
// to bind to the NSMutableArray data in SCCProjectDocument so that we could
// use it to update the Xfast project file. However, strangely TreeController
// creates another NSMutableArray. The contents property does not point to
// the SCCProjectDocument's property. So, we have this keepContents property
// to keep the referenec to the SCCProjectDocument's property.
// Update 1. I removed bindings to contents by treeController. Then, I called
//           setContenet. Since then, the contents points to the same address.
// Update 2. We would call the document method directly not using addObserver.
//           Then, we may not need this keepContents.
@property NSMutableArray *keepContents;

@property BOOL nodeIsSelected;

@end

#pragma mark -

@implementation SCCProjectNavigatorViewController

@synthesize treeController;
@synthesize myOutlineView;
@synthesize addFolderButton;

/**
 *  Inits the receiver. This is the Designated Initializer.
 *
 *  @param nibNameOrNil   nib name
 *  @param nibBundleOrNil nib bundle
 *
 *  @return The initialized object.
 */
- (instancetype)initWithNibName:(NSString *)nibNameOrNil
                         bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCProjectNavigatorViewController"
                           bundle:nibBundleOrNil];
    if (self) {
        _contents = [[NSMutableArray alloc] init];
        
        // cache the reused icon images
        folderImage = [[NSWorkspace sharedWorkspace]
                       iconForFileType:NSFileTypeForHFSTypeCode(kGenericFolderIcon)];
        [folderImage setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
        
        urlImage = [[NSWorkspace sharedWorkspace]
                    iconForFileType:NSFileTypeForHFSTypeCode(kGenericURLIcon)];
        [urlImage setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
        
        projectImage = [[NSWorkspace sharedWorkspace]
                        iconForFileType:NSFileTypeForHFSTypeCode(kGenericApplicationIcon)];
        [projectImage setSize:NSMakeSize(kIconImageSize, kIconImageSize)];
    }
    return self;
}

/**
 *  Returns an object initialized with the content for the
 * outline view.
 *
 *  @param contents The array for the initial data for the outline view,
 *
 *  @return The initialized object.
 */
- (instancetype)initWithContent:(NSMutableArray *)contents
{
    self = [self initWithNibName:@"SCCProjectNavigatorViewController"
                          bundle:nil];
    if (self) {
        _keepContents = contents;
        _nodeIsSelected = YES;
    }
    return self;
}

/**
 *  Prepares the receiver for service after it has been loaded from a nib file.
 */
- (void)awakeFromNib
{
    // apply our custom ImageAndTextCell for rendering the first column's cells
    NSTableColumn *tableColumn = [myOutlineView tableColumnWithIdentifier:COLUMNID_NAME];
    ImageAndTextCell *imageAndTextCell = [[ImageAndTextCell alloc] init];
    [imageAndTextCell setEditable:YES];
    [tableColumn setDataCell:imageAndTextCell];
    
    // prepare the separator
    separatorCell = [[SeparatorCell alloc] init];
    [separatorCell setEditable:NO];
    
    // add our content
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
    [myOutlineView expandItem:nil expandChildren:YES];
    [treeController setSelectsInsertedObjects:YES];
    
    //
    NSUInteger indexArr[] = {0,0};
    NSIndexPath *indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:1];
    [treeController setSelectionIndexPath:indexPath];
    indexPath = [NSIndexPath indexPathWithIndexes:indexArr length:2];
    [treeController setSelectionIndexPath:indexPath];
    [self willChangeValueForKey:@"nodeIsSelected"];
    [self didChangeValueForKey:@"nodeIsSelected"];
    
}

- (SCCProjectDocument *)document
{
    if (_document == nil) {
        _document = [[[[self view] window] windowController] document];
    }
    return _document;
}

#pragma mark - Actions

- (void)populateOutlineContents
{
    // hide the outline view - don't show it as we are building the content
    [myOutlineView setHidden:YES];
    
    // add the "Bookmarks" section
    NSDictionary *entries0 = [_keepContents copy];
    [_keepContents removeAllObjects];
    [treeController setContent:_contents];
    
    NSIndexPath *currentIndexPath = [NSIndexPath indexPathWithIndex:0];
    [self addFolder0:PROJECT_NAME withIndexPath:currentIndexPath];
    [self addEntries0:entries0 discloseParent:YES withIndexPath:currentIndexPath];
    [_keepContents addObjectsFromArray:[[_contents firstObject] children]];
    
//    [self selectParentFromSelection];
    
    
    // we are done populating the outline view content, show it again
    [myOutlineView setHidden:NO];
    
    NSIndexPath *projectIndexPath = [currentIndexPath indexPathByAddingIndex:0];
    [treeController setSelectionIndexPath:projectIndexPath];
    
//    [self willChangeValueForKey:@"nodeIsSelected"];
//    [self didChangeValueForKey:@"nodeIsSelected"];
}

#pragma mark - Add Folders, Xfast, and Products


// -------------------------------------------------------------------------------
//	addFolder:folderName
// -------------------------------------------------------------------------------
- (void)addFolder0:(NSString *)folderName withIndexPath:(NSIndexPath *)currentIndexPath
{
    [self addFolder0:folderName
             withKey:[[XCKeyBuilder createUnique] build]
            withPath:nil
       withIndexPath:currentIndexPath];
}


- (void)addFolder0:(NSString *)folderName withKey:(NSString *)key withPath:(NSString *)path
     withIndexPath:(NSIndexPath *)currentIndexPath
{
    SCCTreeAdditionObj *treeObjInfo = [[SCCTreeAdditionObj alloc]
                                       initWithURL:nil
                                       withName:folderName
                                       withKey:key
                                       withPathRelativeToParent:path
                                       selectItsParent:NO];
    [self performAddFolder0:treeObjInfo withIndexPath:currentIndexPath];
}



// -------------------------------------------------------------------------------
//	performAddFolder:treeAddition
// -------------------------------------------------------------------------------
- (void)performAddFolder0:(SCCTreeAdditionObj *)treeAddition
            withIndexPath:(NSIndexPath *)currentIndexPath
{
    SCCChildNode *node = [[SCCChildNode alloc] init];
    node.nodeTitle = [treeAddition nodeName];
    node.key = [treeAddition nodeKey];
    [treeController insertObject:node atArrangedObjectIndexPath:currentIndexPath];
}




// -------------------------------------------------------------------------------
//	addEntries:discloseParent:
// -------------------------------------------------------------------------------
- (void)addEntries0:(NSDictionary *)entries
    discloseParent:(BOOL)discloseParent
      withIndexPath:(NSIndexPath *)currentIndexPath
{
    NSUInteger indexAppended = 0;
    for (id entry in entries)
    {
        NSIndexPath *itemIndexPath = [currentIndexPath indexPathByAddingIndex:indexAppended];
//        NSLog(@"indexPath: %@", itemIndexPath);
        if ([entry isKindOfClass:[NSDictionary class]])
        {
            NSString *urlStr = [entry objectForKey:KEY_URL];
            NSString *keyStr = [entry objectForKey:KEY_KEY];
            NSString *pathStr = [entry objectForKey:KEY_PATH];
            
            if ([entry objectForKey:KEY_SEPARATOR])
            {
                // its a separator mark, we treat is as a leaf
                [self addSeparatorWithIndexPath:itemIndexPath];
            }
            else if ([entry objectForKey:KEY_FOLDER])
            {
                // we treat file system folders as a leaf and show its contents in the NSCollectionView
                
                NSString *folderName = [entry objectForKey:KEY_FOLDER];
                
                [self addChild0:urlStr
                      withName:folderName
                       withKey:keyStr
      withPathRelativeToParent:pathStr
                  selectParent:YES
                 withIndexPath:itemIndexPath];
            }
            else if ([entry objectForKey:KEY_URL])
            {
                // its a leaf item with a URL
                NSString *nameStr = [entry objectForKey:KEY_NAME];
                [self addChild0:urlStr
                      withName:nameStr
                       withKey:keyStr
      withPathRelativeToParent:pathStr
                  selectParent:YES
                 withIndexPath:itemIndexPath];
            }
            else
            {
                // it's a generic container
                NSString *folderName = [entry objectForKey:KEY_GROUP];
                [self addFolder0:folderName withKey:keyStr withPath:pathStr
                   withIndexPath:itemIndexPath];
                
                // add its children
                NSDictionary *newChildren = [entry objectForKey:KEY_ENTRIES];
                [self addEntries0:newChildren discloseParent:NO
                    withIndexPath:itemIndexPath];
                
//                [self selectParentFromSelection];
            }
        }
        indexAppended++;
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
//	performAddChild:treeAddition
// -------------------------------------------------------------------------------
- (void)performAddChild0:(SCCTreeAdditionObj *)treeAddition
           withIndexPath:(NSIndexPath *)currentIndexPath
{
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
    [treeController insertObject:node atArrangedObjectIndexPath:currentIndexPath];
}

- (void)addSeparatorWithIndexPath:(NSIndexPath *)currentIndexPath
{
    [self addChild0:nil withName:nil withKey:nil withPathRelativeToParent:nil selectParent:YES
      withIndexPath:currentIndexPath];
}

// -------------------------------------------------------------------------------
//	addChild:url:withName:selectParent
// -------------------------------------------------------------------------------
- (void)addChild0:(NSString *)url
        withName:(NSString *)nameStr
         withKey:(NSString *)key
withPathRelativeToParent:(NSString *)path
    selectParent:(BOOL)select
    withIndexPath:(NSIndexPath *)currentIndexPath
{
    SCCTreeAdditionObj *treeObjInfo = [[SCCTreeAdditionObj alloc]
                                       initWithURL:url
                                       withName:nameStr
                                       withKey:key
                                       withPathRelativeToParent:nil
                                       selectItsParent:select];
    [self performAddChild0:treeObjInfo withIndexPath:currentIndexPath];
}


#pragma mark - Trash

- (void)populateOutlineContents0
{
    // hide the outline view - don't show it as we are building the content
    [myOutlineView setHidden:YES];
    
    [self populateNavigatorOutline];
    
    // remove the current selection
    NSArray *selection = [treeController selectionIndexPaths];
    [treeController removeSelectionIndexPaths:selection];
    
    
    [myOutlineView setHidden:NO];	// we are done populating the outline view content, show it again
}



#pragma mark - Interface

/**
 *  Returns the selected key.
 *
 *  @return The key string.
 */
- (NSString *)selectedKey
{
    NSArray *selection = [treeController selectedObjects];
    if ([selection count] == 1) {
        return [[selection firstObject] key];
    } else {
        return nil;
    }
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
    //    node.urlString = [treeAddition nodePathRelativeToParent];
    
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


- (void)addFolder:(NSString *)folderName
{
    [self addFolder:folderName withKey:[[XCKeyBuilder createUnique] build] withPath:nil];
}


// -------------------------------------------------------------------------------
//	addFolderAction:sender:
// -------------------------------------------------------------------------------
- (IBAction)addFolderAction:(id)sender
{
    [self addFolder:UNTITLED_NAME];
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
                  selectParent:YES];
            }
            else
            {
                // it's a generic container
                NSString *folderName = [entry objectForKey:KEY_GROUP];
                [self addFolder:folderName withKey:keyStr withPath:pathStr];
                
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

- (void)populateNavigatorOutline
{
    NSDictionary *entries0 = [_keepContents copy];
    [_keepContents removeAllObjects];
    
    // add the "Bookmarks" section
    [treeController setContent:_contents];
    [self addFolder:PROJECT_NAME];
    [self addEntries:entries0 discloseParent:YES];
    
    [_keepContents addObjectsFromArray:[[_contents firstObject] children]];
    
//    [_keepContents addObjectsFromArray:_contents];
    
    [self selectParentFromSelection];
}

#pragma mark - Add Xfast

/**
 *  Add a new Xfast file.
 *
 *  @param type The Xfast type
 *  @param name The Xfast name
 */
- (void)addXfast:(NSString *)type withName:(NSString *)name withKey:(NSString *)key
{
    NSString *xfastName = [NSString stringWithFormat:@"%@.%@.xfast", name, type];

    [self addChild:xfastName // need this name for URL
          withName:xfastName
           withKey:key
withPathRelativeToParent:nil
      selectParent:YES];
    
    [[self document] pushProject];
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
            [[groupNode nodeTitle] isEqualToString:PROJECT_NAME]);
}


- (BOOL)isProjectGroup:(SCCBaseNode *)groupNode
{
    return ([groupNode nodeIcon] == nil &&
            [[groupNode nodeTitle] isEqualToString:PROJECT_NAME]);
}


#pragma mark - Managing Views

// -------------------------------------------------------------------------------
//  contentReceived:notif
//
//  Notification sent from IconViewController class,
//  indicating the file system content has been received
// -------------------------------------------------------------------------------
//- (void)contentReceived:(NSNotification *)notif
//{
//    [progIndicator setHidden:YES];
//    [progIndicator stopAnimation:self];
//}
//


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
- (BOOL)outlineView:(NSOutlineView *)outlineView
shouldEditTableColumn:(NSTableColumn *)tableColumn
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
                        else if ([[item urlString] hasSuffix:@".xfast"])
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
                    else if ([self isProjectGroup:item])
                    {
                        [item setNodeIcon:projectImage];
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
        NSLog(@"Project Navigator: multiple selection - clear the right side view");
//        [self removeSubview];
//        currentView = nil;
    }
    else
    {
        if ([selection count] == 1 && [self nodeIsSelected] == YES)
        {
            // single selection
//            NSLog(@"Project Navigator View: an item selected.");
//            [self willChangeValueForKey:@"nodeIsSelected"];
//            [self didChangeValueForKey:@"nodeIsSelected"];
            [[self document] showProjectNavigatorView];
        }
        else
        {
//            NSLog(@"Project Navigator: no selection - no view to display");
//            [self willChangeValueForKey:@"nodeIsSelected"];
//            [self didChangeValueForKey:@"nodeIsSelected"];
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
    
    return YES;
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
            result = NSDragOperationNone;
        }
        else
        {
            if (index == -1)
            {
                // don't allow dropping on a child
                result = NSDragOperationNone;
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

// -------------------------------------------------------------------------------
//	handleFileBasedDrops:pboard:withIndexPath:
//
//	The user is dragging file-system based objects (probably from Finder)
// -------------------------------------------------------------------------------
- (void)handleFileBasedDrops:(NSPasteboard *)pboard withIndexPath:(NSIndexPath *)indexPath
{
    NSArray *fileNames = [pboard propertyListForType:NSFilenamesPboardType];
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
            
            [treeController insertObject:node atArrangedObjectIndexPath:indexPath];
            
        }
    }
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
    if (targetItem)
    {
        // drop down inside the tree node:
        // feth the index path to insert our dropped node
        indexPath = [[targetItem indexPath] indexPathByAddingIndex:index];
    }
    else
    {
        // drop at the top root level
        if (index == -1)	// drop area might be ambibuous (not at a particular location)
            indexPath = [NSIndexPath indexPathWithIndex:self.contents.count]; // drop at the end of the top level
        else
            indexPath = [NSIndexPath indexPathWithIndex:index]; // drop at a particular place at the top level
    }
    
    NSPasteboard *pboard = [info draggingPasteboard];	// get the pasteboard
    
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
    
    if (result == YES) {
        [[self document] pushProject];
    }
    
    return result;
}

#pragma mark - Action

- (IBAction)delete:(id)sender {
    NSLog(@"ProjectNavigatorView: delete");
    for (NSIndexPath *indexPath in [treeController selectionIndexPaths]) {
        [treeController removeObjectAtArrangedObjectIndexPath:indexPath];
    }
    // Save the content.
    [[self document] pushProject];
}

@end











































