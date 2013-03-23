//
//  Converter.h
//  Currency Converter
//
//  Created by Sang Chul Choi on 3/20/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Converter : NSObject
{
    float sourceCurrencyAmount, rate;
}
@property(readwrite) float sourceCurrencyAmount, rate;
- (float)convertCurrency;
@end
