//
//  ActivityController.h
//  HelloApplication
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ActivityMonitorDelegate.h"

@class CurrentApp;

@interface ActivityController : NSObject <ActivityMonitorDelegate>

//@property (retain) CurrentApp *currentApp;
@property (assign) IBOutlet NSTextField *activityDisplay;

@end
