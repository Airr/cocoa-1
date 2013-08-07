/*
     File: ATTableCellView.h 
 Abstract: A basic subclass of NSTableCellView that adds some properties strictly for allowing access to the items in code.
*/

#import <Cocoa/Cocoa.h>

@class ATColorView;

@interface ATTableCellView : NSTableCellView {
@private
    IBOutlet NSTextField *subTitleTextField;
    IBOutlet ATColorView *colorView;
    IBOutlet NSProgressIndicator *progessIndicator;
    IBOutlet NSButton *removeButton;
    BOOL _isSmallSize;
}

@property(assign) NSTextField *subTitleTextField;
@property(assign) ATColorView *colorView;
@property(assign) NSProgressIndicator *progessIndicator;

- (void)layoutViewsForSmallSize:(BOOL)smallSize animated:(BOOL)animated;

@end
