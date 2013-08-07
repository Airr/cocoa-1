//
//  SCPerson.m
//  alder
//
//  Created by Sang Chul Choi on 8/4/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCPerson.h"

@implementation SCPerson

@synthesize name;
@synthesize occupation;
@synthesize isChecked;
@synthesize delegate; //synthesise  MyClassDelegate delegate

- (void) myMethodToDoStuff {
    [self.delegate SCPersonDelegateMethod:self];
}

@end
