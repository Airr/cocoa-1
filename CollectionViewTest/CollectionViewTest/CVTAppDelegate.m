//
//  CVTAppDelegate.m
//  CollectionViewTest
//
//  Created by Sang Chul Choi on 3/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "CVTAppDelegate.h"
#import "PersonModel.h"

@implementation CVTAppDelegate

@synthesize personModelArray;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
}

-(void)insertObject:(PersonModel *)p inPersonModelArrayAtIndex:(NSUInteger)index {
    [personModelArray insertObject:p atIndex:index];
}

-(void)removeObjectFromPersonModelArrayAtIndex:(NSUInteger)index {
    [personModelArray removeObjectAtIndex:index];
}

-(void)setPersonModelArray:(NSMutableArray *)a {
    personModelArray = a;
}

-(NSArray*)personModelArray {
    return personModelArray;
}

- (void)awakeFromNib {
    
    PersonModel * pm1 = [[PersonModel alloc] init];
    pm1.name = @"John Appleseed";
    pm1.occupation = @"Doctor";
    
    PersonModel * pm2 = [[PersonModel alloc] init];
    pm2.name = @"Jane Carson";
    pm2.occupation = @"Teacher";
    
    PersonModel * pm3 = [[PersonModel alloc] init];
    pm3.name = @"Ben Alexander";
    pm3.occupation = @"Student";
    
    NSMutableArray * tempArray = [NSMutableArray arrayWithObjects:pm1, pm2, pm3, nil];
    [self setPersonModelArray:tempArray];
    
}

@end
