//
//  SCCLibraryXfastViewController.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/31/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCLibraryXfastViewController.h"

@interface SCCLibraryXfastViewController ()
@property (weak) IBOutlet NSTableView *tableView;
@property (strong) IBOutlet NSArrayController *arrayController;
@property NSMutableArray *array;
@end

@implementation SCCLibraryXfastViewController


- (instancetype)initWithNibName:(NSString *)nibNameOrNil
                         bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:@"SCCLibraryXfastViewController" bundle:nil];
    if (self) {
        _array = [NSMutableArray array];
        
    }
    return self;
}

- (void)addData
{
    NSDictionary *item = [NSDictionary dictionaryWithObjectsAndKeys:
                          @"Fasta", @"title",
                          @"Fasta is for handling a fasta file.", @"description",
                          [NSImage imageNamed:@"fasta-512.png"], @"image",
                          nil];
    [_array addObject:item];
    item = [NSDictionary dictionaryWithObjectsAndKeys:
            @"Fastq", @"title",
            @"Fastq is for handling a fastq file.", @"description",
            [NSImage imageNamed:@"fastq-512.png"], @"image",
            nil];
    [_array addObject:item];
    item = [NSDictionary dictionaryWithObjectsAndKeys:
            @"wc", @"title",
            @"wc counts characters, words, and lines in a text file.", @"description",
            [NSImage imageNamed:@"wc-256.png"], @"image",
            nil];
    [_array addObject:item];
    [_arrayController setContent:_array];

}

@end
