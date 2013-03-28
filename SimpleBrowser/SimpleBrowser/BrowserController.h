//
//  BrowserController.h
//  SimpleBrowser
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class WebView;

@interface BrowserController : NSObject

@property (assign) IBOutlet WebView *myWebView;
@property (assign) IBOutlet NSTextField *address;
@property (assign) IBOutlet NSButton *backButton;
@property (assign) IBOutlet NSButton *forwardButton;

- (IBAction)loadPreviousPage:(id)sender;
- (IBAction)loadNextPage:(id)sender;
- (IBAction)loadURLFrom:(id)sender;

@end
