//
//  SCCConfigurationGroup.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCConfiguration.h"

@interface SCCConfigurationGroup : SCCConfiguration

@property NSString *title;
+ (id) configurationWithTitle:(NSString *)title;

@end
