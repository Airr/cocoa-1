//
//  SCCProjectNavigatorViewController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCProjectNavigatorViewController : NSViewController
<NSOutlineViewDelegate,NSDraggingSource,NSOutlineViewDataSource,
NSControlTextEditingDelegate>

- (instancetype)initWithContent:(NSMutableArray *)contents;

- (void)addXfast:(NSString *)type withName:(NSString *)name withKey:(NSString *)key;
- (NSString *)selectedKey;

@end
