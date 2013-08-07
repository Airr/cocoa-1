//
//  SCWindowController.h
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCWindowController : NSWindowController
@property NSManagedObjectContext *managedObjectContext;
- (IBAction)runAlignment:(id)sender;
- (IBAction)runAnalysis:(id)sender;
- (IBAction)stopAnalysis:(id)sender;
- (IBAction)addAnalysis:(id)sender;
- (IBAction)removeAnalysis:(id)sender;
- (IBAction)showTheSheet:(id)sender;
- (IBAction)endTheSheet:(id)sender;
@end
