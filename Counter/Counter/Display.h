//
//  Display.h
//  Counter
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
@class UpOrDown;

@interface Display : NSObject
{
    IBOutlet NSTextField *displayField;
    IBOutlet UpOrDown *counter;
}

//- (void)updateDisplay:(NSNumber *)newValue;
@end
