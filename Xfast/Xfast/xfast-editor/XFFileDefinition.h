//
//  XFFileDefinition.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "XFAbstractDefinition.h"

@interface XFFileDefinition : XFAbstractDefinition

@property(nonatomic, strong, readonly) NSString* filePath;
@property(nonatomic, readonly) BOOL copyToDestination;

#pragma mark - Class Methods

+ (XFFileDefinition*)fileDefinitionWithFilePath:(NSString*)filePath
                              copyToDestination:(BOOL)copyToDestination;

#pragma mark - Initialization & Destruction

- (id)initWithFilePath:(NSString*)filePath
     copyToDestination:(BOOL)copyToDestination;


#pragma mark - Interface Methods

- (NSString*)name;

@end
