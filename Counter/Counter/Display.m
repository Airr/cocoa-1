//
//  Display.m
//  Counter
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "Display.h"
#import "UpOrDown.h"
@implementation Display

- (void)updateDisplay:(NSNumber *)newValue
{
    [displayField setIntegerValue:[newValue integerValue]];
}

- (void)awakeFromNib
{
    [counter addObserver:self forKeyPath:@"count" options:NSKeyValueObservingOptionNew context:NULL];
}

- (void)finalize
{
    [counter removeObserver:self forKeyPath:@"count"];
    [super finalize];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self updateDisplay:[object valueForKeyPath:keyPath]];
}
@end
