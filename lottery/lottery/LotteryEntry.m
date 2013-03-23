//
//  LotteryEntry.m
//  lottery
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "LotteryEntry.h"

@implementation LotteryEntry
- (void)dealloc
{
    NSLog(@"deallocating %@", self);
    // entryDate release might cause problems if setEntryDate was never called.
    [entryDate release];
    [super dealloc];
}
- (void)prepareRandomNumbers
{
    firstNumber = ((int)random() % 100) + 1;
    secondNumber = ((int)random() % 100) + 1;
}
- (void)setEntryDate:(NSDate *)date
{
    [date retain];
    [entryDate release];
    entryDate = date;
}
- (NSDate *)entryDate
{
    return entryDate;
}
- (int)firstNumber
{
    return firstNumber;
}
- (int)secondNumber
{
    return secondNumber;
}
- (NSString *)description
{
    NSDateFormatter *df = [[NSDateFormatter alloc] init];
    [df setTimeStyle:NSDateFormatterNoStyle];
    [df setDateStyle:NSDateFormatterMediumStyle];
    NSString *result;
    result = [NSString stringWithFormat:@"%@ = %d and %d",
              [df stringFromDate:entryDate],
              firstNumber, secondNumber];
    // What are autoreleased objects?
//    result = [[NSString alloc] initWithFormat:@"%@ = %d and %d",
//              [df stringFromDate:entryDate],
//              firstNumber, secondNumber];
//    [result autorelease];
    [df release];
    return result;
}
@end
