//
//  Converter.m
//  Currency Converter
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "Converter.h"

@implementation Converter
@synthesize sourceCurrencyAmount, rate;
- (float)convertCurrency {
    return self.sourceCurrencyAmount * self.rate;
}
@end
