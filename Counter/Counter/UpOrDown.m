//
//  UpOrDown.m
//  Counter
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "UpOrDown.h"
#import "Display.h"
@implementation UpOrDown

- (IBAction)step:(id)sender
{
//    [display updateDisplay:[NSNumber numberWithInteger:[sender integerValue]]];
    [self setValue:[NSNumber numberWithInteger:[sender integerValue]] forKey:@"count"];
}
@end
