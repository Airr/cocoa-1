//
//  PersonModel.h
//  CollectionViewTest
//
//  Created by Sang Chul Choi on 3/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface PersonModel : NSObject
{
    NSString * name;
    NSString * occupation;
}
@property(retain, readwrite) NSString * name;
@property(retain, readwrite) NSString * occupation;

@end
