/*
     File: ATBasicTableViewWindowController.h 
 Abstract: Demonstrates the most basic dataSource/delegate implementation of a View Based NSTableView  
*/

#import <Cocoa/Cocoa.h>

@interface ATBasicTableViewWindowController : NSViewController<NSTableViewDelegate, NSTableViewDataSource> {
@private
    // An array of dictionaries that contain the contents to display
    NSMutableArray *_tableContents;
    IBOutlet NSTableView *_tableView;
}

@end
