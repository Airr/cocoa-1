//
//  XYZPerson.h
//  XYZPerson
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface XYZPerson : NSObject
@property (readonly) NSString* firstName;
@property (readonly) NSString* lastName;
@property (readonly) NSDate* dateOfBirth;
- (void)sayHello;
+ (void)person;
@end
