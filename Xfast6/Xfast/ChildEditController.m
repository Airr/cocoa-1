/*
     File: ChildEditController.m 
 Abstract: Controller object for the edit sheet panel.
  
  Version: 1.5 
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import "ChildEditController.h"
#import "MyWindowController.h"

@interface ChildEditController ()
{
	BOOL cancelled;
	NSDictionary *savedFields;
	
	IBOutlet NSButton *doneButton;
	IBOutlet NSButton *cancelButton;
	IBOutlet NSForm *editForm;
}
@end

@implementation ChildEditController

// -------------------------------------------------------------------------------
//	windowNibName
// -------------------------------------------------------------------------------
- (NSString *)windowNibName
{
	return @"ChildEdit";
}

// -------------------------------------------------------------------------------
//	edit:startingValues:from
// -------------------------------------------------------------------------------
- (NSDictionary *)edit:(NSDictionary *)startingValues from:(MyWindowController *)sender
{
	NSWindow *window = [self window];

	cancelled = NO;

	NSArray *editFields = [editForm cells];
	if (startingValues != nil)
	{
		// we are editing current entry, use its values as the default
		savedFields = startingValues;

		[[editFields objectAtIndex:0] setStringValue:[startingValues objectForKey:@"name"]];
		[[editFields objectAtIndex:1] setStringValue:[startingValues objectForKey:@"url"]];
	}
	else
	{
		// we are adding a new entry,
		// make sure the form fields are empty due to the fact that this controller is recycled
		// each time the user opens the sheet -
        //
		[[editFields objectAtIndex:0] setStringValue:@""];
		[[editFields objectAtIndex:1] setStringValue:@""];
	}
	
	[NSApp beginSheet:window modalForWindow:[sender window] modalDelegate:nil didEndSelector:nil contextInfo:nil];
	[NSApp runModalForWindow:window];
	// sheet is up here...

	[NSApp endSheet:window];
	[window orderOut:self];

	return savedFields;
}

// -------------------------------------------------------------------------------
//	done:sender
// -------------------------------------------------------------------------------
- (IBAction)done:(id)sender
{
	NSArray* editFields = [editForm cells];
	if ([[[editFields objectAtIndex:1] stringValue] length] == 0)
	{
		// you must provide a URL
		NSBeep();
		return;
	}
	
	// save the values for later
	
	NSString* urlStr;
	if (![[[editFields objectAtIndex:1] stringValue] hasPrefix:@"http://"])
	{
		urlStr = [NSString stringWithFormat:@"http://%@", [[editFields objectAtIndex:1] stringValue]];
	}
	else
	{
		urlStr = [[editFields objectAtIndex:1] stringValue];
	}
	savedFields = [NSMutableDictionary dictionaryWithObjectsAndKeys:
							 [[editFields objectAtIndex:0] stringValue], @"name",
							 urlStr, @"url",
							 nil];
	
	[NSApp stopModal];
}

// -------------------------------------------------------------------------------
//	cancel:sender
// -------------------------------------------------------------------------------
- (IBAction)cancel:(id)sender
{
	[NSApp stopModal];
	cancelled = YES;
}

// -------------------------------------------------------------------------------
//	wasCancelled:
// -------------------------------------------------------------------------------
- (BOOL)wasCancelled
{
	return cancelled;
}

@end