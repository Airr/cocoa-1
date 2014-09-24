//
//  SCCContentTextViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/30/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCContentTextViewController.h"
#import "SCCProjectDocument.h"

@interface SCCContentTextViewController ()
@property (unsafe_unretained) IBOutlet NSTextView *textView;
@property (weak) IBOutlet NSButton *buttonSave;
@property (nonatomic,weak) SCCProjectDocument *document;
@property NSString *path;
@end

@implementation SCCContentTextViewController

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCContentTextViewController" bundle:nil];
    if (self) {
    }
    return self;
}

- (SCCProjectDocument *)document
{
    if (_document == nil) {
        _document = [[[[self view] window] windowController] document];
    }
    return _document;
}

- (void)awakeFromNib
{
}

- (IBAction)save:(id)sender {
    [[self document] saveText:[[self textView] string] withPath:[self path]];
    
}

- (void)changeText:(NSString *)text withPath:(NSString *)path
{
    [self setPath:path];
    [[self textView] setString:text];
    [[[self textView] textStorage] setFont:[NSFont fontWithName:@"Monaco" size:11]];
}

@end
