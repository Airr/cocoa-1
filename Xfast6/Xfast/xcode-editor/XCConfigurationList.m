//
//  XCConfigurationList.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/20/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "XCConfigurationList.h"
#import "XCProject.h"

@implementation XCConfigurationList

- (instancetype)initWithProject:(XCProject*)project
                            key:(NSString*)key
{
    self = [super init];
    if (self)
    {
        _project = project;
        _key = [key copy];
        
        _buildConfigurations = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)addBuildConfigurationKey:(NSString *)aKey
{
    [[self buildConfigurations] addObject:aKey];
}

/**
 *  Returns the build configuration as a dictionary.
 *
 *  @return Dictionary of the build configuration.
 */
- (NSMutableDictionary*)asDictionary
{
    NSMutableDictionary* data = [NSMutableDictionary dictionary];
    [data setObject:[NSString stringFromMemberType:XCConfigurationListType]
             forKey:@"isa"];
    [data setObject:[self buildConfigurations]
             forKey:@"buildConfigurations"];
    
    return data;
}


@end
