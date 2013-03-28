//
//  ActivityMonitorDelegate.h
//  HelloApplication
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

// #import <Foundation/Foundation.h>

@class CurrentApp;

@protocol ActivityMonitorDelegate // <NSObject>
//@optional
- (void)applicationDidLaunch: (CurrentApp *) app;
- (void)applicationDidTerminate: (CurrentApp *) app;
@end
