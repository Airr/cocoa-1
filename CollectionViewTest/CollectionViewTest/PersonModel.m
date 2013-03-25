//
//  PersonModel.m
//  CollectionViewTest
//
//  Created by Sang Chul Choi on 3/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "PersonModel.h"

@implementation PersonModel
@synthesize name;
@synthesize occupation;

-(void) dealloc {
    [name release];
    [occupation release];
    [super dealloc];
}
@end
