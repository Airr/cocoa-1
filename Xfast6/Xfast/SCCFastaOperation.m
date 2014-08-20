//
//  SCCFastaOperation.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCFastaOperation.h"
#import "XfastEditor.h"

@implementation SCCFastaOperation

-(id)initWithXFProject:(XFProject *)project
{
    if (self = [super init])
    {
        _project = project;
        _commandLine = [self commandLineOfCurrentTarget:project];
        
    }
    return self;
}

- (NSString *)commandLineOfCurrentTarget:(XFProject *)project
{
    NSMutableString *commandLine = [NSMutableString stringWithFormat:@" "];
    XFTarget *target = [project currentTarget];
    NSArray *members = [target members];
    [members enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        XFSourceFile *file = (XFSourceFile *)obj;
        [commandLine appendFormat:@"%@", [file absolutePath]];
    }];
    
    return commandLine;
}

-(void)main
{
    @try {
        // Do some work on myData and report the results.
        NSLog(@"Welcome - main: BEGIN");
        NSString *filePath = [_commandLine stringByReplacingOccurrencesOfString:@" " withString:@""];
        NSLog(@"Welcome [%@]", _commandLine);
        
        // absoluteString <-> URLWithString
        // path <-> fileURLWithPath
        //        NSURL *fileURL = [NSURL fileURLWithPath:filePath];
        NSURL *fileURL = [NSURL URLWithString:filePath];
        
        NSError *error;
        NSStringEncoding usedEnconing;
        NSString *cotentFile = [NSString stringWithContentsOfURL:fileURL
                                                    usedEncoding:&usedEnconing
                                                           error:&error];
        NSInteger length = [[cotentFile componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]] count];
        NSLog(@"File %@ - %lu lines.", fileURL, length);
        NSLog(@"Welcome - main: END");
    }
    @catch(...) {
        // Do not rethrow exceptions.
    }
}

@end
