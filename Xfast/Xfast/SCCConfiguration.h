//
//  SCCConfiguration.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SCCConfiguration : NSObject

@property NSArray *children;
@property NSString *nibName;
@property CGFloat height;

+ (id) configuration;
+ (id) configurationWithNibName:(NSString *)name height:(CGFloat)height;

@end
