//
//  ConverterController.h
//  Currency Converter
//
//  Created by Sang Chul Choi on 3/21/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Converter.h"

@interface ConverterController : NSObject
{
    IBOutlet NSTextField* amountField;
    IBOutlet NSTextField* dollarField;
    IBOutlet NSTextField* rateField;
    Converter *converter;
}
- (IBAction)convert:(id)sender;
@end
