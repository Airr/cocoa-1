//
//  XYZPerson.m
//  XYZPerson
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "XYZPerson.h"

@implementation XYZPerson

- (id)init {
    return [self initWithFirstName:@"John" lastName:@"Doe"];
}

- (id)initWithFirstName:(NSString *)aFirstName lastName:(NSString *)aLastName dateOfBirth:(NSDate *)aDOB
{
    self = [super init];
    if (self)
    {
        _firstName = aFirstName;
        _lastName = aLastName;
        _dateOfBirth = aDOB;
    }
    return self;
}

- (id)initWithFirstName:(NSString *)aFirstName lastName:(NSString *)aLastName
{
    return [self initWithFirstName:aFirstName lastName:aLastName dateOfBirth:nil];
}


- (void)sayHello
{
    [self saySomething:@"Hello, World from XYZPerson!"];
}

- (void)saySomething:(NSString*)greeting
{
    NSLog(@"Thru saySomething: %@ %@ %@", greeting, _firstName, _lastName);
}

+ (id)person
{
    return [[self alloc] init];
}

@end
