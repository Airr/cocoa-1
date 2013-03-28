//
//  Logger.m
//  Counter
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "Logger.h"
#import "UpOrDown.h"

@implementation Logger
- (void)logValue:(NSNumber *)value
{
    NSLog(@"%@", value);
}

- (void)awakeFromNib
{
    [counter addObserver:self forKeyPath:@"count" options:NSKeyValueObservingOptionNew context:NULL];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self logValue:[object valueForKeyPath:keyPath]];
}

- (void)finalize
{
    [counter removeObserver:self forKeyPath:@"count"];
    [super finalize];
}
@end
