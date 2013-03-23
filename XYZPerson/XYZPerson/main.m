//
//  main.m
//  XYZPerson
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "XYZPerson.h"
#import "XYZShoutingPerson.h"

int main(int argc, const char * argv[])
{

    @autoreleasepool {
        
        // insert code here...
        NSLog(@"Hello, World from the main!");
        XYZPerson* person = [[XYZPerson alloc] initWithFirstName:@"Sang Chul" lastName:@"Choi"];
        [person sayHello];
        
        XYZShoutingPerson* anotherPerson = [XYZShoutingPerson person];
        [anotherPerson sayHello];
        
        XYZPerson* somePerson = nil;
        if (!somePerson) {
            NSLog(@"No such somePerson?");
        }
        
        [anotherPerson release];
        [person release];
        
    }
    return 0;
}

