//
//  BNRDocument.h
//  cos-tahdoodle
//
//  Created by Sang Chul Choi on 7/17/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BNRDocument : NSDocument <NSTableViewDataSource>
@property (nonatomic) NSMutableArray *tasks;

@property (nonatomic) IBOutlet NSTableView *taskTable;

- (IBAction)addTask:(id)sender;
@end
