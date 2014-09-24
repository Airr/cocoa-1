//
//  SCCProjectWindowController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCProjectWindowController : NSWindowController

- (id)initWithData:(NSMutableDictionary *)data;
- (void)addXfast:(NSString *)type withName:(NSString *)name withKey:(NSString *)key;
- (void)showContentOfXfast:(NSMutableArray *)content;

- (void)showContentOfNothing;

- (void)showContentOfProduct;

- (void)showContentOfProject;

- (NSString *)selectedNodeKeyOfProjectNavigator;


- (NSMutableArray *)treeContentOfXfastOutline;

- (void)showXfastProjectSettings:(NSMutableDictionary *)settings;

- (void)showXfastText:(NSString *)text;


- (void)showXfastText:(NSString *)text withPath:(NSString *)path;

- (void)addEntryWithProject:(NSString *)project
                  withXfast:(NSString *)xfast
                 withTarget:(NSString *)target
                  withStart:(NSString *)start
                    withEnd:(NSString *)end
                 withStatus:(NSString *)status;

@end
