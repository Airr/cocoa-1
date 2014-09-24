//
//  SCCCommand.h
//  Xfast
//
//  Created by Sang Chul Choi on 9/2/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//
#import <Cocoa/Cocoa.h>

extern NSString *kJobDidFinish;

@protocol SCCCommand <NSObject>

@property (copy) NSString *type;
@property NSDictionary    *buildSettings;
@property NSArray         *sourceBuildFiles;
@property NSArray         *outputBuildFiles;

/**
 *  Returns the executable's path
 *
 *  @return The path to the executable
 */
- (NSString *)launchPath;

/**
 *  Returns the array of options to the executable
 *
 *  @return The array of options
 */
- (NSArray *)arguments;

/**
 *  Returns the array of main input files.
 *
 *  @return The array of input files.
 */
- (NSArray *)inputFiles;

/**
 *  Returns the array of main output files.
 *
 *  @return The array of output files.
 */
- (NSArray *)outputFiles;

/**
 *  Returns the absolute path of the file at an index-th position.
 *
 *  @param index An index within the bounds of the array.
 *
 *  @return The NSString located at index.
 */
- (NSString *)inputFileAtIndex:(NSUInteger)index;

/**
 *  Returns the absolute path of the file at an index-th position.
 *
 *  @param index An index within the bounds of the array.
 *
 *  @return The NSString located at index.
 */
- (NSString *)outputFileAtIndex:(NSUInteger)index;

/**
 *  Returns the array of arguments.
 *
 *  @return The array of arguments.
 */
- (NSArray *)arrayOfArguments;

//- (void)setBuildSettings:(NSDictionary *)buildSettings;
//
//- (void)setSourceBuildFiles:(NSArray *)files;
//
//- (void)setOutputBuildFiles:(NSArray *)files;


@end