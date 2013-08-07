/*
     File: CustomTableViewController.m 
 Abstract: The view controller for displaying an NSTableView. 
  Version: 1.1
 */

#import "CustomTableViewController.h"

@implementation CustomTableViewController

@synthesize managedObjectContext;

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
	// add the default set of items to our table
	[myTableArray addObjects: [NSArray arrayWithObjects:
								[NSDictionary dictionaryWithObjectsAndKeys:
									@"Keith", @"name",
									nil],
								 [NSDictionary dictionaryWithObjectsAndKeys:
									@"Craig", @"name",
									nil],
								 [NSDictionary dictionaryWithObjectsAndKeys:
									@"Eric", @"name",
									nil],
								 nil]];
}

@end