//
//  SCPerson.h
//  alder
//
//  Created by Sang Chul Choi on 8/4/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SCPerson;
@protocol SCPersonDelegate
- (void) SCPersonDelegateMethod: (SCPerson *) sender;
@end



@interface SCPerson : NSObject

@property NSString * name;
@property NSString * occupation;
@property BOOL isChecked;
@property (nonatomic, weak) id <SCPersonDelegate> delegate;

@end
