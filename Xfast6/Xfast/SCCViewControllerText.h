//
//  SCCViewControllerText.h
//  Xfast
//
//  Created by Sang Chul Choi on 7/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCCViewControllerText : NSViewController
{
    IBOutlet NSTextView *_textView;
}

@property(readwrite,retain) NSTextStorage *textStorage;
@property(readonly) NSTextView *textView;
@property(readonly) NSView *containerView;

- (id)initWithFilePath:(NSString *)filePath;
- (id)initWithTextStorage:(NSTextStorage *)textStorage;
- (NSLayoutManager *)layoutManagerForTextStorage:(NSTextStorage *)givenStorage;
- (NSTextContainer *)textContainerForLayoutManager:(NSLayoutManager *)layoutManager;
- (NSTextView *)textViewForTextContainer:(NSTextContainer *)textContainer;
- (void)setTextWithFilePath:(NSString *)filePath;
@end
