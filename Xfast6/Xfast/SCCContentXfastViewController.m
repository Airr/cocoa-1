//
//  SCCContentXfastViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/29/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCContentXfastViewController.h"
#import "SCCProjectDocument.h"

@interface SCCContentXfastViewController ()

@property (strong) IBOutlet NSDictionaryController *dictionaryController;
@property (weak) IBOutlet NSTableView *tableView;
@property NSMutableDictionary *settings;

@property (nonatomic, weak) SCCProjectDocument *document;

@end

@implementation SCCContentXfastViewController

- (instancetype)initWithNibName:(NSString *)nibNameOrNil
                         bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCContentXfastViewController" bundle:nil];
    if (self) {
        
    }
    return self;
}

- (void)dealloc
{
    //[self removeObserver:self forKeyPath:@"settings"];
}

- (void)awakeFromNib
{
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    // has the array controller's "selectionIndexes" binding changed?
    if (object == self)
    {
        if ([keyPath compare:@"settings"] == NSOrderedSame)
        {
            [[self document] pushContentXfast];
        }
    }
    else
    {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}



- (SCCProjectDocument *)document
{
    if (_document == nil) {
        _document = [[[[self view] window] windowController] document];
    }
    return _document;
}

- (IBAction)add:(id)sender
{
    if ([[_dictionaryController selectedObjects] count] > 0) {
        //
        NSUInteger index = [_dictionaryController selectionIndex];
        id obj = [_dictionaryController newObject];
        [obj setKey:@"key"];
        [obj setValue:@"value"];
        [_dictionaryController insertObject:obj atArrangedObjectIndex:index];
    } else {
        // Add it at the end of the list.
        id obj = [_dictionaryController newObject];
        [obj setKey:@"key"];
        [obj setValue:@"value"];
        [_dictionaryController addObject:obj];
    }
    
    // Save the new setting value.
    [[self document] pushContentXfast];
}

- (IBAction)remove:(id)sender
{
    if ([[_dictionaryController selectedObjects] count] > 0) {
        //
        NSIndexSet *indices = [_dictionaryController selectionIndexes];
        [_dictionaryController removeObjectsAtArrangedObjectIndexes:indices];
    
        // Save the new setting value.
        [[self document] pushContentXfast];
    }
}

- (void)changeSettings:(NSMutableDictionary *)settings
{
    [self setSettings:settings];
//    NSArray *incluedKeys = [NSArray arrayWithObjects:
//                            @"key1",
//                            @"key2",
//                            nil];
//    [_dictionaryController setIncludedKeys:incluedKeys];
    NSArray *excludedKeys = [NSArray arrayWithObjects:
                             @"id", nil];
    [_dictionaryController setExcludedKeys:excludedKeys];

    [_dictionaryController setContent:_settings];
//    [_dictionaryController bind:NSContentDictionaryBinding
//                       toObject:self
//                    withKeyPath:@"settings"
//                        options:nil];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
//    NSDictionary *userInfo = [obj userInfo];
//    NSTextView *aView = [userInfo valueForKey:@"NSFieldEditor"];
//    NSLog(@"controlTextDidEndEditing %@", [aView string] );
    [[self document] pushContentXfast];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
//    NSDictionary *userInfo = [aNotification userInfo];
//    NSTextView *aView = [userInfo valueForKey:@"NSFieldEditor"];
//    NSLog(@"controlTextDidChange >>%@<<", [aView string] );
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
//    NSLog(@"control: textShouldEndEditing >%@<", [fieldEditor string] );
    if ([[fieldEditor string] length] > 0) {
        return YES;
    } else {
        return NO;
    }
}

@end
