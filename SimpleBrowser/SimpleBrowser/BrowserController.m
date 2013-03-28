//
//  BrowserController.m
//  SimpleBrowser
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "BrowserController.h"
#import <WebKit/WebKit.h>

@implementation BrowserController

- (void) resetButtons {
    [self.backButton setEnabled:[self.myWebView canGoBack]];
    [self.forwardButton setEnabled:[self.myWebView canGoForward]];
}

- (IBAction)loadPreviousPage:(id)sender
{
    [self.myWebView goBack:sender];
    [self resetButtons];
}

- (IBAction)loadNextPage:(id)sender
{
    [self.myWebView goForward:sender];
    [self resetButtons];
}

- (IBAction)loadURLFrom:(id)sender
{
    [self.myWebView takeStringURLFrom:sender];
}

- (void)awakeFromNib {
    [self.address setStringValue:@"http://lemming.iab.alaska.edu"];
    [self loadURLFrom:self.address];
    [self.backButton setEnabled:NO];
    [self.forwardButton setEnabled:NO];
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    [[sender window] setTitle:title];
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    NSLog(@"Error?");
}
@end
