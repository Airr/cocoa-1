//
//  SCCLogJobViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/31/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCLogJobViewController.h"

@interface SCCLogJobViewController ()

@property (strong) IBOutlet NSDateFormatter *sharedDateFormatter;
@property (weak) IBOutlet NSTableView *tableView;
@property (strong) IBOutlet NSArrayController *arrayController;
@property NSMutableArray *array;

@end

@implementation SCCLogJobViewController

- (instancetype)initWithNibName:(NSString *)nibNameOrNil
                         bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCLogJobViewController" bundle:nil];
    if (self) {
        _array = [NSMutableArray array];
    }
    return self;
}

- (void)awakeFromNib
{
}

- (void)addData
{
//    NSBundle *bundle = [NSBundle mainBundle];
//    NSString *path = [bundle pathForResource: @"people" ofType: @"dict"];
//    NSArray *listFromFile = [NSArray arrayWithContentsOfFile: path];
//    
//    if (listFromFile != nil)
//    {
//        [_array addObjectsFromArray:listFromFile];
//        [_arrayController setContent:_array];
//    }
}

- (IBAction)remove:(id)sender {
    NSInteger row = [_tableView rowForView:sender];
    if (row != -1) {
        [_array removeObjectAtIndex:row];
        [_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:row] withAnimation:NSTableViewAnimationEffectFade];
        [self _selectRowStartingAtRow:row];
    }

}

- (void)_selectRowStartingAtRow:(NSInteger)row {
    if ([_tableView selectedRow] == -1) {
        if (row == -1) {
            row = 0;
        }
    }
}

- (void)addEntryWithProject:(NSString *)project
                  withXfast:(NSString *)xfast
                 withTarget:(NSString *)target
                  withStart:(NSString *)start
                    withEnd:(NSString *)end
                 withStatus:(NSString *)status
{
    NSDictionary *entry = [NSDictionary dictionaryWithObjectsAndKeys:
                           project, @"lastName",
                           xfast, @"firstName",
                           target, @"street",
                           start, @"city",
                           end, @"state",
                           status, @"zip",
                           nil];
    [_arrayController addObject:entry];
}

- (void)removeEntryWithProject:(NSString *)project
                     withXfast:(NSString *)xfast
                    withTarget:(NSString *)target
                     withStart:(NSString *)start
                       withEnd:(NSString *)end
                    withStatus:(NSString *)status
{
    NSDictionary *entry = [NSDictionary dictionaryWithObjectsAndKeys:
                           project, @"lastName",
                           xfast, @"firstName",
                           target, @"street",
                           start, @"city",
                           end, @"state",
                           status, @"zip",
                           nil];
    [_arrayController removeObject:entry];
//    
//    for (NSMutableDictionary *mob in _arrayController.arrangedObjects) {
//        NSString *projectM = [mob objectForKey:@"lastName"];
//        NSString *xfastM = [mob objectForKey:@"firstName"];
//        if ([projectM compare:project] == NSOrderedSame &&
//            [xfastM compare:xfast] == NSOrderedSame) {
//            [mob setValue:@"finished" forKey:@"zip"];
//        }
//    }
//    [_arrayController addObject:entry];
}


//<dict>
//<key>lastName</key>
//<string>Sixpack</string>
//<key>firstName</key>
//<string>Sally</string>
//<key>street</key>
//<string>451 University Avenue</string>
//<key>city</key>
//<string>Palo Alto</string>
//<key>state</key>
//<string>CA</string>
//<key>zip</key>
//<string>94301</string>
//<key>id</key>
//<string>123</string>
//</dict>


@end
