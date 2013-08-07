/*
     File: CustomImageViewController.m 
 Abstract: The view controller for displaying images. 
  Version: 1.1 
 */

#import "CustomImageViewController.h"

@implementation CustomImageViewController

@synthesize managedObjectContext;

// -------------------------------------------------------------------------------
//	configureImage:
// -------------------------------------------------------------------------------
- (void)configureImage:(NSString*)imagePathStr
{
	// load the image from the given path string and set is to the NSImageView
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:imagePathStr];
	[imageView setImage:image];
	[textView setStringValue:[imagePathStr lastPathComponent]];	// display the file name
}

- (void)stopImage
{
	// load the default image from our bundle
	NSString* imagePathStr = [[NSBundle mainBundle] pathForResource:@"stop" ofType:@"jpg"];
	[self configureImage:imagePathStr];
}

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
	// load the default image from our bundle
	NSString* imagePathStr = [[NSBundle mainBundle] pathForResource:@"LakeDonPedro" ofType:@"jpg"];
	[self configureImage:imagePathStr];
}

// -------------------------------------------------------------------------------
//	openImageAction:sender
//
//	User clicked the "Open" button, open the NSOpenPanel to choose an image.
// -------------------------------------------------------------------------------
- (IBAction)openImageAction:(id)sender
{
	NSOpenPanel* openPanel = [NSOpenPanel openPanel];
	
	NSArray* fileTypes = [NSArray arrayWithObjects:@"jpg", @"gif", @"png", @"tiff", nil];
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setMessage:@"Choose an image file to display:"];
    [openPanel setAllowedFileTypes:fileTypes];
    [openPanel setDirectoryURL:[NSURL fileURLWithPath:@"/Library/Desktop Pictures/"]];
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        
        if (result == NSOKButton)
        {
            if ([[openPanel URL] isFileURL])
                [self configureImage:[[openPanel URL] path]];
        }
        
    }];
}

@end
