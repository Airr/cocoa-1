//
//  main.m
//  cos-nsarray
//
//  Created by Sang Chul Choi on 7/15/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

int main(int argc, const char * argv[])
{

    @autoreleasepool {
        // Create three NSDate objects
        NSDate *now = [NSDate date];
        NSDate *tomorrow = [now dateByAddingTimeInterval:24.0 * 60.0 * 60.0];
        NSDate *yesterday = [now dateByAddingTimeInterval:-24.0 * 60.0 * 60.0];
        
        // Create an array containing all three
        NSArray *dateList = @[now, tomorrow, yesterday];
        
        // Print a couple of dates
        NSLog(@"The first date is %@", dateList[0]);
        NSLog(@"The third date is %@", dateList[2]);
        
        // How many dates are in the array?
        NSLog(@"There are %lu dates", [dateList count]);
        
        for (NSDate *d in dateList) {
            NSLog(@"Here is a date: %@", d);
        }
        
        // Read in a file as a huge string (ignoring the possibility of an error)
        NSString *nameString =
        [NSString stringWithContentsOfFile:@"/usr/share/dict/propernames"
                                  encoding:NSUTF8StringEncoding
                                     error:NULL];
        
        // Break it into an array of strings
        NSArray *names = [nameString componentsSeparatedByString:@"\n"];
        
        // Go through the array one string at a time
        for (NSString *n in names) {
            
            // Look for the string "aa" in a case-insensitive manner
            NSRange r = [n rangeOfString:@"AA" options:NSCaseInsensitiveSearch];
            
            // Was it found?
            if (r.location != NSNotFound) {
                NSLog(@"%@", n);
            }
        }
        
    }
    return 0;
}

