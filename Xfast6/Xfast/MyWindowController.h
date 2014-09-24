/*
     File: MyWindowController.h 
 Abstract: Interface for MyWindowController class, the main controller class for this sample.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

@interface MyWindowController : NSWindowController

@property NSString *xfastName;

- (IBAction)create:(id)sender;
- (IBAction)cancel:(id)sender;

@end
