//
//  BNRPerson.m
//  cos-bmitime
//
//  Created by Sang Chul Choi on 7/15/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "BNRPerson.h"

@implementation BNRPerson
- (float)heightInMeters
{
    return _heightInMeters;
}

- (void)setHeightInMeters:(float)h
{
    _heightInMeters = h;
}

- (int)weightInKilos
{
    return _weightInKilos;
}

- (void)setWeightInKilos:(int)w
{
    _weightInKilos = w;
}

- (float)bodyMassIndex
{
    return _weightInKilos / (_heightInMeters * _heightInMeters);
}

@end
