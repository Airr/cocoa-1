//
//  BNRAppDelegate.h
//  cos-itahdoodle
//
//  Created by Sang Chul Choi on 7/17/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <UIKit/UIKit.h>
// Declare a helper function that you will use to get a path
// to the location on disk where you can save the to-do list
NSString *BNRDocPath(void);


@interface BNRAppDelegate : UIResponder <UIApplicationDelegate, UITableViewDataSource>

@property (strong, nonatomic) UIWindow *window;

@property (nonatomic) UITableView *taskTable;
@property (nonatomic) UITextField *taskField;
@property (nonatomic) UIButton *insertButton;

@property (nonatomic) NSMutableArray *tasks;

- (void)addTask:(id)sender;

@end
