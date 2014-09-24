//
//  SCCContentOutlineViewController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/25/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCContentOutlineViewController : NSViewController
<NSOutlineViewDelegate,NSDraggingSource,NSOutlineViewDataSource>


- (void)populateOutlineContentsWithArray:(NSMutableArray *)content;

- (NSMutableArray *)treeContent;
@end
