/*
     File: ChildNode.h 
 Abstract: Generic child node object used with NSOutlineView and NSTreeController.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

#import "BaseNode.h"

@interface ChildNode : BaseNode
{
	NSString *description;
	NSTextStorage *text;
}

- (void)setDescription:(NSString *)newDescription;
- (NSString *)description;
- (void)setText:(id)newText;
- (NSTextStorage *)text;

@end
