//
//  SCCContentSelectorBarViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCContentSelectorBarViewController.h"

@interface SCCContentSelectorBarViewController ()
@property (weak) IBOutlet NSButton *general;
@property (weak) IBOutlet NSButton *settings;

@end

@implementation SCCContentSelectorBarViewController

//- (void)viewDidLoad {
//    [super viewDidLoad];
//    // Do view setup here.
//}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCContentSelectorBarViewController" bundle:nil];
    if (self) {
    }
    return self;
}

- (void)setButtonTitleFor:(NSButton*)button toString:(NSString*)title withColor:(NSColor*)color
{
    NSMutableAttributedString *buttonTitle = [[button attributedTitle] mutableCopy];
    NSRange selectedRange = NSMakeRange(0, [buttonTitle length]);
    
    if (title) {
        [buttonTitle replaceCharactersInRange:selectedRange withString:title];
        
    }
    [buttonTitle beginEditing];
    [buttonTitle addAttribute:NSForegroundColorAttributeName
                        value:color
                        range:selectedRange];
    [buttonTitle endEditing];
    [button setAttributedTitle:buttonTitle];
}

- (void)setButtonColorFor:(NSButton*)button withColor:(NSColor*)color
{
    [self setButtonTitleFor:button toString:nil withColor:color];
}

- (IBAction)showOutline:(id)sender {
    
}


- (IBAction)showGeneral:(id)sender {
    [self setButtonColorFor:[self general] withColor:[NSColor blueColor]];
    [self setButtonColorFor:[self settings] withColor:[NSColor blackColor]];
}

- (IBAction)showSettings:(id)sender {
    [self setButtonColorFor:[self general] withColor:[NSColor blackColor]];
    [self setButtonColorFor:[self settings] withColor:[NSColor blueColor]];
}

@end
