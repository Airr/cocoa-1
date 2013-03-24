//
//  Person.h
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Person : NSObject {
    NSString *personName;
    float expectedRaise;
}
@property (readwrite, copy) NSString *personName;
@property (readwrite) float expectedRaise;
@end
