//
//  RMDocument.h
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class Person;

@interface RMDocument : NSDocument {
    NSMutableArray *employees;
    IBOutlet NSTableView *tableView;
    IBOutlet NSArrayController *employeeController;
}
- (IBAction)createEmployee:(id)sender;
- (void)setEmployees:(NSMutableArray *)a;
- (void)insertObject:(Person *)p inEmployeesAtIndex:(NSUInteger)index;
- (void)removeObjectFromEmployeesAtIndex:(NSUInteger)index;
- (IBAction)removeEmployee:(id)sender;
@end