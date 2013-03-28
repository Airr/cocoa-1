//
//  CurrentApp.h
//  HelloApplication
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ActivityMonitorDelegate.h"

@interface CurrentApp : NSObject

//@property id delegate;
@property IBOutlet NSObject <ActivityMonitorDelegate> *delegate;

@end
