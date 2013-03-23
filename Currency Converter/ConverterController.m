//
//  ConverterController.m
//  Currency Converter
//
//  Created by Sang Chul Choi on 3/21/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "ConverterController.h"

@implementation ConverterController
- (IBAction)convert:(id)sender
{
    float amount;
    converter = [[Converter alloc] init];
    [converter setSourceCurrencyAmount:[dollarField floatValue]];
    [converter setRate:[rateField floatValue]];
    amount = [converter convertCurrency];
    [amountField setFloatValue:amount];
    [rateField selectText:self];
}
@end
