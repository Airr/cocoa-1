//
//  SCCConfiguration.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCConfiguration.h"

@implementation SCCConfiguration

- (id) init
{
	self = [super init];
	if (self) {
		_children = nil;
	}
	return self;
}

- (id) initWithNibName:(NSString *)name height:(CGFloat)height
{
	self = [super init];
	if (self) {
		_children = nil;
		_nibName = name;
		_height = height;
	}
	return self;
}

+ (id) configuration
{
	return [[self alloc] init];
}

+ (id) configurationWithNibName:(NSString *)name height:(CGFloat)height
{
	return [[self alloc] initWithNibName:name height:height];
}

- (NSString *) description
{
	return [NSString stringWithFormat:@"<%@>", NSStringFromClass([self class])];
}

@end
