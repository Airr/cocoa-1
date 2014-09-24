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


#import "XCProject.h"
#import "XCGroup.h"
#import "XCSourceFile.h"
#import "XCTarget.h"
#import "XCFileOperationQueue.h"
#import "XCBuildConfiguration.h"
#import "XCConfigurationList.h"
#import "XCSourceFileDefinition.h"
#import "XCKeyBuilder.h"

@interface XCProject ()

@end

/**
 *  XCProject is originally developed by JASPER BLUES. This class is used
 * to manage the Xfast project.
 *
 * @see XFProject
 */
@implementation XCProject


@synthesize fileOperationQueue = _fileOperationQueue;

#pragma mark - Class Methods

+ (XCProject*)projectWithNewFilePath:(NSString*)filePath
{
    return [[XCProject alloc] initWithFilePath:filePath create:YES];
}

/**
 *  Creates a project instance. The file path must exist.
 *
 *  @param filePath path
 *
 *  @return XCProject
 */
+ (XCProject*)projectWithFilePath:(NSString*)filePath
{
    return [[XCProject alloc] initWithFilePath:filePath];
}

/**
 *  Returns a file path of the first one relative to to the
 * second.
 *
 *  @param path1 The first file path
 *  @param path2 The second file path
 *
 *  @return The relative file path. It returns nil if the last components are 
 * different. Two paths must be absolute paths, or relative paths. It returns
 * nil if one path is absolute, and the other is relative.
 *
 *  @code
 *  path1: 0/1/2/3/file
 *  path2: 0/1/file
 *  -> 2/3/file
 *  @endcode
 *
 *  @code
 *  path1: 1/1/2/3/file
 *  path2: 0/1/file
 *  -> ../../1/1/2/3/file
 *  @endcode
 *
 */
+ (NSString *)relativePath:(NSString *)path1 relativeTo:(NSString *)path2
{
    NSString *filename = [path1 lastPathComponent];
    NSString *filename2 = [path2 lastPathComponent];
    if ([filename compare:filename2] != NSOrderedSame) {
        return nil;
    }
    NSArray *a1 = [[path1 stringByDeletingLastPathComponent] pathComponents];
    NSArray *a2 = [[path2 stringByDeletingLastPathComponent] pathComponents];
    NSString *first1 = [a1 firstObject];
    NSString *first2 = [a2 firstObject];
    if ([first1 compare:@"/"] == NSOrderedSame ||
        [first2 compare:@"/"] == NSOrderedSame) {
        if ([first1 compare:first2] != NSOrderedSame) {
            return nil;
        }
    }
    
    NSUInteger i = 0;
    while (i < [a1 count] && i < [a2 count] &&
           [[a1 objectAtIndex:i] compare:[a2 objectAtIndex:i]] == NSOrderedSame) {
        i++;
    }
    NSMutableString *s = [NSMutableString stringWithCapacity:10];
    NSUInteger j = [a2 count] - i;
    
    NSMutableArray *dots = [NSMutableArray arrayWithCapacity:j];
    for (NSUInteger k = 0; k < j; k++) {
        [dots setObject:@".." atIndexedSubscript:k];
    }
    NSString *dotsString = [NSString pathWithComponents:dots];
    
    NSRange r11 = NSMakeRange(i, [a1 count] - i);
    NSArray *a11 = [a1 subarrayWithRange:r11];
    NSString *suffixPath = [NSString pathWithComponents:a11];
    NSString *relativePath = [dotsString stringByAppendingPathComponent:suffixPath];
    NSString *relativeFilepath = [relativePath stringByAppendingPathComponent:filename];
    return relativeFilepath;
}

/**
 *  Returns a directory of the first one relative to to the second.
 *
 *  @param path1 The first directory
 *  @param path2 The second directory
 *
 *  @return The relative directory.
 * Two paths must be absolute paths, or relative paths. It returns
 * nil if one path is absolute, and the other is relative.
 *
 *  @code
 *  path1: 0/1/2/3
 *  path2: 0/1
 *  -> 2/3
 *  @endcode
 *
 *  @code
 *  path1: 1/1/2/3
 *  path2: 0/1
 *  -> ../../1/1/2/3
 *  @endcode
 *
 */
+ (NSString *)relativeDir:(NSString *)path1 relativeTo:(NSString *)path2
{
    NSArray *a1 = [path1 pathComponents];
    NSArray *a2 = [path2 pathComponents];
    NSString *first1 = [a1 firstObject];
    NSString *first2 = [a2 firstObject];
    if ([first1 compare:@"/"] == NSOrderedSame ||
        [first2 compare:@"/"] == NSOrderedSame) {
        if ([first1 compare:first2] != NSOrderedSame) {
            return nil;
        }
    }
    
    NSUInteger i = 0;
    while (i < [a1 count] && i < [a2 count] &&
           [[a1 objectAtIndex:i] compare:[a2 objectAtIndex:i]] == NSOrderedSame) {
        i++;
    }
    NSMutableString *s = [NSMutableString stringWithCapacity:10];
    NSUInteger j = [a2 count] - i;
    
    NSMutableArray *dots = [NSMutableArray arrayWithCapacity:j];
    for (NSUInteger k = 0; k < j; k++) {
        [dots setObject:@".." atIndexedSubscript:k];
    }
    NSString *dotsString = [NSString pathWithComponents:dots];
    
    NSRange r11 = NSMakeRange(i, [a1 count] - i);
    NSArray *a11 = [a1 subarrayWithRange:r11];
    NSString *suffixPath = [NSString pathWithComponents:a11];
    NSString *relativePath = [dotsString stringByAppendingPathComponent:suffixPath];
    return relativePath;
}


#pragma mark - Initialization & Destruction

/**
 *  Creates folders at the filePath directory.
 *
 *  @param filePath The base directory.
 *
 *  @return The project path: if the base directory is @b /path/to/project,
 * it returns @b /path/to/project/project.xfastproj
 */
- (NSString *)createBaseFolders:(NSString *)filePath
{
     NSFileManager* fileManager = [NSFileManager defaultManager];
    // 1. Create directories
    NSString *projectName = [filePath lastPathComponent];
    NSString *projectMainPath = [filePath stringByAppendingPathComponent:projectName];
//    NSString *projectXfastPath = [filePath stringByAppendingPathComponent:@"xfastfolder"];
    NSString *projectPath = [projectMainPath stringByAppendingPathExtension:@"xfastproj"];
    [fileManager createDirectoryAtPath:projectMainPath
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];
//    [fileManager createDirectoryAtPath:projectXfastPath
//           withIntermediateDirectories:YES
//                            attributes:nil
//                                 error:nil];
    [fileManager createDirectoryAtPath:projectPath
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];
    return projectPath;
}


/**
 *  Creates a new project instance with the specified path. It returns nil, if
 * the path exists or the specified directory cannot be created.
 *
 *  If the filePath is @b /path/to/last, the last component, @b last, becomes 
 * the project name. Three folders are created in the filePath:
 * 
 * - Main path: @b /path/to/last/last
 *
 * - Xfast path: @b /path/to/last/xfastfolder
 *
 * - Project file path: @b /path/to/last/last.xfastproj
 *
 *  @param filePath A path string identifying the XCProject directory to create.
 *         You must specify a full path. This parameter must not be nil.
 *
 *  @return XCProject
 */
- (id)initWithFilePath:(NSString*)filePath create:(BOOL)needToCreate
{
    self = [super init];
    if (self)
    {
        // 1. Check folder
        NSMutableString *projectPath = [NSMutableString stringWithString:@""];
        
        if (needToCreate) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            if ([fileManager fileExistsAtPath:filePath]) {
                NSLog(@"should return nil;");
            }
            // 2. Create folder
            [projectPath appendString:[self createBaseFolders:filePath]];
        } else {
            [projectPath appendString:filePath];
        }
        
        // 3. Set object members
        _filePath = [projectPath copy];
        
        if (needToCreate) {
            _dataStore = [self createData];
        } else {
            _dataStore = [[NSMutableDictionary alloc]
                          initWithContentsOfFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]];
            
            if (!_dataStore)
            {
                [NSException raise:XCProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
            }
        }
        _fileOperationQueue = [[XCFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];

    }
    return self;
}

/**
 *  Creates a new project editor instance with the specified Project.xcodeproj
 * file.
 *
 *  The file path must be of the form of @b /path/to/last/last.xfastproj.
 *
 *  @param filePath A path to the xfastproj folder. The path must be a project
 * folder, not a base folder.
 *
 *  @return XCProject
 * 
 *  @sa initWithNewFilePath:
 */
- (id)initWithFilePath:(NSString*)filePath
{
    return [self initWithFilePath:filePath create:NO];
//    if ((self = [super init]))
//    {
//        _filePath = [filePath copy];
//        _dataStore = [[NSMutableDictionary alloc]
//                      initWithContentsOfFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]];
//
//        if (!_dataStore)
//        {
//            [NSException raise:XCProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
//        }
//
//        _fileOperationQueue = [[XCFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];
//
//    }
//    return self;
}

#pragma mark - Interface Methods

- (NSString *)memberFilePath:(NSString *)key
{
    XCSourceFile *file = [self fileWithKey:key];
    return nil;
}

- (XcodeMemberType)memberTypeWithKey:(NSString*)key
{
    NSDictionary* obj = [[self objects] valueForKey:key];
    if (obj == nil) {
        return PBXNilType;
    } else {
        return [[obj valueForKey:@"isa"] asMemberType];
    }
}


#pragma mark - Streams


- (BOOL)writeWithDictionary:(NSDictionary *)data ToFile:(NSString *)path
{
    NSMutableString *contentProjectFile = [NSMutableString stringWithCapacity:1000];
    
    NSString *objects = [self stringObjectsWithDictionary:[data objectForKey:@"objects"]
                                                  rootKey:[data objectForKey:@"rootObject"]];
    
    [contentProjectFile appendFormat:
     @"// !$*UTF8*$!\n"
     @"{\n"
     @"\tarchiveVersion = %@;\n"
     @"\tclasses = {\n"
     @"\t};\n"
     @"\tobjectVersion = %@;\n"
     @"\trootObject = %@;\n"
     @"\tobjects = {\n"
     @"%@"
     @"\t};\n"
     @"}\n",
     [data objectForKey:@"archiveVersion"],
     [data objectForKey:@"objectVersion"],
     [data objectForKey:@"rootObject"],
     objects
     ];
    
    [contentProjectFile writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
    return YES;
}

- (NSString *)stringObjectsWithDictionary:(NSDictionary *)data
                                  rootKey:(NSString *)key
{
    NSMutableString *content = [NSMutableString stringWithCapacity:1000];
    
    [content appendFormat:@"/* Begin PBXProject section */\n"];
    NSDictionary *project = [data objectForKey:key];
    [content appendFormat:@"\t\t%@ /* Project object */ = {\n", key];
    [content appendFormat:@"\t\t\tisa = %@;\n", [project objectForKey:@"isa"]];
    [content appendFormat:@"\t\t\tmainGroup = %@;\n", [project objectForKey:@"mainGroup"]];
    [content appendFormat:@"\t\t\tbuildConfigurationList = %@;\n", [project objectForKey:@"buildConfigurationList"]];
    [content appendFormat:@"\t\t\tcompatibilityVersion = \"%@\";\n",
     [project objectForKey:@"compatibilityVersion"]];
    [content appendFormat:@"\t\t};\n"];
    [content appendFormat:@"/* End PBXProject section */\n\n"];
    
    [content appendFormat:@"/* Begin XCBuildConfiguration section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XCBuildConfigurationType)
        {
            [content appendFormat:@"\t\t%@ /* Release */ = {\n", key];
            [content appendFormat:@"\t\t\tisa = %@;\n", [obj objectForKey:@"isa"]];
            [content appendFormat:@"\t\t\tname = %@;\n", [obj objectForKey:@"name"]];
            [content appendFormat:@"\t\t\tbuildSettings = {\n"];
            [[obj valueForKey:@"buildSettings"] enumerateKeysAndObjectsUsingBlock:^(NSString* key2, NSDictionary* obj2, BOOL* stop2)
             {
                 [content appendFormat:@"\t\t\t\t%@ = %@;\n", key2, obj2];
                 
             }];
            [content appendFormat:@"\t\t\t};\n"];
            [content appendFormat:@"\t\t};\n"];
        }
    }];
    [content appendFormat:@"/* End XCBuildConfiguration section */\n\n"];
  
    [content appendFormat:@"/* Begin XCConfigurationList section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XCConfigurationListType)
        {
            [content appendFormat:@"\t\t%@ /* Build configuration list for PBXProject */ = {\n", key];
            [content appendFormat:@"\t\t\tisa = %@;\n", [obj objectForKey:@"isa"]];
            [content appendFormat:@"\t\t\tdefaultConfigurationIsVisible = %@;\n", [obj objectForKey:@"defaultConfigurationIsVisible"]];
            [content appendFormat:@"\t\t\tdefaultConfigurationName = %@;\n", [obj objectForKey:@"defaultConfigurationName"]];
            
            [content appendFormat:@"\t\t\tbuildConfigurations = (\n"];
            NSEnumerator *enumerator = [[obj valueForKey:@"buildConfigurations"] objectEnumerator];
            id anObject;
            while (anObject = [enumerator nextObject]) {
                 [content appendFormat:@"\t\t\t\t%@,\n", anObject];
                
            }
            [content appendFormat:@"\t\t\t);\n"];
            [content appendFormat:@"\t\t};\n"];
        }
    }];
    [content appendFormat:@"/* End XCConfigurationList section */\n\n"];
    
    [content appendFormat:@"/* Begin PBXGroup section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == PBXGroupType)
        {
            [content appendFormat:@"\t\t%@ = {\n", key];
            [content appendFormat:@"\t\t\tisa = %@;\n", [obj objectForKey:@"isa"]];
            [content appendFormat:@"\t\t\tsourceTree = %@;\n", [obj objectForKey:@"sourceTree"]];
            
            [content appendFormat:@"\t\t\tchildren = (\n"];
            NSEnumerator *enumerator = [[obj valueForKey:@"children"] objectEnumerator];
            id anObject;
            while (anObject = [enumerator nextObject]) {
                 [content appendFormat:@"\t\t\t\t%@,\n", anObject];
                
            }
            [content appendFormat:@"\t\t\t);\n"];
            [content appendFormat:@"\t\t};\n"];
        }
    }];
    [content appendFormat:@"/* End PBXGroup section */\n\n"];
    
    [content appendFormat:@"/* Begin PBXSourcesBuildPhase section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == PBXSourcesBuildPhaseType)
        {
            [content appendFormat:@"\t\t%@ = {\n", key];
            [content appendFormat:@"\t\t\tisa = %@;\n", [obj objectForKey:@"isa"]];
            [content appendFormat:@"\t\t\trunOnlyForDeploymentPostprocessing = %@;\n", [obj objectForKey:@"runOnlyForDeploymentPostprocessing"]];
            
            [content appendFormat:@"\t\t\tfiles = (\n"];
            NSEnumerator *enumerator = [[obj valueForKey:@"files"] objectEnumerator];
            id anObject;
            while (anObject = [enumerator nextObject]) {
                 [content appendFormat:@"\t\t\t\t%@,\n", anObject];
                
            }
            [content appendFormat:@"\t\t\t);\n"];
            [content appendFormat:@"\t\t};\n"];
        }
    }];
    [content appendFormat:@"/* End PBXSourcesBuildPhase section */\n\n"];
    
    [content appendFormat:@"/* Begin PBXNativeTarget section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == PBXNativeTargetType)
        {
            [content appendFormat:@"\t\t%@ = {\n", key];
            [content appendFormat:@"\t\t\tisa = %@;\n", [obj objectForKey:@"isa"]];
            [content appendFormat:@"\t\t\tbuildConfigurationList = %@;\n", [obj objectForKey:@"buildConfigurationList"]];
            
            [content appendFormat:@"\t\t\tbuildPhases = (\n"];
            NSEnumerator *enumerator = [[obj valueForKey:@"buildPhases"] objectEnumerator];
            id anObject;
            while (anObject = [enumerator nextObject]) {
                 [content appendFormat:@"\t\t\t\t%@,\n", anObject];
                
            }
            [content appendFormat:@"\t\t\t);\n"];
            [content appendFormat:@"\t\t};\n"];
        }
    }];
    [content appendFormat:@"/* End PBXNativeTarget section */\n\n"];
   
    return content;
}

#pragma mark - Create Data

/**
 *  Creates a dictionary for a new data.
 *
 *  @return Dictionary.
 */
- (NSMutableDictionary *)createData
{
    NSMutableDictionary *data = [NSMutableDictionary dictionary];
    [data setObject:@"1" forKey:@"archiveVersion"];
    [data setObject:@"1" forKey:@"objectVersion"];
    NSString* rootObjectKey = [[XCKeyBuilder createUnique] build];
    [data setObject:rootObjectKey forKey:@"rootObject"];
    [data setObject:[self objectsWithRootKey:rootObjectKey] forKey:@"objects"];
    return data;
}

/**
 *  Creates the list of objects for the new data.
 *
 *  @param rootKey Root key.
 *
 *  @return List of objects
 */
- (NSMutableDictionary *)objectsWithRootKey:(NSString *)rootKey
{
    // 1. Configuration
    NSMutableDictionary *buildConfiguration = [self objectBuildConfiguration];
    NSString *buildConfigurationKey = [[XCKeyBuilder createUnique] build];
    
    // 2. Configuration list.
    NSArray *buildConfigurations = [NSArray arrayWithObjects:
                                    buildConfigurationKey,
                                    nil];
    NSMutableDictionary *configurationList = [self objectConfigurationListWithConfigurations:buildConfigurations];
    NSString *configurationListKey = [[XCKeyBuilder createUnique] build];
    
    // 3. Group
    NSMutableDictionary *group = [self objectGroup];
    NSString *groupKey = [[XCKeyBuilder createUnique] build];
    
    // 4. Project
    NSMutableDictionary *project = [self objectProjectWithMainGroup:groupKey
                                             buildConfigurationList:configurationListKey];
    
    NSMutableDictionary *objects = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                    project, rootKey,
                                    buildConfiguration, buildConfigurationKey,
                                    configurationList, configurationListKey,
                                    group, groupKey,
                                    nil];
    return objects;
}

- (NSMutableDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
                      buildConfigurationList:(NSString *)buildKey
{
    NSMutableDictionary *project = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                             @"PBXProject", @"isa",
                             @"Xfast 1.0", @"compatibilityVersion",
                             groupKey, @"mainGroup",
                             @"xxx", @"productRefGroup",
                             @"\"\"", @"projectDirPath",
                             @"\"\"", @"projectRoot",
                             buildKey, @"buildConfigurationList",
                             nil];
    return project;
}

- (NSMutableDictionary *)objectBuildConfiguration
{
    NSMutableDictionary *buildSettings = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                   @"NO", @"ALWAYS_SEARCH_USER_PATHS",
                                   @"\"gnu++0x\"", @"CLANG_CXX_LANGUAGE_STANDARD",
                                   @"\"lxbc++\"", @"CLANG_CXX_LIBRARY",
                                   @"10.9", @"MACOSX_DEPLOYMENT_TARGET",
                                   @"macosx", @"SDKROOT",
                                   nil];
    NSMutableDictionary *buildConfiguration = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                        @"XCBuildConfiguration", @"isa",
                                        buildSettings, @"buildSettings",
                                        @"Release", @"name",
                                        nil];
    return buildConfiguration;
}

/**
 *  Creates a default configuration list.
 *
 *  @param keys Build configuration keys.
 *
 *  @return Configuration list.
 */
- (NSMutableDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys
{
    NSMutableDictionary *configurationList = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                              @"XCConfigurationList", @"isa",
                                              //                                       @0, @"defaultConfigurationIsVisible",
                                              @"Release", @"defaultConfigurationName",
                                              keys, @"buildConfigurations",
                                              nil];
    return configurationList;
}

- (NSMutableDictionary *)objectGroup
{
    NSArray *children = [NSArray array];
    NSMutableDictionary *group = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                           @"PBXGroup", @"isa",
                           children, @"children",
                           @"\"<group>\"", @"sourceTree",
                           nil];
    return group;
}

- (NSMutableDictionary *)objectGroupWithKeys:(NSArray *)children
                                    withName:(NSString *)name
{
    NSMutableDictionary *group = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                  @"PBXGroup", @"isa",
                                  children, @"children",
                                  name, @"name",
                                  @"<group>", @"sourceTree", // for saving
                                  nil];
    return group;
}

- (NSMutableDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files
{
    NSMutableDictionary *sourceBuildPhase = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                      @"PBXSourcesBuildPhase", @"isa",
                                      files, @"files",
                                      nil];
    return sourceBuildPhase;
}

- (NSMutableDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases
{
    NSMutableDictionary *target = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                            @"PBXNativeTarget", @"isa",
                            buildKey, @"buildConfigurationList",
                            phases, @"buildPhases",
                            nil];
    return target;
}



#pragma mark - Xfast Files

- (void)addXfastToProject:(NSString *)type withName:(NSString *)name withKey:(NSString *)key
{
    NSDictionary* reference = [self makeXfastFileReferenceWithPath:name
                                                              name:name
                                                         xfastType:type
                                                              type:SourceCodeXfast];
    [[self objects] setObject:reference forKey:key];
}

- (NSDictionary*)makeXfastFileReferenceWithPath:(NSString*)path
                                           name:(NSString*)name
                                      xfastType:(NSString *)xfastType
                                           type:(XcodeSourceFileType)type
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:PBXFileReferenceType] forKey:@"isa"];
    [reference setObject:@"4" forKey:@"fileEncoding"];
    [reference setObject:xfastType forKey:@"xfastType"];
    [reference setObject:NSStringFromXCSourceFileType(type) forKey:@"lastKnownFileType"];
    if (name != nil)
    {
        [reference setObject:[name lastPathComponent] forKey:@"name"];
    }
    if (path != nil)
    {
        [reference setObject:path forKey:@"path"];
    }
    [reference setObject:@"<group>" forKey:@"sourceTree"];
    return reference;
}

#pragma mark Files

- (NSArray*)files
{
    NSMutableArray* results = [NSMutableArray array];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == PBXFileReferenceType)
        {
            XcodeSourceFileType fileType = XCSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);
            NSString* path = [obj valueForKey:@"path"];
            NSString* name = [obj valueForKey:@"name"];
            NSString* sourceTree = [obj valueForKey:@"sourceTree"];
            [results addObject:[XCSourceFile sourceFileWithProject:self
                                                               key:key
                                                              type:fileType
                                                              name:name
                                                        sourceTree:(sourceTree ? sourceTree : @"<group>")
                                                              path:path]];
        }
    }];
    return results;
}

- (XCSourceFile*)fileWithKey:(NSString*)key
{
    NSDictionary* obj = [[self objects] valueForKey:key];
    if (obj &&
        ([[obj valueForKey:@"isa"] asMemberType] == PBXFileReferenceType ||
         [[obj valueForKey:@"isa"] asMemberType] == PBXReferenceProxyType))
    {
        XcodeSourceFileType fileType = XCSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);

        NSString* name = [obj valueForKey:@"name"];
        NSString* sourceTree = [obj valueForKey:@"sourceTree"];

        if (name == nil)
        {
            name = [obj valueForKey:@"path"];
        }
        return [XCSourceFile sourceFileWithProject:self
                                               key:key
                                              type:fileType
                                              name:name
                                        sourceTree:(sourceTree ? sourceTree : @"<group>")
            path:[obj valueForKey:@"path"]];
    }
    return nil;
}

- (XCSourceFile*)fileWithName:(NSString*)name
{
    for (XCSourceFile* projectFile in [self files])
    {
        if ([[projectFile name] isEqualToString:name])
        {
            return projectFile;
        }
    }
    return nil;
}


- (NSArray*)headerFiles
{
    return [self projectFilesOfType:SourceCodeHeader];
}

- (NSArray*)objectiveCFiles
{
    return [self projectFilesOfType:SourceCodeObjC];
}

- (NSArray*)objectiveCPlusPlusFiles
{
    return [self projectFilesOfType:SourceCodeObjCPlusPlus];
}


- (NSArray*)xibFiles
{
    return [self projectFilesOfType:XibFile];
}

- (NSArray*)imagePNGFiles
{
    return [self projectFilesOfType:ImageResourcePNG];
}

// need this value to construct relative path in XcodeprojDefinition
- (NSString*)filePath
{
    return _filePath;
}

#pragma mark Groups

/**
 *  Removes all of the groups and leaves the main group empty.
 */
- (void)removeAllGroup
{
    for (XCGroup *group in [self groups]) {
        [[self objects] removeObjectForKey:[group key]];
    }
//    NSString *mainGroupKey = [self mainGroupKey];
//    NSMutableDictionary *group = [self objectGroup];
//    [[self objects] setObject:group forKey:mainGroupKey];
//    [self save];
}



- (void)printTree
{
    [[self rootGroup] printTree];
    
}

- (NSMutableArray *)asTreeData
{
    NSMutableArray *mainLeavesOfTree = [[self rootGroup] asTreeData];
    NSMutableArray *tree = [NSMutableArray array];
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                 [self name], @"group",
                                 [self rootObjectKey], @"key",
                                 mainLeavesOfTree, @"entries",
                                 [self name], @"path",
                                 nil];
    [tree addObject:item];
    return tree;
}

- (NSArray*)groups
{

    NSMutableArray* results = [[NSMutableArray alloc] init];
    [[_dataStore objectForKey:@"objects"] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {

        if ([[obj valueForKey:@"isa"] asMemberType] == PBXGroupType ||
            [[obj valueForKeyPath:@"isa"] asMemberType] == PBXVariantGroupType)
        {
            [results addObject:[self groupWithKey:key]];
        }
    }];
    return results;
}

//TODO: Optimize this implementation.
- (XCGroup*)rootGroup
{
    for (XCGroup* group in [self groups])
    {
        if ([group isRootGroup])
        {
            return group;
        }
    }
    return nil;
}

- (NSArray*)rootGroups
{
    XCGroup* group = [self rootGroup];
    if (group)
    {
        return [NSArray arrayWithObject:group];
    }

    NSMutableArray* results = [NSMutableArray array];
    for (XCGroup* group in [self groups])
    {
        if ([group parentGroup] == nil)
        {
            [results addObject:group];
        }
    }

    return [results copy];
}

- (XCGroup*)groupWithKey:(NSString*)key
{
    XCGroup* group = [_groups objectForKey:key];
    if (group)
    {
        return group;
    }

    NSDictionary* obj = [[self objects] objectForKey:key];
    if (obj &&
        ([[obj valueForKey:@"isa"] asMemberType] == PBXGroupType ||
         [[obj valueForKey:@"isa"] asMemberType] == PBXVariantGroupType))
    {

        NSString* name = [obj valueForKey:@"name"];
        NSString* path = [obj valueForKey:@"path"];
        NSArray* children = [obj valueForKey:@"children"];
        XCGroup* group = [XCGroup groupWithProject:self key:key alias:name path:path children:children];

        [_groups setObject:group forKey:key];

        return group;
    }
    return nil;
}

- (XCGroup*)groupForGroupMemberWithKey:(NSString*)key
{
    for (XCGroup* group in [self groups])
    {
        if ([group memberWithKey:key])
        {
            return group;
        }
    }
    return nil;
}


- (XCGroup*)groupWithSourceFile:(XCSourceFile*)sourceFile
{
    for (XCGroup* group in [self groups])
    {
        for (id <XcodeGroupMember> member in [group members])
        {
            if ([member isKindOfClass:[XCSourceFile class]] && [[sourceFile key] isEqualToString:[member key]])
            {
                return group;
            }
        }
    }
    return nil;
}

//TODO: This could fail if the path
//attribute on a given group is more than one directory. Start with candidates and
//TODO: search backwards.
- (XCGroup*)groupWithPathFromRoot:(NSString*)path
{
    NSArray* pathItems = [path componentsSeparatedByString:@"/"];
    XCGroup* currentGroup = [self rootGroup];
    for (NSString* pathItem in pathItems)
    {
        id <XcodeGroupMember> group = [currentGroup memberWithDisplayName:pathItem];
        if ([group isKindOfClass:[XCGroup class]])
        {
            currentGroup = group;
        }
        else
        {
            return nil;
        }
    }
    return currentGroup;
}

#pragma mark - SourceBuildPhase

/**
 *  Adds an empty source build phase.
 *
 *  @return SourceBuildPhase key
 */
- (NSString *)addSourceBuildPhase
{
    return nil;
}

#pragma mark - Targets

- (NSString *)name
{
    return [[_filePath lastPathComponent] stringByDeletingPathExtension];
}
/**
 *  Add a targe.
 *
 *  @param aTarget The target.
 *
 *  @return YES if I added it, otherwise NO.
 */
- (BOOL)addTarget:(NSString *)aName
{
    NSString *groupFilePath = [[self name] stringByAppendingPathComponent:@"Products"];
    XCGroup *groupXfast = [self groupWithPathFromRoot:groupFilePath];
    
    XCSourceFileDefinition *sourceDefinition =
    [XCSourceFileDefinition sourceDefinitionWithName:aName
                                                data:nil
                                                type:XcodeTarget];
    [sourceDefinition setFileOperationType:XCFileOperationTypeReferenceOnly];
    [groupXfast addTarget:sourceDefinition];
    
    
    return YES;
}

- (BOOL)addXCTarget:(XCTarget *)aTarget
{
    [self targets];
    [_targets addObject:aTarget];
    return YES;
}

- (NSArray*)targets
{
    if (_targets == nil)
    {
        _targets = [[NSMutableArray alloc] init];
        [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
        {
            if ([[obj valueForKey:@"isa"] asMemberType] == PBXNativeTargetType)
            {
                XCTarget* target =
                    [XCTarget targetWithProject:self
                                            key:key
                                           name:[obj valueForKey:@"name"]
                                    productName:[obj valueForKey:@"productName"]
                               productReference:[obj valueForKey:@"productReference"]];
                [_targets addObject:target];
            }
        }];
    }
    return _targets;
}

- (XCTarget*)targetWithName:(NSString*)name
{
    for (XCTarget* target in [self targets])
    {
        if ([[target name] isEqualToString:name])
        {
            return target;
        }
    }
    return nil;
}


/**
 *  Performs file operations and then writes the project file.
 */
- (void)save
{
    [_fileOperationQueue commitFileOperations];
    [self saveOnlyProjectFile];
    NSLog(@"Saved project");
}


/**
 *  Writes the project file.
 */
- (void)saveOnlyProjectFile
{
    [[_dataStore description] writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]
                               atomically:YES
                                 encoding:NSUTF8StringEncoding error:nil];
//    [self writeWithDictionary:_dataStore
//                       ToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]];
//    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"] atomically:YES];
}


- (NSMutableDictionary*)objects
{
    return [_dataStore objectForKey:@"objects"];
}

- (NSMutableDictionary*)dataStore
{
    return _dataStore;
}

- (void)dropCache
{
    _targets = nil;
    _configurations = nil;
    _rootObjectKey = nil;
}

- (void)setObject:(id)anObject
{
    [self setObject:anObject forKey:[anObject key]];
}

- (void)setObject:(id)anObject forKey:(id<NSCopying>)aKey
{
    [[self objects] setObject:[anObject asDictionary] forKey:aKey];
}

#pragma mark - Configuration List

/**
 *  Adds a new configuration list.
 *
 *  @return YES
 */
- (BOOL)addConfigurationList
{
    NSString *key = [[XCKeyBuilder createUnique] build];
    XCConfigurationList *list = [[XCConfigurationList alloc] initWithProject:self
                                                                         key:key];
    [[self objects] setObject:[list asDictionary]
                       forKey:[list key]];
    return YES;
}

#pragma mark - Configurations



- (NSDictionary*)configurations
{
    if (_configurations == nil)
    {
        NSString* buildConfigurationRootSectionKey =
            [[[self objects] objectForKey:[self rootObjectKey]] objectForKey:@"buildConfigurationList"];
        NSDictionary* buildConfigurationDictionary = [[self objects] objectForKey:buildConfigurationRootSectionKey];
        _configurations =
            [[XCBuildConfiguration buildConfigurationsFromArray:[buildConfigurationDictionary objectForKey:@"buildConfigurations"]
                inProject:self] mutableCopy];
        _defaultConfigurationName = [[buildConfigurationDictionary objectForKey:@"defaultConfigurationName"] copy];
    }

    return [_configurations copy];
}

- (NSDictionary*)configurationWithName:(NSString*)name
{
    return [[self configurations] objectForKey:name];
}

- (XCBuildConfiguration*)defaultConfiguration
{
    return [[self configurations] objectForKey:_defaultConfigurationName];
}

/**
 *  Adds a new build configuration.
 *
 *  @param aBuildConfiguration The build configuration to add.
 *
 *  @return YES if I added the build configuration, otherwise NO.
 */
- (BOOL)addBuildConfiguration:(XCBuildConfiguration *)aBuildConfiguration
{
    [[self objects] setObject:[aBuildConfiguration asDictionary]
                       forKey:[aBuildConfiguration key]];
    NSString* buildConfigurationRootSectionKey =
    [[[self objects] objectForKey:[self rootObjectKey]] objectForKey:@"buildConfigurationList"];
    
    NSDictionary* buildConfigurationDictionary = [[self objects] objectForKey:buildConfigurationRootSectionKey];
    NSMutableArray *buildConfigurationKeys = [buildConfigurationDictionary objectForKey:@"buildConfigurations"];
    [buildConfigurationKeys addObject:[aBuildConfiguration key]];
    
    return YES;
}

#pragma mark Private

- (NSString*)rootObjectKey
{
    if (_rootObjectKey == nil)
    {
        _rootObjectKey = [[_dataStore objectForKey:@"rootObject"] copy];;
    }

    return _rootObjectKey;
}

- (NSString *)mainGroupKey
{
    NSString *rootKey = [self rootObjectKey];
    NSMutableDictionary *rootObject = [[self objects] objectForKey:rootKey];
    return [rootObject objectForKey:@"mainGroup"];
}

- (NSArray*)projectFilesOfType:(XcodeSourceFileType)projectFileType
{
    NSMutableArray* results = [NSMutableArray array];
    for (XCSourceFile* file in [self files])
    {
        if ([file type] == projectFileType)
        {
            [results addObject:file];
        }
    }
    return results;
}

@end