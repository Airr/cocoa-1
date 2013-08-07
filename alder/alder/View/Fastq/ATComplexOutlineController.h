/*
     File: ATComplexOutlineController.h 
 Abstract: 
 The main controller for the "Complex Outline View" example window.
*/

#import <Cocoa/Cocoa.h>

#import "ATDesktopEntity.h"

@interface ATComplexOutlineController : NSWindowController {
@private
    ATDesktopFolderEntity *_rootContents;
    IBOutlet NSOutlineView *_outlineView;
    IBOutlet NSPathControl *_pathCtrlRootDirectory;
    IBOutlet NSDateFormatter *_sharedDateFormatter;
    IBOutlet NSTextField *_txtFldChildrenLimit;
    
    ATDesktopEntity *_itemBeingDragged;
}

- (void)pathCtrlValueChanged:(id)sender;
- (IBAction)btnInCellClicked:(id)sender;

- (IBAction)btnDeleteRowClicked:(id)sender;
- (IBAction)btnDeletedSelectedRowsClicked:(id)sender;   

- (IBAction)btnDemoMove:(id)sender;
- (IBAction)btnDemoBatchedMoves:(id)sender;

- (IBAction)chkbxFloatGroupRowsClicked:(id)sender;

- (IBAction)clrWellChanged:(id)sender;

@end
