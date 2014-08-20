////////////////////////////////////////////////////////////////////////////////
//
//  JASPER BLUES
//  Copyright 2012 Jasper Blues
//  All Rights Reserved.
//
//  NOTICE: Jasper Blues permits you to use, modify, and distribute this file
//  in accordance with the terms of the license agreement accompanying it.
//
////////////////////////////////////////////////////////////////////////////////


#import <Foundation/Foundation.h>


@interface XFFileOperationQueue : NSObject
{

@private
    NSString* _baseDirectory;
    NSMutableDictionary* _filesToWrite;
    NSMutableDictionary* _frameworksToCopy;
    NSMutableArray* _filesToDelete;
    NSMutableArray* _directoriesToCreate;
}

/**
 * Initializes this file operation with a directory.
 */
- (id)initWithBaseDirectory:(NSString*)baseDirectory;

/**
 * Returns YES if the file exists in the directory.
 */
- (BOOL)fileWithName:(NSString*)name existsInProjectDirectory:(NSString*)directory;

/**
 * Writes the content to a file.
 */
- (void)queueTextFile:(NSString*)fileName inDirectory:(NSString*)directory withContents:(NSString*)contents;

/**
 * Writes the data ta a file.
 */
- (void)queueDataFile:(NSString*)fileName inDirectory:(NSString*)directory withContents:(NSData*)contents;


- (void)queueFrameworkWithFilePath:(NSString*)filePath inDirectory:(NSString*)directory;

/**
 * Deletes a file.
 */
- (void)queueDeletion:(NSString*)filePath;

/**
 * Creates a directory in the parent directory.
 */
- (void)queueDirectory:(NSString*)withName inDirectory:(NSString*)parentDirectory;

/**
 * Executes file operations in the queues.
 */
- (void)commitFileOperations;

@end

