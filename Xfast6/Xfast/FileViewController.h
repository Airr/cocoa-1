/*
     File: FileViewController.h 
 Abstract: Controller object for our file view.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

@interface FileViewController : NSViewController

@property (readwrite, strong) NSURL *url;

@end
