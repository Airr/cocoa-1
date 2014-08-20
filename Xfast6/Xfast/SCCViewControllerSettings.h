//
//  SCCViewControllerSettings.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class XFProject;



@interface SCCViewControllerSettings : NSViewController <NSOutlineViewDelegate,NSOutlineViewDataSource,NSTableViewDelegate,NSTableViewDataSource>

- (id)initWithXFProject:(XFProject *)xfastProject;

@end
