//
//  SCCProjectDocument.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface SCCProjectDocument : NSDocument

#pragma mark - Files

- (NSArray *)addFileReference:(NSMutableArray *)filePaths;

- (void)addSourceFiles:(NSMutableArray *)newNodes toTarget:(NSString *)targetKey;

- (NSArray *)xfastOutputBuildFilesWithTargetKey:(NSString *)aTargetKey;

- (NSString *)addTarget:(NSString *)aName;

- (void)showXfastProjectSettings:(NSString *)key;

- (void)pushContentXfast;

- (void)saveText:(NSString *)text withPath:(NSString *)path;

- (void)pushProject;

- (void)showProjectNavigatorView;

- (NSString *)operationOfXfast;

- (NSArray *)outFilesOfTargetWithKey:(NSString *)aKey;

@end
