//
//  BNRPerson.h
//  cos-bmitime
//
//  Created by Sang Chul Choi on 7/15/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BNRPerson : NSObject
{
    // BNRPerson has two instance variables
    float _heightInMeters;
    int _weightInKilos;
}
// BNRPerson has methods to read and set its instance variables
- (float)heightInMeters;
- (void)setHeightInMeters:(float)h;
- (int)weightInKilos;
- (void)setWeightInKilos:(int)w;

// BNRPerson has a method that calculates the Body Mass Index
- (float)bodyMassIndex;
@end
