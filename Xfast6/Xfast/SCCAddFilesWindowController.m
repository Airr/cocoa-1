//
//  SCCAddFilesWindowController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/9/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCAddFilesWindowController.h"
#import "XfastEditor.h"

@interface SCCAddFilesWindowController ()

@property (weak) IBOutlet NSOutlineView *xfastOutlineView;
@property (nonatomic) NSMutableArray *sources;

@end

@implementation SCCAddFilesWindowController


- (id)init
{
    self = [super initWithWindowNibName:@"SCCAddFilesWindowController"];
    // TODO: Change the name of your NIB
    return self;
}

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
        _sources = [NSMutableArray array];
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (IBAction)add:(id)sender
{
    NSLog(@"controllel add");
    
    XFTarget *target = [_xfastProject currentTarget];
    [[_xfastOutlineView selectedRowIndexes] enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        //... do something with idx
        // *stop = YES; to stop iteration early
//        NSLog(@"idx: %lu", (unsigned long)idx);
        XFGroup *item = [_xfastOutlineView itemAtRow:idx];
        NSLog(@"Selected file: %@", [item key]);
        XFSourceFile *sourceFile = [_xfastProject fileWithKey:[item key]];
        NSLog(@"sourceFile");
        [target addMember:sourceFile];
        NSLog(@"Added: %@", sourceFile);
    }];
    
//    XCSourceFile* sourceFile = [project fileWithName:@"MyNewClass.m"];
//    XCTarget* examples = [project targetWithName:@"Examples"];
//    [examples addMember:sourceFile];
    NSLog(@"B: xfastProject save");
    [_xfastProject save];
    NSLog(@"E: xfastProject save");
    
    [self willChangeValueForKey:@"xfastProject"];
    [self didChangeValueForKey:@"xfastProject"];
    
//    for (id object in _sources) {
//        // do something with object
//        NSLog(@"File key: %@", object);
//    }
//
    
    [[self window] close];
    
}

- (IBAction)cancel:(id)sender
{
    NSLog(@"controllel cancel");
    [[self window] close];

}

#pragma mark *** Overrides of outlineView Protocols ***

- (NSInteger)outlineView:(NSOutlineView *)outlineView
  numberOfChildrenOfItem:(id)item
{

    if (item == nil) {
        XFGroup *group = [_xfastProject rootGroup];
        return [[group members] count];
    } else {
        NSLog(@"group: %@", (XFGroup*)item);
        return [((XFGroup *)item).members count];
    }

}

- (id)outlineView:(NSOutlineView *)outlineView
            child:(NSInteger)index
           ofItem:(id)item
{
   
    if (item == nil) {
        XFGroup *group = [_xfastProject rootGroup];
        return [[group members] objectAtIndex:index];
    } else {
        return [((XFGroup *)item).members objectAtIndex:index];
    }

}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    
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

- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
           byItem:(id)item
{
            NSLog(@"XfastOutline - %@\n", item);
    return item;

}

- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    NSOutlineView *theOutline =[notification object];
    
    if ([theOutline selectedRow] != -1) {
        XFGroup *item = [theOutline itemAtRow:[theOutline selectedRow]];
        
        NSString *filePath = [[[_xfastProject filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:[item pathRelativeToProjectRoot]];
        
        NSLog(@"selected outline view 2 %@: %@", [item displayName], filePath);
        NSLog(@"Extension is %@", [[item displayName] pathExtension]);
        if ([item groupMemberType] == XFPBXFileReferenceType) {
            NSLog(@"Absolute Path: %@", [item absolutePath]);
        }
        
 //       NSLog(@"selected row: %ld", (long)[theOutline selectedRow]);
 //       [_sources addObject:[item key]];
    }
}


@end
