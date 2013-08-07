//
//  SCRunAnalysisViewController.m
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCRunAnalysisViewController.h"

@interface SCRunAnalysisViewController ()

@end

@implementation SCRunAnalysisViewController

@synthesize managedObjectContext;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)configureImage:(NSString*)imagePathStr
{
	// load the image from the given path string and set is to the NSImageView
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:imagePathStr];
	[imageView setImage:image];
}

- (void)awakeFromNib
{
	// load the default image from our bundle
	NSString* imagePathStr = [[NSBundle mainBundle] pathForResource:@"LakeDonPedro" ofType:@"jpg"];
	[self configureImage:imagePathStr];
}

@end
