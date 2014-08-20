//
//  SCCViewControllerText.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCViewControllerText.h"

@interface SCCViewControllerText ()

@end

const CGFloat LargeNumberForText = 1.0e7; // Any larger dimensions and the text could become blurry.

@implementation SCCViewControllerText

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (id)initWithTextStorage:(NSTextStorage *)givenStorage {
//    self = [super init];
    self = [super initWithNibName:@"SCCViewControllerText" bundle:nil];
    if (self) {
        _textStorage = givenStorage;
        NSLayoutManager *layoutManager = [self layoutManagerForTextStorage:_textStorage];
        NSTextContainer *textContainer = [self textContainerForLayoutManager:layoutManager];
        _textView = [self textViewForTextContainer:textContainer]; // not retained, the text storage is owner of the whole system
    }
    return self;
}

- (id)initWithFilePath:(NSString *)filePath
{
    self = [super initWithNibName:@"SCCViewControllerText" bundle:nil];
    if (self) {
//        NSError *error;
//        NSString *newContents = [[NSString alloc] initWithContentsOfFile:filePath usedEncoding:NULL error:&error];
//        if (newContents) {
//            _textStorage = [[NSTextStorage alloc] initWithPath:filePath documentAttributes:NULL];
//        } else {
//            NSLog(@"newContents is empty.");
//        }
//        
//        NSLayoutManager *layoutManager = [self layoutManagerForTextStorage:_textStorage];
//        NSTextContainer *textContainer = [self textContainerForLayoutManager:layoutManager];
//        [_textView setTextContainer:textContainer];
    }
    return self;
}

/* No special action is taken, but subclasses can override this to configure the layout manager more specifically.
 */
- (NSLayoutManager *)layoutManagerForTextStorage:(NSTextStorage *)givenStorage {
    NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
    [givenStorage addLayoutManager:layoutManager];
    return layoutManager;
}

/* The text container is created with very large dimensions so as not to impede the natural flow of the text by forcing it to wrap. The value of LargeNumberForText was not chosen arbitrarily; any larger and the text would begin to look blurry. It's a limitation of floating point numbers and goes all the way down to Postscript. No other special action is taken in setting up the text container, but subclasses can override this to configure it more specifically.
 */
- (NSTextContainer *)textContainerForLayoutManager:(NSLayoutManager *)layoutManager {
    NSTextContainer *textContainer = [[NSTextContainer alloc] initWithContainerSize:NSMakeSize(LargeNumberForText, LargeNumberForText)];
    [layoutManager addTextContainer:textContainer];
    return textContainer;
}

/* Sets up a text view with reasonable initial settings. Subclasses can override this to configure it more specifically.
 */
- (NSTextView *)textViewForTextContainer:(NSTextContainer *)textContainer {
    [_textView setTextContainer:textContainer];
    NSTextView *view = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100) textContainer:textContainer];
    [view setMaxSize:NSMakeSize(LargeNumberForText, LargeNumberForText)];
    [view setSelectable:YES];
    [view setEditable:YES];
    [view setRichText:YES];
    [view setImportsGraphics:YES];
    [view setUsesFontPanel:YES];
    [view setUsesRuler:YES];
    [view setAllowsUndo:YES];
    return view;
}

- (void)setTextWithFilePath:(NSString *)filePath
{
    NSError *error;
    NSString *newContents = [[NSString alloc] initWithContentsOfFile:filePath usedEncoding:NULL error:&error];
    if (newContents) {
        _textStorage = [[NSTextStorage alloc] initWithPath:filePath documentAttributes:NULL];
    } else {
        NSLog(@"newContents is empty.");
    }
    
    NSLayoutManager *layoutManager = [self layoutManagerForTextStorage:_textStorage];
    NSTextContainer *textContainer = [self textContainerForLayoutManager:layoutManager];
    [_textView setTextContainer:textContainer];
}

@end
