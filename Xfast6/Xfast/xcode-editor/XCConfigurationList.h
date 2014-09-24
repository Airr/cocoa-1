//
//  XCConfigurationList.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/20/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class XCProject;

@interface XCConfigurationList : NSObject

@property (weak,readonly) XCProject *project;
@property NSMutableArray *buildConfigurations;
@property (copy,readonly) NSString *key;

- (instancetype)initWithProject:(XCProject*)project
                            key:(NSString*)key;


- (void)addBuildConfigurationKey:(NSString *)aKey;

- (NSDictionary*)asDictionary;

@end
