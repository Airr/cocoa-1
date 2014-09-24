//
//  SCCCommandOfcat.m
//  Xfast
//
//  Created by Sang Chul Choi on 9/4/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCCommandOfcat.h"
#import "XfastEditor.h"

@implementation SCCCommandOfcat

@synthesize type = _type;
@synthesize buildSettings = _buildSettings;
@synthesize sourceBuildFiles = _sourceBuildFiles;
@synthesize outputBuildFiles = _outputBuildFiles;

- (NSString *)launchPath
{
    NSString *path  = [NSString stringWithFormat:@"%@",
                       [[NSBundle mainBundle] pathForResource:_type
                                                       ofType:@"command"]];
    return path;
}


- (NSArray *)arguments
{
    // We need a copy of members.
    // Use the
    
//    NSString *infiles = [[_sourceBuildFiles valueForKey:@"description"] componentsJoinedByString:@""];
//    NSString *outfile = [_outputBuildFiles firstObject];
    
    NSMutableString *infiles = [NSMutableString string];
    for (XFSourceFile *file in _sourceBuildFiles) {
        [infiles appendString:[file absolutePath]];
        [infiles appendString:@" "];
    }
    
    NSMutableString *outfiles = [NSMutableString string];
    for (XFSourceFile *file in _outputBuildFiles) {
        [outfiles appendString:[file absolutePath]];
//        [outfiles appendString:@" "];
    }
    
    // option, input files, and a single output file.
    // See cat.command for the three arguments.
    // The output file is created at
    // /Users/goshng/Library/Developer/Xcode/DerivedData/Xfast-agiydlcybkmshzaimcxzxwlrhdzo/Build/Products/Debug
    // because the path is relative to the working directory.
    return [NSArray arrayWithObjects:@"", infiles, outfiles, nil];
}


- (NSArray *)inputFiles
{
    return nil;
}


- (NSArray *)outputFiles
{
    return nil;
}

- (NSString *)inputFileAtIndex:(NSUInteger)index
{
    XFSourceFile *file = [_sourceBuildFiles objectAtIndex:index];
    return [file absolutePath];
}

/**
 *  Returns the absolute path of the index-th output file.
 *
 *  @param index An index within the bounds of the array.
 *
 *  @return The NSString located at index.
 *
 *
 */
- (NSString *)outputFileAtIndex:(NSUInteger)index
{
    XFSourceFile *file = [_sourceBuildFiles objectAtIndex:index];
    //    XFSourceFile *file = [_outputBuildFiles objectAtIndex:index];
    
    return [[file absolutePath] stringByAppendingString:@".out"];
}

- (NSArray *)arrayOfArguments
{
    NSMutableArray *array = [NSMutableArray array];
    for (NSUInteger i = 0; i < [_sourceBuildFiles count]; i++) {
        NSMutableArray *item = [NSMutableArray arrayWithObjects:
                                [self inputFileAtIndex:i],
                                [self outputFileAtIndex:i] , nil];
        [array addObject:item];
    }
    return array;
}

@end
