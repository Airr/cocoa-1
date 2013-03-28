//
//  CurrentApp.m
//  HelloApplication
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "CurrentApp.h"

@implementation CurrentApp

- (void)reportActivity: (NSNotification *) notification {
    NSLog(@"%@", notification.name );
}

- (id)init {
    if (self = [super init]) {
        [self registerNotifications];
    }
    return self;
}

-(void) applicationDidLaunch: (NSNotification *) notification {
    //[[NSNotificationCenter defaultCenter] postNotificationName:@"Launched" object:self];
    if ([self.delegate respondsToSelector:@selector(applicationDidLaunch:)] ) {
        NSLog(@"Delegate implements applicationDidLaunch:.");
    } else {
        NSLog(@"We're on our own.");
    }
//    if ([self.delegate respondsToSelector:@selector(applicationDidLaunch:)] ) {
        [self.delegate applicationDidLaunch:self];
//    }
}



-(void) applicationDidTerminate: (NSNotification *) notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:@"Terminated" object:self];
//    if ([self.delegate respondsToSelector:@selector(applicationDidTerminate:)]) {
        [self.delegate applicationDidTerminate:self];
//    }
}

-(void)setUpNotification:(NSString *)notification withSelector:(SEL)methodName {
    [[[NSWorkspace sharedWorkspace] notificationCenter]
     addObserver:self
     selector:methodName
     name:notification
     object:nil];
}

-(void) registerNotifications {
    [self setUpNotification:NSWorkspaceDidLaunchApplicationNotification
               withSelector:@selector(applicationDidLaunch:)];
    [self setUpNotification:NSWorkspaceDidTerminateApplicationNotification
               withSelector:@selector(applicationDidTerminate:)];
}

@end
