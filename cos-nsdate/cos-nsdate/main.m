//
//  main.m
//  cos-nsdate
//
//  Created by Sang Chul Choi on 7/15/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

int main(int argc, const char * argv[])
{

    @autoreleasepool {
//        NSDate *now = [NSDate date];
        NSDate *now = [[NSDate alloc] init];
        NSLog(@"This NSDate object lives at %p", now);
        NSLog(@"The date is %@", now);
        double seconds = [now timeIntervalSince1970];
        NSLog(@"It has been %f seconds since the start of 1970.", seconds);
        
        NSHost *currentHost = [NSHost currentHost];
        NSString *localizedName = [currentHost localizedName];
        NSLog(@"Your computer's name is %@.", localizedName);
        
        NSDate *later = [now dateByAddingTimeInterval:100000];
        NSLog(@"In 100,000 seconds it will be %@", later);
        
        NSCalendar *cal = [NSCalendar currentCalendar];
        NSLog(@"My calendar is %@", [cal calendarIdentifier]);
        unsigned long day = [cal ordinalityOfUnit:NSDayCalendarUnit
                                           inUnit:NSMonthCalendarUnit
                                          forDate:now];
        NSLog(@"This is day %lu of the month", day);
        
        NSDateComponents *comps = [[NSDateComponents alloc] init];
        [comps setYear:1974];
        [comps setMonth:9];
        [comps setDay:29];
        [comps setHour:0];
        [comps setMinute:0];
        [comps setSecond:0];
        
        NSCalendar *g = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
        NSDate *dateOfBirth = [g dateFromComponents:comps];
        seconds = [now timeIntervalSinceDate:dateOfBirth];
        NSLog(@"It has been %f seconds since I was born.", seconds);
        
        NSDate *currentTime = nil;
        NSLog(@"currentTime's value is %p", currentTime);
    }
    return 0;
}

