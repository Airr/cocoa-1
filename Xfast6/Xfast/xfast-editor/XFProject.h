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
#import "XfastSourceFileType.h"

@class XFClassDefinition;
@class XFGroup;
@class XFFileOperationQueue;
@class XFSourceFile;
@class XFTarget;
@class XFSubProjectDefinition;
@class XFBuildConfiguration;

NSString* const XFProjectNotFoundException;

@interface XFProject : NSObject
{
@protected
    XFFileOperationQueue* _fileOperationQueue;

    NSString* _filePath;
    NSMutableDictionary* _dataStore;
    NSMutableArray* _targets;

    NSMutableDictionary* _groups;
    NSMutableDictionary* _configurations;

    NSString* _defaultConfigurationName;
    NSString* _rootObjectKey;
}

@property (nonatomic,copy) NSString *currentTargetKey;

@property(nonatomic, strong, readonly) XFFileOperationQueue* fileOperationQueue;

#pragma mark Inits

+ (XFProject*)projectWithNewFilePath:(NSString*)filePath
                       baseDirectory:(NSString *)dirPath;

+ (XFProject*)projectWithFilePath:(NSString*)filePath
                    baseDirectory:(NSString *)dirPath;

- (id)initWithNewFilePath:(NSString*)filePath
            baseDirectory:(NSString *)dirPath;

/**
* Creates a new project editor instance with the specified Project.xcodeproj file.
*/
- (id)initWithFilePath:(NSString*)filePath baseDirectory:(NSString *)dirPath;
//- (id)initWithFilePath:(NSString*)filePath;

#pragma mark Properties

- (NSString *)name;

#pragma mark Files
/**
* Returns all file resources in the project, as an array of `XFSourceFile` objects.
*/
- (NSArray*)files;

/**
* Returns the project file with the specified key, or nil.
*/
- (XFSourceFile*)fileWithKey:(NSString*)key;

/**
* Returns the project file with the specified name, or nil. If more than one project file matches the specified name,
* which one is returned is undefined.
*/
- (XFSourceFile*)fileWithName:(NSString*)name;

/**
* Returns all header files in the project, as an array of `XFSourceFile` objects.
*/
- (NSArray*)headerFiles;

/**
* Returns all implementation obj-c implementation files in the project, as an array of `XFSourceFile` objects.
*/
- (NSArray*)objectiveCFiles;

/**
* Returns all implementation obj-c++ implementation files in the project, as an array of `XFSourceFile` objects.
*/
- (NSArray*)objectiveCPlusPlusFiles;

/**
* Returns all the xib files in the project, as an array of `XFSourceFile` objects.
*/
- (NSArray*)xibFiles;

- (NSArray*)imagePNGFiles;

- (NSString*)filePath;


/* ====================================================================================================================================== */
#pragma mark Groups
/**
* Lists the groups in an xcode project, returning an array of `XFGroup` objects.
*/
- (NSArray*)groups;

/**
 * Returns the root (top-level) group.
 */
- (XFGroup*)rootGroup;

/**
 * Returns the root (top-level) groups, if there are multiple. An array of rootGroup if there is only one.
 */
- (NSArray*)rootGroups;

/**
* Returns the group with the given key, or nil.
*/
- (XFGroup*)groupWithKey:(NSString*)key;

/**
 * Returns the group with the specified display name path - the directory relative to the root group. Eg Source/Main
 */
- (XFGroup*)groupWithPathFromRoot:(NSString*)path;

/**
* Returns the parent group for the group or file with the given key;
*/
- (XFGroup*)groupForGroupMemberWithKey:(NSString*)key;

/**
 * Returns the parent group for the group or file with the source file
 */
- (XFGroup*)groupWithSourceFile:(XFSourceFile*)sourceFile;

#pragma mark Command Lines



#pragma mark Targets

- (XFTarget*)currentTarget;

- (XFTarget*)targetWithKey:(NSString*)key;

- (void)addTarget:(XFTarget *)target;

/**
* Lists the targets in an xcode project, returning an array of `XFTarget` objects.
*/
- (NSArray*)targets;

/**
* Returns the target with the specified name, or nil. 
*/
- (XFTarget*)targetWithName:(NSString*)name;

#pragma mark Configurations

/**
* Returns the target with the specified name, or nil. 
*/
- (NSDictionary*)configurations;

- (NSDictionary*)configurationWithName:(NSString*)name;

- (XFBuildConfiguration*)defaultConfiguration;

/* ====================================================================================================================================== */
#pragma mark Saving
/**
* Saves a project after editing.
*/
- (void)save;

/**
 * Saves a project to project.pbxproj not to the given filepath.
 */
- (void)saveToPBXProj;

/* ====================================================================================================================================== */
/**
* Raw project data.
*/
- (NSMutableDictionary*)objects;

- (NSMutableDictionary*)dataStore;

- (void)dropCache;

@end
