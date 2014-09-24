//
//  SCCViewControllerBuildSettings.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/22/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCViewControllerBuildSettings.h"

@interface SCCViewControllerBuildSettings ()
@property (weak) IBOutlet NSTableView *tableView;
@property (strong) IBOutlet NSDictionaryController *dictController;
@property (strong) IBOutlet NSArrayController *peopleList;
@property NSDictionary *currentPerson;

@end

@implementation SCCViewControllerBuildSettings


- (id)initWithFilePath:(NSString *)filePath
{
    self = [super initWithNibName:@"SCCViewControllerBuildSettings" bundle:nil];
    if (self) {
    }
    return self;
}


//- (void)viewDidLoad {
//    [super viewDidLoad];
//    // Do view setup here.
//}

// the keys to our array controller to be displayed in the table view,
#define KEY_LAST	@"lastName"
#define KEY_FIRST	@"firstName"

// -------------------------------------------------------------------------------
//	awakeFromNib:
// -------------------------------------------------------------------------------
- (void)awakeFromNib
{
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *path = [bundle pathForResource: @"people" ofType: @"dict"];
    NSArray *listFromFile = [NSArray arrayWithContentsOfFile: path];
    
    [_tableView setSortDescriptors:[NSArray arrayWithObjects:[[NSSortDescriptor alloc] initWithKey:@"lastName" ascending:YES],
                                   [[NSSortDescriptor alloc] initWithKey:@"firstName" ascending:YES],
                                   nil]];
    
    [_peopleList addObserver:self forKeyPath:@"selectionIndexes" options:NSKeyValueObservingOptionNew context:nil];
    
    // read the list of PEOPLE from disk in 'people.dict'
    if (listFromFile != nil)
    {
        [_peopleList addObjects:listFromFile];
    }
    
    // select the first person in the table
    [_peopleList setSelectionIndex:0];
    
    // bind the "currentPerson" dictionary to our dictionary controller
    [_dictController bind:NSContentDictionaryBinding
                 toObject:self
              withKeyPath:@"currentPerson"
                  options:nil];
    // another way is:
    // [dictController setContent:entry];
    // but that would tie the content to the current dictionary instance
    // binding the dictionary means that if we replace the dictionary itself with a new one
    // the content of the dictionary controller will be updated as well
    
    // load 2 localized key strings for display in the table to the right,
    // note: we could use localized keys for the "entire" dictionary but we are
    // illustrating here to be selective with only two keys
    //
    NSString *firstNameLocalizedKey = NSLocalizedString(@"firstName", @"");
    NSString *lastNameLocalizedKey = NSLocalizedString(@"lastName", @"");
    [_dictController setLocalizedKeyDictionary:
     [NSDictionary dictionaryWithObjectsAndKeys: firstNameLocalizedKey, KEY_FIRST,
      lastNameLocalizedKey, KEY_LAST,
      nil]];
    NSArray *incluedKeys = [NSArray arrayWithObjects:
                            @"lastName",
                            @"firstName",
                            @"street"
                            @"city", nil];
    [_dictController setIncludedKeys:incluedKeys];
    NSArray *excludedKeys = [NSArray arrayWithObjects:
                             @"id", nil];
    [_dictController setExcludedKeys:excludedKeys];
    
    // note: each person has one "excluded key" called "id",
    // which we could use as private data - not to be displayed to the user.
}

// -------------------------------------------------------------------------------
//	sortDesc:
// -------------------------------------------------------------------------------
- (NSArray *)sortDesc
{
    return [_tableView sortDescriptors];
}

// -------------------------------------------------------------------------------
//	observeValueForKeyPath:keyPath:object:change:context
// -------------------------------------------------------------------------------
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    // has the array controller's "selectionIndexes" binding changed?
    if (object == _peopleList)
    {
        if ([[object selectedObjects] count] > 0)
        {
            // update our current person and reflect the change to our dictionary controller
            [self setCurrentPerson: [[object selectedObjects] objectAtIndex:0]];
            [_dictController bind:NSContentDictionaryBinding toObject:self withKeyPath:@"currentPerson" options:nil];
        }
    }
    else
    {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}



@end
