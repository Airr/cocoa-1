//
//  Greeter.h
//  HelloWorld
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Greeter : NSObject

+ (id)greeterWithName:(NSString *)newName;

- (NSString*)greeting;
- (id)initWithName:(NSString*)newName;
@property (copy) NSString *name;
@property (assign, readonly) NSInteger age;
@property (copy) NSDate *today;
@property (assign, getter = isUpperCase) BOOL upperCase;

@end
