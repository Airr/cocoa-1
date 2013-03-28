//
//  AppDelegate.m
//  HelloWorld
//
//  Created by Sang Chul Choi on 3/27/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "AppDelegate.h"
#import "Greeter.h"
@implementation AppDelegate

//- (NSTextField*)labelWithText:(NSString*)labelText
//{
//    NSRect labelFrame = NSMakeRect(20, 20, 540, 50);
//    NSTextField* label = [[NSTextField alloc] initWithFrame:labelFrame];
//    [label setEditable:NO];
//    [label setSelectable:NO];
//    [label setAlignment:NSCenterTextAlignment];
//    [label setFont:[NSFont boldSystemFontOfSize:36]];
//    [label setStringValue:labelText];
//    return label;
//}
//
//- (Greeter*)greeter {
//    return [[Greeter alloc] init];
//}
//
//- (Greeter*)greeterFor:(NSString*)personName {
//    return [[Greeter alloc] initWithName:personName];
//}
//
//- (void)setUpperCaseName:(Greeter *)greeter {
//    NSLog(@"The name was %@.", [greeter name]);
//    greeter.name = [greeter.name uppercaseString];
//    NSLog(@"The name is %@.", [greeter name]);
//}
//
//- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
//{
//    Greeter* greeter = [self greeterFor:@"Maggie"];
//    NSLog(@"Greeter: %@", greeter);
//    NSLog(@"This occurred in %@ at line %d in file %s.",
//          NSStringFromSelector(_cmd), __LINE__, __FILE__);
//    [self setUpperCaseName:greeter];
//    NSTextField* label = [self labelWithText:[greeter greeting]];
//    [[self.window contentView] addSubview:label];
//}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    Greeter *host = [[Greeter alloc] initWithName:@"Maggie"];
    NSLog(@"Greeter %@", host);
}
@end
