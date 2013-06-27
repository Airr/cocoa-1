//
//  BigLetterView.h
//  TypingTutor
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface BigLetterView : NSView {
    NSColor *bgColor;
    NSString *string;
}
@property (strong) NSColor *bgColor; @property (copy) NSString *string;
@end
