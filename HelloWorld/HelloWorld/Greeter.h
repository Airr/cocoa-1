//
//  Greeter.h
//  HelloWorld
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Greeter : NSObject

- (NSString*)greeting;
- (id)initWithName:(NSString*)newName;
@property (copy) NSString *name;
@property (assign, readonly) NSInteger age;
@property (retain) Greeter *buddy;
@property (assign, getter = isUpperCase) BOOL upperCase;

@end
