//
//  BookShelfViewController.m
//  BookShelf
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "BookShelfViewController.h"
#import "PragBook.h"

@interface BookShelfViewController ()

@end

@implementation BookShelfViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (IBAction)getValue:(id) sender {
    //START:code.kvc.getvalue
    //[self.valueField setStringValue:[self.book valueForKey:[sender stringValue]]];
    //END:code.kvc.getvalue
    NSLog(@"%@", [sender stringValue]);
}

- (PragBook *)book {
    if (!_book) {
        self.book = [[PragBook alloc] init];
        self.book.title = @"Manage Your Project Portfolio";
        self.book.author = @"Johanna Rothman";
    }
    return _book;
}

@end
