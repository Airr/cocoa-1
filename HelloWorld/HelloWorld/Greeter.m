//
//  Greeter.m
//  HelloWorld
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "Greeter.h"

@implementation Greeter

@synthesize name, age, buddy, upperCase;
//- (NSString *) name {
//    return name;
//}
//
//- (void) setName:(NSString *)newName {
//    name = newName;
//}

- (NSString*)greeting {
    return [[NSString alloc] initWithFormat:@"Hello, %@!", self.name];
}

- (id)init {
    return [self initWithName:@"World"];
}

- (NSString*)description {
    return [[NSString alloc] initWithFormat:@"name: %@ \n created: %@",
            name, [NSDate date]];
}

- (id)initWithName:(NSString*)newName {
    if (self = [super init]) {
        self.name = newName;
        self.upperCase = YES;
    }
    return self;
}

@end
