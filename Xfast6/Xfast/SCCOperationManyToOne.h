//
//  SCCOperationManyToOne.h
//  Xfast
//
//  Created by Sang Chul Choi on 9/4/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SCCCommand.h"

@interface SCCOperationManyToOne : NSOperation

- (id)initWithCommand:(id<SCCCommand>)command;

@end
