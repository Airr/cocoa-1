//
//  SCCTargetWindowController.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/14/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SCCViewControllerTarget.h"

@interface SCCTargetWindowController : NSWindowController
{
    SCCViewControllerTarget *viewController;
}

@property NSString *xfastName;

- (IBAction)create:(id)sender;
- (IBAction)cancel:(id)sender;

@end
