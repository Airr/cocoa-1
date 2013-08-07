//
//  SCStopAnalysisViewController.h
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCStopAnalysisViewController : NSViewController
{
	IBOutlet NSImageView*	imageView;	// the image to display
}
@property NSManagedObjectContext *managedObjectContext;

@end
