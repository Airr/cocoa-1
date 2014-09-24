////////////////////////////////////////////////////////////////////////////////
//
//  JASPER BLUES
//  Copyright 2012 - 2013 Jasper Blues
//  All Rights Reserved.
//
//  NOTICE: Jasper Blues permits you to use, modify, and distribute this file
//  in accordance with the terms of the license agreement accompanying it.
//
////////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import "XcodeMemberType.h"
#import "XcodeSourceFileType.h"

@class XCClassDefinition;
@class XCGroup;
@class XCFileOperationQueue;
@class XCSourceFile;
@class XCTarget;
@class XCSubProjectDefinition;
@class XCBuildConfiguration;

NSString* const XCProjectNotFoundException;

@interface XCProject : NSObject
{
@protected
    // Set these in init method.
    XCFileOperationQueue* _fileOperationQueue;
    NSString* _filePath;
    NSMutableDictionary* _dataStore;
    
    // Do not set this in init method.
    NSMutableArray* _targets;
    NSMutableDictionary* _groups;
    NSMutableDictionary* _configurations;
    NSString* _rootObjectKey;
    NSString* _defaultConfigurationName;
}

@property(nonatomic, strong, readonly) XCFileOperationQueue* fileOperationQueue;

#pragma mark Inits

/**
 * Creates a new project in the specified path. If the new directory exists or
 * the specified directory cannot be created, it returns nil.
 */
+ (XCProject*)projectWithNewFilePath:(NSString*)filePath;


+ (XCProject*)projectWithFilePath:(NSString*)filePath;


+ (NSString *)relativePath:(NSString *)path1 relativeTo:(NSString *)path2;

+ (NSString *)relativeDir:(NSString *)path1 relativeTo:(NSString *)path2;

- (id)initWithFilePath:(NSString*)filePath create:(BOOL)needToCreat;

- (id)initWithFilePath:(NSString*)filePath;

- (NSMutableDictionary *)objectGroupWithKeys:(NSArray *)children
                                    withName:(NSString *)name;


#pragma mark - Interface Methods

- (XcodeMemberType)memberTypeWithKey:(NSString*)key;

#pragma mark - Xfast File

- (void)addXfastToProject:(NSString *)type withName:(NSString *)name withKey:(NSString *)key;

#pragma mark Streams

/**
* Returns all file resources in the project, as an array of `XCSourceFile` objects.
*/
- (NSArray*)files;

/**
* Returns the project file with the specified key, or nil.
*/
- (XCSourceFile*)fileWithKey:(NSString*)key;

/**
* Returns the project file with the specified name, or nil. If more than one project file matches the specified name,
* which one is returned is undefined.
*/
- (XCSourceFile*)fileWithName:(NSString*)name;

/**
* Returns all header files in the project, as an array of `XCSourceFile` objects.
*/
- (NSArray*)headerFiles;

/**
* Returns all implementation obj-c implementation files in the project, as an array of `XCSourceFile` objects.
*/
- (NSArray*)objectiveCFiles;

/**
* Returns all implementation obj-c++ implementation files in the project, as an array of `XCSourceFile` objects.
*/
- (NSArray*)objectiveCPlusPlusFiles;

/**
* Returns all the xib files in the project, as an array of `XCSourceFile` objects.
*/
- (NSArray*)xibFiles;

- (NSArray*)imagePNGFiles;

- (NSString*)filePath;


#pragma mark Groups


- (void)removeAllGroup;

- (void)printTree;

- (NSMutableArray *)asTreeData;

/**
* Lists the groups in an xcode project, returning an array of `XCGroup` objects.
*/
- (NSArray*)groups;

/**
 * Returns the root (top-level) group.
 */
- (XCGroup*)rootGroup;

/**
 * Returns the root (top-level) groups, if there are multiple. An array of rootGroup if there is only one.
 */
- (NSArray*)rootGroups;

/**
* Returns the group with the given key, or nil.
*/
- (XCGroup*)groupWithKey:(NSString*)key;

/**
 * Returns the group with the specified display name path - the directory relative to the root group. Eg Source/Main
 */
- (XCGroup*)groupWithPathFromRoot:(NSString*)path;

/**
* Returns the parent group for the group or file with the given key;
*/
- (XCGroup*)groupForGroupMemberWithKey:(NSString*)key;

/**
 * Returns the parent group for the group or file with the source file
 */
- (XCGroup*)groupWithSourceFile:(XCSourceFile*)sourceFile;

- (NSString *)addSourceBuildPhase;


#pragma mark - Targets

- (NSString *)name;

- (BOOL)addTarget:(NSString *)aTarget;

- (BOOL)addXCTarget:(XCTarget *)aTarget;

/**
* Lists the targets in an xcode project, returning an array of `XCTarget` objects.
*/
- (NSArray*)targets;

/**
* Returns the target with the specified name, or nil. 
*/
- (XCTarget*)targetWithName:(NSString*)name;

#pragma mark - Configuration List

#pragma mark - Build Configurations

/**
* Returns the target with the specified name, or nil. 
*/
- (NSDictionary*)configurations;

- (NSDictionary*)configurationWithName:(NSString*)name;

- (XCBuildConfiguration*)defaultConfiguration;

- (BOOL)addBuildConfiguration:(XCBuildConfiguration *)aBuildConfiguration;

#pragma mark - Saving

- (void)setObject:(id)anObject;

- (void)setObject:(id)anObject forKey:(id<NSCopying>)aKey;

/**
* Saves a project after editing.
*/
- (void)save;

- (void)saveOnlyProjectFile;

/**
* Raw project data.
*/
- (NSMutableDictionary*)objects;

- (NSMutableDictionary*)dataStore;

- (void)dropCache;


- (NSString*)rootObjectKey;

- (NSString *)mainGroupKey;

@end
