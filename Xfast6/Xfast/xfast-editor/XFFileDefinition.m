//
//  XFFileDefinition.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "XFFileDefinition.h"

@implementation XFFileDefinition

#pragma mark - Class Methods

+ (XFFileDefinition*)fileDefinitionWithFilePath:(NSString*)filePath
                              copyToDestination:(BOOL)copyToDestination
{

    return [[XFFileDefinition alloc] initWithFilePath:filePath copyToDestination:copyToDestination];
}

#pragma mark - Initialization & Destruction

- (id)initWithFilePath:(NSString*)filePath copyToDestination:(BOOL)copyToDestination
{
    self = [super init];
    if (self)
    {
        _filePath = [filePath copy];
        _copyToDestination = copyToDestination;
    }
    return self;
}

#pragma mark - Interface Methods

- (NSString*)name
{
    return [_filePath lastPathComponent];
}

@end
