//
//  CVTAppDelegate.h
//  CollectionViewTest
//
//  Created by Sang Chul Choi on 3/25/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface CVTAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (retain, readwrite) NSMutableArray* personModelArray;

@end
