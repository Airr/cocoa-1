//
//  main.m
//  lottery
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LotteryEntry.h"

int main(int argc, const char * argv[])
{

    @autoreleasepool {
        // Create the date object
        NSDate *now = [[NSDate alloc] init];
        NSCalendar *cal = [NSCalendar currentCalendar];
        NSDateComponents *weekComponents = [[NSDateComponents alloc] init];
        
        // Seed the random number generator
        srandom((unsigned)time(NULL));
        NSMutableArray *array;
        array = [[NSMutableArray alloc] init];
        int i;
        for (i = 0; i < 10; i++) {
            [weekComponents setWeek:i];
            // Create a date/time object that is ‘i’ weeks from now
            NSDate *iWeeksFromNow;
            iWeeksFromNow = [cal dateByAddingComponents:weekComponents
                                                 toDate:now
                                                options:0];
            // Create a new instance of LotteryEntry
            LotteryEntry *newEntry = [[LotteryEntry alloc] init];
            [newEntry prepareRandomNumbers];
            [newEntry setEntryDate:iWeeksFromNow];
            
            // Add the LotteryEntry object to the array
            [array addObject:newEntry];
            [newEntry release];
        }
        // Done with 'now' and 'weekComponents'
        [now release];
        [weekComponents release];
        // How about 'cal' and 'iWeeksFromNow'?
        
        for (LotteryEntry *entryToPrint in array) {
            // Display its contents
            NSLog(@"%@", entryToPrint);
        }
        // Done with 'array'
        [array release];
    }
    return 0;
}

