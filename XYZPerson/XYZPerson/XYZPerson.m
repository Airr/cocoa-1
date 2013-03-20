//
//  XYZPerson.m
//  XYZPerson
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "XYZPerson.h"

@implementation XYZPerson

- (void)sayHello
{
    [self saySomething:@"Hello, World from XYZPerson!"];
}

- (void)saySomething:(NSString*)greeting
{
    NSLog(@"Thru saySomething: %@", greeting);
}

+ (id)person
{
    return [[self alloc] init];
}

@end
