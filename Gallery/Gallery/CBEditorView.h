//
//  CBEditorView.h
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
@class CBMainWindow; @class CBPhoto;

@interface CBEditorView : NSViewController
@property (assign) CBMainWindow* mainWindowController; @property (retain) IBOutlet IKImageView* imageView;
- (void) editPhoto: (CBPhoto*)photo;
@end
