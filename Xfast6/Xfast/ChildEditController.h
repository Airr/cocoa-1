/*
     File: ChildEditController.h 
 Abstract: Controller object for the edit sheet panel.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

@class MyWindowController;

@interface ChildEditController : NSWindowController

- (NSDictionary *)edit:(NSDictionary *)startingValues from:(MyWindowController *)sender;
- (BOOL)wasCancelled;

@end
