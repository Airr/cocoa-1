//
//  XYZPerson.h
//  XYZPerson
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface XYZPerson : NSObject
@property (copy) NSString* firstName;
@property (copy) NSString* lastName;
@property (readonly) NSString* fullName;
@property (readonly) NSDate* dateOfBirth;
- (id)init;
- (id)initWithFirstName:(NSString *)aFirstName lastName:(NSString *)aLastName dateOfBirth:(NSDate *)aDOB;
- (id)initWithFirstName:(NSString *)aFirstName lastName:(NSString *)aLastName;
- (void)sayHello;
- (void)saySomething:(NSString*)greeting;
+ (id)person;
@end
