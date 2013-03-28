//
//  BookshelfController.h
//  BookShelf
//
//  Created by Sang Chul Choi on 3/28/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class PragBook;

@interface BookshelfController : NSViewController

@property IBOutlet NSTextField *valueField;
@property (nonatomic) PragBook *book;
-(IBAction) getValue:(id) sender;

@end
