/*
     File: ChildNode.m 
 Abstract: Generic child node object used with NSOutlineView and NSTreeController.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import "ChildNode.h"

@implementation ChildNode

// -------------------------------------------------------------------------------
//	init:
// -------------------------------------------------------------------------------
- (id)init
{
	self = [super init];
	if (self)
    {
		self.nodeTitle = @"";
		description = @"- empty description -";
		text = [[NSTextStorage alloc] init];
	}
	return self;
}

// -------------------------------------------------------------------------------
//	setDescription:newDescription:
// -------------------------------------------------------------------------------
- (void)setDescription:(NSString *)newDescription
{
	description = newDescription;
}

// -------------------------------------------------------------------------------
//	description
// -------------------------------------------------------------------------------
- (NSString *)description
{
	return description;
}

// -------------------------------------------------------------------------------
//	setText:newText:
// -------------------------------------------------------------------------------
- (void)setText:(id)newText
{
	if ([newText isKindOfClass:[NSAttributedString class]])
		[text replaceCharactersInRange:NSMakeRange(0, [text length]) withAttributedString:newText];
	else
		[text replaceCharactersInRange:NSMakeRange(0, [text length]) withString:newText];
}

// -------------------------------------------------------------------------------
//	text:
// -------------------------------------------------------------------------------
- (NSTextStorage *)text
{
	return text;
}

// -------------------------------------------------------------------------------
//	mutableKeys:
//
//	Maintain support for archiving and copying.
// -------------------------------------------------------------------------------
- (NSArray *)mutableKeys
{
	return [[super mutableKeys] arrayByAddingObjectsFromArray:
				[NSArray arrayWithObjects:@"description", @"text", nil]];
}

@end
