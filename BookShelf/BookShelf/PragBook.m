//
//  PragBook.m
//  BookShelf
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "PragBook.h"

@implementation PragBook

@synthesize title, author;

-(id) valueForUndefinedKey:(NSString *)key {
    return [NSString stringWithFormat:@"No property with key %@.", key];
}

@end
