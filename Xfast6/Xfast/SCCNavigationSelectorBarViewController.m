//
//  SCCNavigationSelectorBarViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCNavigationSelectorBarViewController.h"

@interface SCCNavigationSelectorBarViewController ()

@property (weak) IBOutlet NSButton *project;
@property (weak) IBOutlet NSButton *find;
@property (weak) IBOutlet NSButton *report;
@property (weak) IBOutlet NSButton *log;

@end

@implementation SCCNavigationSelectorBarViewController

//- (void)viewDidLoad {
//    [super viewDidLoad];
//    // Do view setup here.
//    // NOT CALLED! WHY? Because viewDidLoad is for 10.10 or later.
//    // Mine computer is 10.9.
//    [[self project] setEnabled:NO];
//}

- (void)loadView {
    [super loadView];
    [_project setEnabled:NO];
}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCNavigationSelectorBarViewController" bundle:nil];
    if (self) {
    }
    return self;
}

- (IBAction)showHideProject:(id)sender {
    if ([[self project] state] == NSOnState) {
        [self willChangeValueForKey:@"project"];
        [[self project] setState:NSOnState];
        [[self find] setState:NSOffState];
        [[self report] setState:NSOffState];
        [[self log] setState:NSOffState];
        
        [[self find] setEnabled:YES];
        [[self report] setEnabled:YES];
        [[self log] setEnabled:YES];
        [self didChangeValueForKey:@"project"];
    } else {
        [[self project] setState:NSOnState];
        
    }
    [[self project] setEnabled:NO];
}

- (IBAction)showHideFind:(id)sender {
    if ([[self find] state] == NSOnState) {
        [self willChangeValueForKey:@"find"];
        [[self project] setState:NSOffState];
        [[self find] setState:NSOnState];
        [[self report] setState:NSOffState];
        [[self log] setState:NSOffState];
        
        [[self project] setEnabled:YES];
        [[self report] setEnabled:YES];
        [[self log] setEnabled:YES];
        [self didChangeValueForKey:@"find"];
    } else {
        [[self find] setState:NSOnState];
    }
    [[self find] setEnabled:NO];
}

- (IBAction)showHideReport:(id)sender {
    if ([[self report] state] == NSOnState) {
        [self willChangeValueForKey:@"report"];
        [[self project] setState:NSOffState];
        [[self find] setState:NSOffState];
        [[self report] setState:NSOnState];
        [[self log] setState:NSOffState];
        
        [[self project] setEnabled:YES];
        [[self find] setEnabled:YES];
        [[self log] setEnabled:YES];
        [self didChangeValueForKey:@"report"];
    } else {
        [[self report] setState:NSOnState];
        
    }
    [[self report] setEnabled:NO];
}
- (IBAction)showHideLog:(id)sender {
    if ([[self log] state] == NSOnState) {
        [self willChangeValueForKey:@"log"];
        [[self project] setState:NSOffState];
        [[self find] setState:NSOffState];
        [[self report] setState:NSOffState];
        [[self log] setState:NSOnState];
        
        [[self project] setEnabled:YES];
        [[self find] setEnabled:YES];
        [[self report] setEnabled:YES];
        [self didChangeValueForKey:@"log"];
    } else {
        [[self log] setState:NSOnState];
        
    }
    [[self log] setEnabled:NO];
}

@end
