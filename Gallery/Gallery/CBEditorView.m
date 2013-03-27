//
//  CBEditorView.m
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "CBEditorView.h"
#import "CBPhoto.h" 
#import "CBMainWindow.h"

@interface CBEditorView ()

@end

@implementation CBEditorView

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}
@synthesize mainWindowController; @synthesize imageView;

- (void) loadView {
    [super loadView];
    [self.imageView setImageWithURL:nil];
}

- (void) editPhoto: (CBPhoto*)photo {
    if ( self.view == nil ) [self loadView];
    NSURL* url = [NSURL fileURLWithPath:photo.filePath];
    [self.imageView setImageWithURL:url];
    [self.mainWindowController activateViewController:self];
}

- (void) dealloc {
    self.imageView = nil;
    [super dealloc];
}
@end
