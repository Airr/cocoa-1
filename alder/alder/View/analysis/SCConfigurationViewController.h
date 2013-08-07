//
//  SCConfigurationViewController.h
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SCConfigurationViewController : NSViewController
@property NSManagedObjectContext *managedObjectContext;
- (IBAction)updateManagedObjectContext:(id)sender;
- (IBAction)add:(id)sender;
@end
