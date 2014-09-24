/*
     File: IconViewController.h 
 Abstract: Controller object for our icon collection view.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

// notification for indicating file system content has been received
extern NSString *kReceivedContentNotification;

@interface IconViewController : NSViewController

@property (readwrite, strong) NSURL *url;
	
@end
