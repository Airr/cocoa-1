/*
     File: CustomImageViewController.h
 Abstract: The view controller for displaying images.
  Version: 1.1
 */

#import <Cocoa/Cocoa.h>

@interface CustomImageViewController : NSViewController
{
	IBOutlet NSImageView*	imageView;	// the image to display
	IBOutlet NSTextField*	textView;	// the image file name to display
}
@property NSManagedObjectContext *managedObjectContext;

- (IBAction)openImageAction:(id)sender;	// user clicked the 'Open' button
- (void)stopImage;
@end
