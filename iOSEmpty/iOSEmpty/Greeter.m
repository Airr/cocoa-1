//
//  Greeter.m
//  HelloWorld
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "Greeter.h"

@implementation Greeter

@synthesize name, age, today, upperCase;
//- (NSString *) name {
//    return name;
//}
//
//- (void) setName:(NSString *)newName {
//    name = newName;
//}

+ (id)greeterWithName:(NSString *)newName
{
    return [[[Greeter alloc] initWithName:newName] autorelease];
}


- (NSString*)greeting {
    return [NSString stringWithFormat:@"Hello, %@!", self.name];
//    return [[NSString alloc] initWithFormat:@"Hello, %@!", self.name];
}

- (id)init {
    return [self initWithName:@"World"];
}

- (NSString*)description {
    return [NSString stringWithFormat:@"name: %@ \n created: %@",
            self.name, self.today];
}

- (id)initWithName:(NSString*)newName {
    if (self = [super init]) {
        self.name = newName;
        self.upperCase = YES;
        self.today = [NSDate date];
    }
    return self;
}

- (void)dealloc {
    [name release];
    [today release];
    [super dealloc];
}

@end
