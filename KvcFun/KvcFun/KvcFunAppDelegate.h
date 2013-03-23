//
//  KvcFunAppDelegate.h
//  KvcFun
//
//  Created by Sang Chul Choi on 3/22/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface KvcFunAppDelegate : NSObject <NSApplicationDelegate>
{
    int fido;
}
@property (assign) IBOutlet NSWindow *window;

- (int)fido;
- (void)setFido:(int)x;
@end
