//
//  ViewController.m
//  MobileBrowser
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    self.address.text = @"http://pragprog.com";
    [self loadURLFromTextField];
}

- (void)loadURLFromTextField {
    NSURL *url = [NSURL URLWithString:self.address.text];
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    [self.webView loadRequest:request];
}

- (void)disableWebView {
    self.webView.userInteractionEnabled = NO;
    self.webView.alpha = 0.25;
}

- (void)enableWebView {
    self.webView.userInteractionEnabled = YES;
    self.webView.alpha = 1.0;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    [self disableWebView];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [self loadURLFromTextField];
    [textField resignFirstResponder];
    [self enableWebView];
    return YES;
}

-(void) resetButtons:(UIWebView *) theWebView {
    [self.backButton setEnabled:[theWebView canGoBack]];
    [self.forwardButton setEnabled:[theWebView canGoForward]];
}


//Web View Delegate methods
-(void)webViewDidStartLoad:(UIWebView *) theWebView {
    [self disableWebView];
    [self.activityView startAnimating];
}

-(void)webViewDidFinishLoad:(UIWebView *)theWebView {
    [self enableWebView];
    [self.activityView stopAnimating];
    [self resetButtons:theWebView];
    self.address.text = [[self.webView.request URL] absoluteString];
}

#pragma mark -
#pragma mark Memory Management
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    self.address = nil; self.webView = nil; self.backButton = nil; self.forwardButton = nil;
    self.activityView = nil;
    [super dealloc];
}

@end
