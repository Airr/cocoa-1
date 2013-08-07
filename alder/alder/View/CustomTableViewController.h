/*
     File: CustomTableViewController.h
 Abstract: The view controller for displaying an NSTableView.
  Version: 1.1 
 */

#import <Cocoa/Cocoa.h>

@interface CustomTableViewController : NSViewController
{
	IBOutlet NSArrayController* myTableArray;
}
@property NSManagedObjectContext *managedObjectContext;

@end
