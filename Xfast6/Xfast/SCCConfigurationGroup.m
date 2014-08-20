//
//  SCCConfigurationGroup.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCConfigurationGroup.h"

@implementation SCCConfigurationGroup

- (id) initWithTitle:(NSString *)title
{
	self = [super init];
	if (self) {
		_title = title;
		self.children = @[[SCCConfiguration configuration], [SCCConfiguration configuration]];
	}
	return self;
}

+ (id) configurationWithTitle:(NSString *)title
{
	return [[self alloc] initWithTitle:title];
}

- (NSString *) description
{
	return [NSString stringWithFormat:@"<%@: %@>", NSStringFromClass([self class]), self.title];
}

@end
