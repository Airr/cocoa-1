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


#import "XFProject.h"
#import "XFGroup.h"
#import "XFSourceFile.h"
#import "XFTarget.h"
#import "XFFileOperationQueue.h"
#import "XFBuildConfiguration.h"
#import "XFKeyBuilder.h"
#import "XcodeMemberType.h"

@interface XFProject ()

@end

@implementation XFProject


@synthesize fileOperationQueue = _fileOperationQueue;

#pragma mark - Class Methods

/**
 *  Creates an Xfast file with a template file.
 *
 *  @param filePath     The file path of an Xfast file.
 *  @param templatePath The plist file with a build setting.
 *
 *  @return XFProject
 */
+ (XFProject*)projectWithNewFilePath:(NSString*)filePath
                        withTemplate:(NSString *)templatePath
{
    return [[XFProject alloc] initWithFilePath:filePath create:YES withTemplate:templatePath];
}

// Not tested.
+ (XFProject*)projectWithNewFilePath:(NSString*)filePath
{
    return [[XFProject alloc] initWithFilePath:filePath create:YES withTemplate:nil];
}

/**
 *  Reads in an Xfast file.
 *
 *  @param filePath The file path to an Xfast file.
 *
 *  @return The Xfast project.
 */
+ (XFProject*)projectWithFilePath:(NSString*)filePath
{
    return [[XFProject alloc] initWithFilePath:filePath];
}

// Not tested.
+ (XFProject*)projectWithNewFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    return [[XFProject alloc] initWithNewFilePath:filePath baseDirectory:dirPath];
}

// Not tested.
+ (XFProject*)projectWithFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    return [[XFProject alloc] initWithFilePath:filePath baseDirectory:dirPath];
}

#pragma mark - Initialization & Destruction

- (id)initWithFilePath:(NSString*)filePath create:(BOOL)needToCreate withTemplate:(NSString *)templatePath
{
    self = [super init];
    if (self)
    {
        NSFileManager* fileManager = [NSFileManager defaultManager];
        // 1. Check folder
        NSString *basePath = [filePath stringByDeletingLastPathComponent];
        if ([fileManager fileExistsAtPath:basePath] == NO) {
            NSLog(@"should return nil;");
            return nil;
        }
        
        // 3. Set object members
        _filePath = [filePath copy];
        
        if (needToCreate) {
            _dataStore = [self createDataWithTemplate:templatePath];
        } else {
            _dataStore = [[NSMutableDictionary alloc]
                          initWithContentsOfFile:_filePath];
            
            if (!_dataStore)
            {
                [NSException raise:XFProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
            }
        }
        _fileOperationQueue = [[XFFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];
        
    }
    return self;
}

- (id)initWithFilePath:(NSString*)filePath
{
    return [self initWithFilePath:filePath create:NO withTemplate:nil];
}

- (id)initWithNewFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    if ((self = [super init]))
    {
        NSFileManager* fileManager = [NSFileManager defaultManager];
        if (![fileManager createDirectoryAtPath:filePath withIntermediateDirectories:YES attributes:nil error:nil])
        {
//            [NSException raise:NSInvalidArgumentException format:@"Error: Create folder failed %@", filePath];
        }
        
        NSString *projectName = [filePath lastPathComponent];
        NSString *projectMainPath = [filePath stringByAppendingPathComponent:projectName];
        NSString *projectPath = [projectMainPath stringByAppendingPathExtension:@"xfast"];
        [fileManager createDirectoryAtPath:projectMainPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        [fileManager createDirectoryAtPath:projectPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        NSString *xfastMainPath = [dirPath stringByAppendingPathComponent:projectName];
        [fileManager createDirectoryAtPath:xfastMainPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        
        NSMutableDictionary *data = [NSMutableDictionary dictionary];
        [data setObject:@"1" forKey:@"archiveVersion"];
        [data setObject:@"46" forKey:@"objectVersion"];
        NSString* rootObjectKey = [[XFKeyBuilder forItemNamed:projectPath] build];
        [data setObject:rootObjectKey forKey:@"rootObject"];
        [data setObject:[self objectsWithRootKey:rootObjectKey withTemplate:nil]
                 forKey:@"objects"];
        
        
        
        NSString *projectFilePath = [projectPath stringByAppendingPathComponent:@"project.pbxproj"];
        [self writeWithDictionary:data ToFile:projectFilePath];

        
        _filePath = [projectPath copy];
        _dataStore = [[NSMutableDictionary alloc]
                      initWithContentsOfFile:projectFilePath];
        
        
        if (!_dataStore)
        {
            [NSException raise:XFProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
        }

//        _fileOperationQueue = [[XFFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];
        _fileOperationQueue = [[XFFileOperationQueue alloc] initWithBaseDirectory:dirPath];

    }
    return self;
}

- (id)initWithFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    if ((self = [super init]))
    {
        _filePath = [filePath copy];
        _dataStore = [[NSMutableDictionary alloc]
                      initWithContentsOfFile:_filePath];
//                      [_filePath stringByAppendingPathComponent:@"project.pbxproj"]];
        if (!_dataStore)
        {
            [NSException raise:XFProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
        }

        _fileOperationQueue = [[XFFileOperationQueue alloc]
                               initWithBaseDirectory:dirPath];
//                               [_filePath stringByDeletingLastPathComponent]];

    }
    return self;
}

#pragma mark - Interface Methods
#pragma mark Streams

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
- (NSMutableDictionary *)createDataWithTemplate:(NSString *)templatePath
{
    NSMutableDictionary *data = [NSMutableDictionary dictionary];
    [data setObject:@"1" forKey:@"archiveVersion"];
    [data setObject:@"1" forKey:@"objectVersion"];
    NSString* rootObjectKey = [[XFKeyBuilder createUnique] build];
    [data setObject:rootObjectKey forKey:@"rootObject"];
    [data setObject:[self objectsWithRootKey:rootObjectKey withTemplate:templatePath]
             forKey:@"objects"];
    return data;
}

/**
 *  Creates the list of objects for the new data.
 *
 *  @param rootKey Root key.
 *
 *  @return List of objects
 */
- (NSMutableDictionary *)objectsWithRootKey:(NSString *)rootKey withTemplate:(NSString *)templatePath
{
    // 1. Configuration
    NSMutableDictionary *buildConfiguration = [self objectBuildConfigurationWithTemplate:templatePath];
    NSString *buildConfigurationKey = [[XFKeyBuilder createUnique] build];
    
    // 2. Configuration list.
    NSArray *buildConfigurations = [NSArray arrayWithObjects:
                                    buildConfigurationKey,
                                    nil];
    NSMutableDictionary *configurationList = [self objectConfigurationListWithConfigurations:buildConfigurations];
    NSString *configurationListKey = [[XFKeyBuilder createUnique] build];
    
    // 3. Group
    NSMutableDictionary *group = [self objectGroup];
    NSString *groupKey = [[XFKeyBuilder createUnique] build];
    
    // 4. Project
    NSMutableDictionary *project = [self objectProjectWithMainGroup:groupKey
                                             buildConfigurationList:configurationListKey
                                                       withTemplate:templatePath];
    
    NSMutableDictionary *objects = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                    project, rootKey,
                                    buildConfiguration, buildConfigurationKey,
                                    configurationList, configurationListKey,
                                    group, groupKey,
                                    nil];
    return objects;
}

/**
 *  Creates the project item for a XFProject.
 *
 *  @param groupKey     The main group key
 *  @param buildKey     The build key
 *  @param templatePath The xfast template file
 *
 *  @return The project item
 */
- (NSMutableDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
                             buildConfigurationList:(NSString *)buildKey
                                       withTemplate:(NSString *)templatePath
{
    NSMutableDictionary *templateDictionary = [NSMutableDictionary dictionaryWithContentsOfFile:templatePath];
    NSMutableDictionary *project = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                    @"XFPBXProject", @"isa",
                                    @"Xfast 1.0", @"compatibilityVersion",
                                    groupKey, @"mainGroup",
                                    buildKey, @"buildConfigurationList",
                                    [templateDictionary objectForKey:@"name"], @"type",
                                    [templateDictionary objectForKey:@"operation"], @"operation",
                                    [templateDictionary objectForKey:@"description"], @"description",
                                    [templateDictionary objectForKey:@"infiles"], @"infiles",
                                    [templateDictionary objectForKey:@"outfiles"], @"outfiles",
                                    @"\"\"", @"projectDirPath",
                                    @"\"\"", @"projectRoot",
                                    nil];
    return project;
}

- (NSMutableDictionary *)objectBuildConfigurationWithTemplate:(NSString *)templatePath
{
    
    NSMutableDictionary *buildSettings = [NSMutableDictionary dictionary];
    NSDictionary *templateDictionary = [NSDictionary dictionaryWithContentsOfFile:templatePath];
    NSArray *templateSettings = [templateDictionary objectForKey:@"buildSettings"];
    [templateSettings enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [buildSettings setObject:[obj objectForKey:@"default"] forKey:[obj objectForKey:@"name"]];
    }];
    
//    NSMutableDictionary *buildSettings = [NSMutableDictionary dictionaryWithObjectsAndKeys:
//                                          @"NO", @"ALWAYS_SEARCH_USER_PATHS",
//                                          @"\"gnu++0x\"", @"CLANG_CXX_LANGUAGE_STANDARD",
//                                          @"\"lxbc++\"", @"CLANG_CXX_LIBRARY",
//                                          @"10.9", @"MACOSX_DEPLOYMENT_TARGET",
//                                          @"macosx", @"SDKROOT",
//                                          nil];
    NSMutableDictionary *buildConfiguration = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                               @"XFBuildConfiguration", @"isa",
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
                                              @"XFConfigurationList", @"isa",
                                              @"Release", @"defaultConfigurationName",
                                              keys, @"buildConfigurations",
                                              nil];
    return configurationList;
}

- (NSMutableDictionary *)objectGroup
{
    NSArray *children = [NSArray array];
    NSMutableDictionary *group = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                  @"XFPBXGroup", @"isa",
                                  children, @"children",
                                  @"<group>", @"sourceTree",
                                  nil];
    return group;
}

- (NSMutableDictionary *)objectGroupWithKeys:(NSArray *)children
                                    withName:(NSString *)name
{
    NSMutableDictionary *group = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                  @"XFPBXGroup", @"isa",
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







#pragma mark Properties

- (NSString *)name
{
    return [[[self filePath] lastPathComponent] stringByDeletingPathExtension];
}

#pragma mark - File Reference

/**
 *  Adds a file reference with the path of Xfast. The Xfast path is in the 
 * main Xfast project. Each Xfast file name is unique. The base path is given
 * by Xfast-project-name/Xfast-filename.
 *
 *  @param type The file type.
 *  @param path The path from the Xfast.
 */
- (void)addFileOfType:(NSString *)type withXfastPath:(NSString *)path
{
    [self addFileOfType:type withXfastPath:path withKey:nil];
}

/**
 *  Adds a file referenece with the path of data. The data path is given by
 * database/xfast-project-name/xfast-file-name/.
 *
 *  @param type The file type.
 *  @param path The path from the
 */
- (void)addFileOfType:(NSString *)type withDataPath:(NSString *)path
{
    [self addFileOfType:type withXfastPath:path withKey:nil];
}

/**
 *  Adds a file reference with an absolute path.
 *
 *  @param type The file type.
 *  @param path The path to the file.
 */
- (void)addFileOfType:(NSString *)type withAbsolutePath:(NSString *)path
{
    [self addFileOfType:type withAbsolutePath:path withKey:nil];
}

- (void)addFileOfType:(NSString *)type withXfastPath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeFileReferenceWithXfastPath:path
                                                                     name:path
                                                                     type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}

- (void)addFileOfType:(NSString *)type withDataPath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeFileReferenceWithDataPath:path
                                                                    name:path
                                                                    type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}

- (void)addFileOfType:(NSString *)type withAbsolutePath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeFileReferenceWithAbsolutePath:path
                                                                        name:path
                                                                        type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}


- (NSMutableDictionary*)makeFileReferenceWithDataPath:(NSString*)path
                                                 name:(NSString*)name
                                                 type:(NSString*)type
{
    return [self makeFileReferenceWithPath:path name:name type:type sourceTree:@"<data>"];
}

- (NSMutableDictionary*)makeFileReferenceWithXfastPath:(NSString*)path
                                                  name:(NSString*)name
                                                  type:(NSString*)type
{
    return [self makeFileReferenceWithPath:path name:name type:type sourceTree:@"<xfast>"];
}


- (NSMutableDictionary*)makeFileReferenceWithAbsolutePath:(NSString*)path
                                                     name:(NSString*)name
                                                     type:(NSString*)type
{
    return [self makeFileReferenceWithPath:path name:name type:type sourceTree:@"<absolute>"];
}

- (NSMutableDictionary*)makeFileReferenceWithPath:(NSString*)path
                                             name:(NSString*)name
                                             type:(NSString*)type
                                       sourceTree:(NSString*)sourceTree
{
    return [self makeFileReferenceWithPath:path name:name type:type sourceTree:sourceTree memberType:XFPBXFileReferenceType];
    
}

/**
 *  Adds the output file reference. The base path is given by
 * Xfast-project/Xfast-file/Xfast-target/out.
 *
 *  @param type The file type.
 *  @param path The path from the base path.
 */
- (void)addOutFileOfType:(NSString *)type withXfastPath:(NSString *)path
{
    [self addOutFileOfType:type withXfastPath:path withKey:nil];
}

/**
 *  Adds the output file referenece to the database. The base path is given by
 * database/Xfast-project/Xfast-file/Xfast-target/out
 *
 *  @param type The file type.
 *  @param path The path from the base path.
 */
- (void)addOutFileOfType:(NSString *)type withDataPath:(NSString *)path
{
    [self addOutFileOfType:type withXfastPath:path withKey:nil];
}

/**
 *  Adds the output file reference with an absolute path.
 *
 *  @param type The file type.
 *  @param path The path to the file.
 */
- (void)addOutFileOfType:(NSString *)type withAbsolutePath:(NSString *)path
{
    [self addOutFileOfType:type withAbsolutePath:path withKey:nil];
}


- (void)addOutFileOfType:(NSString *)type withXfastPath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeOutFileReferenceWithXfastPath:path
                                                                     name:path
                                                                     type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}

- (void)addOutFileOfType:(NSString *)type withDataPath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeOutFileReferenceWithDataPath:path
                                                                    name:path
                                                                    type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}

- (void)addOutFileOfType:(NSString *)type withAbsolutePath:(NSString *)path withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeOutFileReferenceWithAbsolutePath:path
                                                                        name:path
                                                                        type:type];
    if (key) {
        [[self objects] setObject:reference forKey:key];
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
    }
}


- (NSMutableDictionary*)makeOutFileReferenceWithDataPath:(NSString*)path
                                                 name:(NSString*)name
                                                 type:(NSString*)type
{
    return [self makeOutFileReferenceWithPath:path name:name type:type sourceTree:@"<data>"];
}

- (NSMutableDictionary*)makeOutFileReferenceWithXfastPath:(NSString*)path
                                                  name:(NSString*)name
                                                  type:(NSString*)type
{
    return [self makeOutFileReferenceWithPath:path name:name type:type sourceTree:@"<xfast>"];
}


- (NSMutableDictionary*)makeOutFileReferenceWithAbsolutePath:(NSString*)path
                                                     name:(NSString*)name
                                                     type:(NSString*)type
{
    return [self makeOutFileReferenceWithPath:path name:name type:type sourceTree:@"<absolute>"];
}

- (NSMutableDictionary*)makeOutFileReferenceWithPath:(NSString*)path
                                                name:(NSString*)name
                                                type:(NSString*)type
                                          sourceTree:(NSString*)sourceTree
{
    return [self makeFileReferenceWithPath:path name:name type:type sourceTree:sourceTree memberType:XFPBXOutFileReferenceType];
    
}

- (NSMutableDictionary*)makeFileReferenceWithPath:(NSString*)path
                                             name:(NSString*)name
                                             type:(NSString*)type
                                       sourceTree:(NSString*)sourceTree
                                       memberType:(XcodeMemberType)memberType
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:memberType] forKey:@"isa"];
    [reference setObject:@"4" forKey:@"fileEncoding"];
    [reference setObject:type forKey:@"lastKnownFileType"];
    if (name != nil)
    {
        [reference setObject:[name lastPathComponent] forKey:@"name"];
    }
    if (path != nil)
    {
        [reference setObject:path forKey:@"path"];
    }
    [reference setObject:sourceTree forKey:@"sourceTree"];
    return reference;
}


- (NSString *)addFileOfType:(NSString *)type
                   withPath:(NSString *)path
                   withUser:(NSString *)user
                withProject:(NSString *)project
                  withXfast:(NSString *)xfast
                 withSource:(NSString *)source
             withMemberType:(XcodeMemberType)member
                    withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeFileReferenceOfType:type
                                                          withPath:path
                                                          withUser:user
                                                       withProject:project
                                                         withXfast:xfast
                                                        withSource:source
                                                    withMemberType:member];
    
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return key;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}

- (NSMutableDictionary*)makeFileReferenceOfType:(NSString *)type
                                       withPath:(NSString *)path
                                       withUser:(NSString *)user
                                    withProject:(NSString *)project
                                      withXfast:(NSString *)xfast
                                     withSource:(NSString *)source
                                 withMemberType:(XcodeMemberType)member
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:member] forKey:@"isa"];
    
    [reference setObject:@"4" forKey:@"fileEncoding"];
    [reference setObject:type forKey:@"lastKnownFileType"];
    [reference setObject:type forKey:@"type"];
    [reference setObject:path forKey:@"path"];
    [reference setObject:user forKey:@"user"];
    [reference setObject:project forKey:@"project"];
    [reference setObject:xfast forKey:@"xfast"];
    [reference setObject:source forKey:@"sourceTree"];
    [reference setObject:source forKey:@"source"];
    return reference;
}

#pragma mark - Target Reference

- (BOOL)removeTarget:(NSString *)aName
{
    // 1. SourceBuild
    // 2. OutputBuild
    // 3. BuildConfiguration
    // 4. Configuration
    // 5. Target
    XFTarget *target = [self targetWithName:aName];
    [target removeAll];
    [self dropCache];
    
    return YES;
}

/**
 *  Creates the output build phase with an output file.
 *
 *  @return The key to output build phase
 */
- (NSString *)addOutputBuildPhaseWithOperationOfManyToOne:(NSString *)targetName
{
    // Creates an output file reference.
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    NSDictionary *outfiles = [obj objectForKey:@"outfiles"];
    NSDictionary *mainOutfile = [outfiles objectForKey:@"MAIN_OUTFILE"];
    
    NSString *path = [NSString stringWithFormat:@"%@/out/%@",
                      targetName,
                      [mainOutfile objectForKey:@"filename"]];
    NSString *key = [self addFileOfType:[mainOutfile objectForKey:@"filetype"]
                               withPath:path
                               withUser:[self userName]
                            withProject:[self xcprojectFilename]
                              withXfast:[self name]
                             withSource:@"<data>"
                         withMemberType:XFPBXOutFileReferenceType
                                withKey:nil];
    
    // Creates an build file reference.
    NSMutableDictionary* buildFile = [NSMutableDictionary dictionary];
    [buildFile setObject:[NSString stringFromMemberType:XFPBXBuildFileType] forKey:@"isa"];
    [buildFile setObject:key forKey:@"fileRef"];
    NSString *buildFileKey = [[XFKeyBuilder createUnique] build];
    [[self objects] setObject:buildFile forKey:buildFileKey];
    
    // Creates an output build phase.
    NSMutableArray *files = [NSMutableArray arrayWithObjects:buildFileKey, nil];
    return [self addOutputBuildPhaseWithFiles:files withKey:nil];
}

- (NSString *)addOutputBuildPhaseWithOperationOfManyToDirectory
{
    return nil;
}

- (NSString *)addOutputBuildPhaseWithOperationOfManyToMany
{
    return nil;
}

- (NSString *)addOutputBuildPhaseWithOperationOfManyToMultiple
{
    return nil;
}


/**
 *  Adds a new target to the Xfast project.
 *
 *  @param aName The name of a target.
 *
 *  @return YES
 */
- (BOOL)addTarget:(NSString *)aName
{
    // 1. SourceBuild
    // 2. OutputBuild
    // 3. BuildConfiguration
    // 4. Configuration
    // 5. Target
    
    NSString *operation = [self operation];
    if ([operation compare:@"ManyToOne"] == NSOrderedSame) {
        //
        [self addTargetOperationOfManyToOne:aName];
    } else {
        assert(0);
    }
    
//    NSString *sourceKey = [self addSourceBuildPhaseWithEmptyFiles];
//    NSString *operation = [self operation];
//    NSString *addOutputBuildPhaseWithOperation = [NSString stringWithFormat:@"addOutputBuildPhaseWithOperationOf%@", operation];
//    SEL s = NSSelectorFromString(addOutputBuildPhaseWithOperation);
//    if ([self respondsToSelector:s]) {
//        //
//    } else {
//        assert(0);
//    }
//    NSString *outputKey = [self performSelector:s];
//    assert(outputKey != nil);
//    NSString *outputKey = [self addOutputBuildPhaseWithEmptyFiles];

    return YES;
}

/**
 *  Adds a target with the ManyToOne operation.
 *
 *  @param aName The target name.
 *
 *  @return YES or NO.
 */
- (BOOL)addTargetOperationOfManyToOne:(NSString *)aName
{
    // 1. SourceBuild
    // 2. OutputBuild
    // 3. BuildConfiguration
    // 4. Configuration
    // 5. Target
    
    NSString *sourceKey = [self addSourceBuildPhaseWithEmptyFiles];

    NSString *outputKey = [self addOutputBuildPhaseWithOperationOfManyToOne:aName];
    
    NSMutableArray *buildPhaseKeys = [NSMutableArray arrayWithObjects:
                                      sourceKey, outputKey,
                                      nil];
    NSString *buildKey = [self addBuildConfiguration];
    NSMutableArray *buildKeys = [NSMutableArray arrayWithObjects:buildKey, nil];
    NSString *confListKey = [self addConfigurationList:buildKeys];
    
    [self addTargetReference:aName withConfList:confListKey withBuildPhase:buildPhaseKeys];

    [self dropCache];
    [self save];
    return YES;
}


- (NSString *)addTargetReference:(NSString *)name
                    withConfList:(NSString *)confList
                  withBuildPhase:(NSMutableArray *)buildPhase
{
    return [self addTargetReference:name withConfList:confList withBuildPhase:buildPhase withKey:nil];
}


- (NSString *)addTargetReference:(NSString *)name
                    withConfList:(NSString *)confList
                  withBuildPhase:(NSMutableArray *)buildPhase
                         withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeTarget:name
                                         withConfList:confList
                                       withBuildPhase:buildPhase];
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return nil;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        // This saves the target in the object list.
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}



- (NSMutableDictionary *)makeTarget:(NSString *)name
                       withConfList:(NSString *)confList
                     withBuildPhase:(NSMutableArray *)buildPhase
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFPBXNativeTargetType]
                  forKey:@"isa"];
    [reference setObject:name forKey:@"name"];
    [reference setObject:confList forKey:@"buildConfigurationList"];
    [reference setObject:buildPhase forKey:@"buildPhases"];
    return reference;
}

- (NSString *)addConfigurationList:(NSMutableArray *)buildConf
{
    return [self addConfigurationList:buildConf withKey:nil];
}

- (NSString *)addConfigurationList:(NSMutableArray *)buildConf
                            withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeConfigurationList:buildConf];
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return nil;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}


- (NSMutableDictionary *)makeConfigurationList:(NSMutableArray *)buildConf
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFConfigurationListType]
                  forKey:@"isa"];
    [reference setObject:buildConf forKey:@"buildConfigurations"];
    [reference setObject:@"Release" forKey:@"defaultConfigurationName"];
    return reference;
}

- (NSString *)addBuildConfiguration
{
    return [self addBuildConfigurationWithEmptyBuildSettingsWithName:@"Release"
                                                             withKey:nil];
}


- (NSString *)addBuildConfigurationWithEmptyBuildSettingsWithName:(NSString *)name
                                                          withKey:(NSString *)key
{
    NSMutableDictionary *buildSettings = [[self defaultBuildsettings] copy];
    return [self addBuildConfiguration:buildSettings withName:name withKey:key];
}

- (NSString *)addBuildConfiguration:(NSMutableDictionary *)buildSettings
                           withName:(NSString *)name
                            withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeBuildConfiguration:buildSettings
                                                         withName:name];
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return nil;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}

- (NSMutableDictionary *)makeBuildConfiguration:(NSMutableDictionary *)buildSettings
                                       withName:(NSString *)name
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFBuildConfigurationType]
                  forKey:@"isa"];
    [reference setObject:buildSettings forKey:@"buildSettings"];
    [reference setObject:name forKey:@"name"];
    return reference;
}


- (NSString *)addSourceBuildPhaseWithEmptyFiles
{
    NSMutableArray *files = [NSMutableArray array];
    return [self addSourceBuildPhaseWithFiles:files withKey:nil];
}

- (NSString *)addSourceBuildPhaseWithFiles:(NSArray *)files
{
    return [self addSourceBuildPhaseWithFiles:files withKey:nil];
}

- (NSString *)addSourceBuildPhaseWithFiles:(NSArray *)files withKey:(NSString *)key
{
    NSMutableDictionary* reference = [self makeSourcesBuildPhaseReferenceWithPath:files];
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return nil;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}

- (NSString *)addOutputBuildPhaseWithEmptyFiles
{
    NSMutableArray *files = [NSMutableArray array];
    return [self addOutputBuildPhaseWithFiles:files withKey:nil];
}

- (NSString *)addOutputBuildPhaseWithFiles:(NSArray *)files
{
    return [self addOutputBuildPhaseWithFiles:files withKey:nil];
}

- (NSString *)addOutputBuildPhaseWithFiles:(NSArray *)files withKey:(NSString *)key
{
    NSDictionary* reference = [self makeOutputBuildPhaseReferenceWithPath:files];
    if (key) {
        [[self objects] setObject:reference forKey:key];
        return nil;
    } else {
        NSString *uniqueKey = [[XFKeyBuilder createUnique] build];
        [[self objects] setObject:reference forKey:uniqueKey];
        return uniqueKey;
    }
}

- (NSMutableDictionary*)makeSourcesBuildPhaseReferenceWithPath:(NSArray *)files
{
    return [self makeOutputBuildPhaseReferenceWithPath:files
                                   withXcodeMemberType:XFPBXSourcesBuildPhaseType];
}

- (NSMutableDictionary*)makeOutputBuildPhaseReferenceWithPath:(NSArray *)files
{
    return [self makeOutputBuildPhaseReferenceWithPath:files
                                   withXcodeMemberType:XFPBXOutputBuildPhaseType];
}

- (NSMutableDictionary*)makeOutputBuildPhaseReferenceWithPath:(NSArray *)files
                                   withXcodeMemberType:(XcodeMemberType)type
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:type]
                  forKey:@"isa"];
    [reference setObject:files forKey:@"files"];
    return reference;
}

#pragma mark Files

/**
 *  Returns all of the file references.
 *
 *  @return The array of files.
 */
- (NSArray*)files
{
    NSMutableArray* results = [NSMutableArray array];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXFileReferenceType)
        {
            XfastSourceFileType fileType = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);
            NSString* path = [obj valueForKey:@"path"];
            NSString* name = [obj valueForKey:@"name"];
            NSString* sourceTree = [obj valueForKey:@"sourceTree"];
            
            NSString* source = [obj valueForKey:@"source"];
            NSString* user = [obj valueForKey:@"user"];
            NSString* project = [obj valueForKey:@"project"];
            NSString* xfast = [obj valueForKey:@"xfast"];
            NSString* type = [obj valueForKey:@"type"];
            [results addObject:[XFSourceFile sourceFileWithProject:self
                                                               key:key
                                                        sourceType:fileType
                                                              name:name
                                                        sourceTree:source
                                                              path:path
                                                              user:user
                                                           project:project
                                                             xfast:xfast
                                                              type:type]];
//            [results addObject:[XFSourceFile
//                                sourceFileWithProject:self
//                                key:key
//                                type:fileType
//                                name:name
//                                sourceTree:(sourceTree ? sourceTree : @"<group>")
//                                path:path]];
        }
    }];
    return results;
}

/**
 *  Returns the array of output file references.
 *
 *  @return The array of output files.
 */
- (NSArray*)outfiles
{
    NSMutableArray* results = [NSMutableArray array];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXOutFileReferenceType)
        {
            XfastSourceFileType fileType = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);
            NSString* path = [obj valueForKey:@"path"];
            NSString* name = [obj valueForKey:@"name"];
            NSString* sourceTree = [obj valueForKey:@"sourceTree"];
            
            NSString* source = [obj valueForKey:@"source"];
            NSString* user = [obj valueForKey:@"user"];
            NSString* project = [obj valueForKey:@"project"];
            NSString* xfast = [obj valueForKey:@"xfast"];
            NSString* type = [obj valueForKey:@"type"];
            [results addObject:[XFSourceFile sourceFileWithProject:self
                                                               key:key
                                                        sourceType:fileType
                                                              name:name
                                                        sourceTree:source
                                                              path:path
                                                              user:user
                                                           project:project
                                                             xfast:xfast
                                                              type:type]];
        }
    }];
    return results;
}

/**
 *  Returns the file referenced.
 *
 *  @param key The file key.
 *
 *  @return The file referenced.
 */
- (XFSourceFile*)fileWithKey:(NSString*)key
{
    NSDictionary* obj = [[self objects] valueForKey:key];
    if (obj &&
        ([[obj valueForKey:@"isa"] asMemberType] == XFPBXFileReferenceType
         || [[obj valueForKey:@"isa"] asMemberType] == XFPBXOutFileReferenceType
         || [[obj valueForKey:@"isa"] asMemberType] == XFPBXReferenceProxyType))
    {
        XfastSourceFileType fileType = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);

        NSString* name = [obj valueForKey:@"name"];
//        NSString* sourceTree = [obj valueForKey:@"sourceTree"];
        
        NSString* path = [obj valueForKey:@"path"];
        NSString* source = [obj valueForKey:@"source"];
        NSString* user = [obj valueForKey:@"user"];
        NSString* project = [obj valueForKey:@"project"];
        NSString* xfast = [obj valueForKey:@"xfast"];
        NSString* type = [obj valueForKey:@"type"];
        if (name == nil)
        {
            name = [obj valueForKey:@"path"];
        }
        
        return [XFSourceFile sourceFileWithProject:self
                                               key:key
                                        sourceType:fileType
                                              name:name
                                        sourceTree:source
                                              path:path
                                              user:user
                                           project:project
                                             xfast:xfast
                                              type:type];
//        
//        
//        return [XFSourceFile sourceFileWithProject:self
//                                               key:key
//                                              type:fileType
//                                              name:name
//                                        sourceTree:(sourceTree ? sourceTree : @"<group>")
//                                              path:[obj valueForKey:@"path"]];
    }
    return nil;
}

- (XFSourceFile*)fileWithName:(NSString*)name
{
    for (XFSourceFile* projectFile in [self files])
    {
        if ([[projectFile name] isEqualToString:name])
        {
            return projectFile;
        }
    }
    return nil;
}

- (XFSourceFile*)fileWithPath:(NSString*)path
{
    for (XFSourceFile* projectFile in [self files])
    {
        if ([[projectFile absolutePath] isEqualToString:path])
        {
            return projectFile;
        }
    }
    return nil;
}


/**
 *  Returns the path to the file.
 *
 *  @param path    The path from the base.
 *  @param user    The user name.
 *  @param project The project name.
 *  @param xfast   The xfast name.
 *  @param source  The source.
 *
 *  @return The absolute path to the file.
 *
 *  Target7/out is missing: path must be Target7/out/file.cat
 */
- (NSString *)pathToFile:(NSString *)path
                withUser:(NSString *)user
             withProject:(NSString *)project
               withXfast:(NSString *)xfast
              withSource:(NSString *)source
{
    if (source) {
        if ([source compare:@"<data>"] == NSOrderedSame) {
            NSString *pathToFile = [NSString stringWithFormat:@"%@/%@/%@/%@/%@",
                                    [self databaseDirectory],
                                    user,
                                    project,
                                    xfast,
                                    path];
            return pathToFile;
        } else if ([source compare:@"<xfast>"] == NSOrderedSame) {
            NSString *pathToFile = [NSString stringWithFormat:@"%@/%@/%@/%@",
                                    [self projectDirectory],
                                    project,
                                    xfast,
                                    path];
            return pathToFile;
        } else if ([source compare:@"<absolute>"] == NSOrderedSame) {
            return path;
        } else {
            return nil;
        }
    } else {
        if (path) {
            return path;
        } else {
            return nil;
        }
    }
}


/**
 *  Returns an input file in the database given by
 * database/user/project/xfast/in/path
 *
 *  The path can be out/path. Output files can be input files to other targets.
 * In that case, we use out/path.
 *
 *  @param path    The path to the file.
 *  @param user    The user name.
 *  @param project The project name.
 *  @param xfast   The xfast name.
 *
 *  @return The source file.
 */
- (XFSourceFile *)dataFileWithPath:(NSString *)path
                          withUser:(NSString *)user
                       withProject:(NSString *)project
                         withXfast:(NSString *)xfast
{
    return [self fileWithPath:path withUser:user withProject:project withXfast:xfast withSource:@"<data>"];
}


- (XFSourceFile *)dataOutfileWithPath:(NSString *)path
                             withUser:(NSString *)user
                          withProject:(NSString *)project
                            withXfast:(NSString *)xfast
{
    return [self outfileWithPath:path withUser:user withProject:project withXfast:xfast withSource:@"<data>"];
}

- (XFSourceFile *)xfastFileWithPath:(NSString *)path
                        withProject:(NSString *)project
                          withXfast:(NSString *)xfast
{
    return [self fileWithPath:path withUser:nil withProject:project withXfast:xfast withSource:@"<xfast>"];
}


- (XFSourceFile *)xfastOutfileWithPath:(NSString *)path
                           withProject:(NSString *)project
                             withXfast:(NSString *)xfast
{
    return [self outfileWithPath:path withUser:nil withProject:project withXfast:xfast withSource:@"<xfast>"];
}

- (XFSourceFile *)absoluteFileWithPath:(NSString *)path
{
    return [self fileWithPath:path withUser:nil withProject:nil withXfast:nil withSource:@"<absolute>"];
}


- (XFSourceFile *)absoluteOutfileWithPath:(NSString *)path
{
    return [self outfileWithPath:path withUser:nil withProject:nil withXfast:nil withSource:@"<absolute>"];
}



- (XFSourceFile *)fileWithPath:(NSString *)path
                      withUser:(NSString *)user
                   withProject:(NSString *)project
                     withXfast:(NSString *)xfast
                    withSource:(NSString *)source
{
    for (XFSourceFile* projectFile in [self files])
    {
        if ([source compare:@"<data>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile user] compare:user] == NSOrderedSame &&
                [[projectFile project] compare:project] == NSOrderedSame &&
                [[projectFile xfast] compare:xfast] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        } else if ([source compare:@"<xfast>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile project] compare:project] == NSOrderedSame &&
                [[projectFile xfast] compare:xfast] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        } else if ([source compare:@"<absolute>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        }
    }
    return nil;
}

- (XFSourceFile *)outfileWithPath:(NSString *)path
                         withUser:(NSString *)user
                      withProject:(NSString *)project
                        withXfast:(NSString *)xfast
                       withSource:(NSString *)source
{
    for (XFSourceFile* projectFile in [self outfiles])
    {
        if ([source compare:@"<data>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile user] compare:user] == NSOrderedSame &&
                [[projectFile project] compare:project] == NSOrderedSame &&
                [[projectFile xfast] compare:xfast] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        } else if ([source compare:@"<xfast>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile project] compare:project] == NSOrderedSame &&
                [[projectFile xfast] compare:xfast] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        } else if ([source compare:@"<absolute>"] == NSOrderedSame) {
            if ([[projectFile path] compare:path] == NSOrderedSame &&
                [[projectFile sourceTree] compare:source] == NSOrderedSame) {
                //
                return projectFile;
            }
        }
    }
    return nil;
}



- (NSArray*)headerFiles
{
    return [self projectFilesOfType:XFSourceCodeHeader];
}

- (NSArray*)objectiveCFiles
{
    return [self projectFilesOfType:XFSourceCodeObjC];
}

- (NSArray*)objectiveCPlusPlusFiles
{
    return [self projectFilesOfType:XFSourceCodeObjCPlusPlus];
}


- (NSArray*)xibFiles
{
    return [self projectFilesOfType:XFXibFile];
}

- (NSArray*)imagePNGFiles
{
    return [self projectFilesOfType:XFImageResourcePNG];
}

// need this value to construct relative path in XfastprojDefinition
- (NSString*)filePath
{
    return _filePath;
}

#pragma mark Groups

- (NSMutableArray *)asTreeData
{
//    NSMutableArray *mainLeavesOfTree = [[self rootGroup] asTreeData];
    NSMutableArray *trees = [NSMutableArray array];
    
    NSMutableArray *emptyEntries = [NSMutableArray array];
    NSMutableArray *tree = [NSMutableArray array];
    NSMutableDictionary *item = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                 [self name], @"group",
                                 [self rootObjectKey], @"key",
                                 emptyEntries, @"entries",
                                 [self name], @"path",
                                 nil];
    [tree addObject:item];
    [trees addObject:tree];
    
    // Target
    NSMutableArray *tree1 = [NSMutableArray array];
    for (XFTarget *target in [self targets]) {
        
        NSMutableArray *emptyEntries1 = [NSMutableArray array];
        
        NSMutableArray *emptyEntries11 = [NSMutableArray array];
        for (XFSourceFile *file in [target sourceMembers]) {
            NSMutableDictionary *item111 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                            [self pathToFile:[file path]
                                                    withUser:[file user]
                                                 withProject:[file project]
                                                   withXfast:[file xfast]
                                                  withSource:[file sourceTree]], @"url",
                                           [file key], @"key",
                                           [file path], @"path",
                                           nil];
            [emptyEntries11 addObject:item111];
        }
        
        NSMutableArray *emptyEntries12 = [NSMutableArray array];
        for (XFSourceFile *file in [target outputMembers]) {
            NSMutableDictionary *item112 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                            [self pathToFile:[file path]
                                                    withUser:[file user]
                                                 withProject:[file project]
                                                   withXfast:[file xfast]
                                                  withSource:[file sourceTree]], @"url",
                                           [file key], @"key",
                                           [file path], @"path",
                                           nil];
            [emptyEntries12 addObject:item112];
        }
        
        
        
        NSMutableDictionary *item11 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                       @"in", @"group",
                                       [target key], @"key",
                                       emptyEntries11, @"entries",
                                       @"in", @"path",
                                       nil];
        NSMutableDictionary *item12 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                       @"out", @"group",
                                       [target key], @"key",
                                       emptyEntries12, @"entries",
                                       @"out", @"path",
                                       nil];
        [emptyEntries1 addObject:item11];
        [emptyEntries1 addObject:item12];
        
        
        NSMutableDictionary *item1 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                      [target name], @"group",
                                      [target key], @"key",
                                      emptyEntries1, @"entries",
                                      [target name], @"path",
                                      nil];
        [tree1 addObject:item1];
    }
    [trees addObject:tree1];
    
    
    NSMutableArray *tree2 = [NSMutableArray array];
    for (XFSourceFile *file in [self files]) {
        NSMutableDictionary *item2 = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                      [file absolutePath], @"url",
                                      [file key], @"key",
                                      [file absolutePath], @"path",
                                      nil];
        [tree2 addObject:item2];
    }
    [trees addObject:tree2];
    
    return trees;
}

- (NSArray*)groups
{

    NSMutableArray* results = [[NSMutableArray alloc] init];
    [[_dataStore objectForKey:@"objects"]
     enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXGroupType
            || [[obj valueForKeyPath:@"isa"] asMemberType] == XFPBXVariantGroupType)
        {
            [results addObject:[self groupWithKey:key]];
        }
    }];
    return results;
}

//TODO: Optimize this implementation.
- (XFGroup*)rootGroup
{
    for (XFGroup* group in [self groups])
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
    XFGroup* group = [self rootGroup];
    if (group)
    {
        return [NSArray arrayWithObject:group];
    }

    NSMutableArray* results = [NSMutableArray array];
    for (XFGroup* group in [self groups])
    {
        if ([group parentGroup] == nil)
        {
            [results addObject:group];
        }
    }

    return [results copy];
}

- (XFGroup*)groupWithKey:(NSString*)key
{
    XFGroup* group = [_groups objectForKey:key];
    if (group)
    {
        return group;
    }

    NSDictionary* obj = [[self objects] objectForKey:key];
    if (obj
        && ([[obj valueForKey:@"isa"] asMemberType] == XFPBXGroupType
            || [[obj valueForKey:@"isa"] asMemberType] == XFPBXVariantGroupType))
    {

        NSString* name = [obj valueForKey:@"name"];
        NSString* path = [obj valueForKey:@"path"];
        NSArray* children = [obj valueForKey:@"children"];
        XFGroup* group = [XFGroup groupWithProject:self key:key alias:name path:path children:children];

        [_groups setObject:group forKey:key];

        return group;
    }
    return nil;
}

- (XFGroup*)groupForGroupMemberWithKey:(NSString*)key
{
    for (XFGroup* group in [self groups])
    {
        if ([group memberWithKey:key])
        {
            return group;
        }
    }
    return nil;
}

- (XFGroup*)groupWithSourceFile:(XFSourceFile*)sourceFile
{
    for (XFGroup* group in [self groups])
    {
        for (id <XfastGroupMember> member in [group members])
        {
            if ([member isKindOfClass:[XFSourceFile class]]
                && [[sourceFile key] isEqualToString:[member key]])
            {
                return group;
            }
        }
    }
    return nil;
}

//TODO: This could fail if the path attribute on a given group is more than one directory. Start with candidates and
//TODO: search backwards.
- (XFGroup*)groupWithPathFromRoot:(NSString*)path
{
    NSArray* pathItems = [path componentsSeparatedByString:@"/"];
    XFGroup* currentGroup = [self rootGroup];
    for (NSString* pathItem in pathItems)
    {
        id <XfastGroupMember> group = [currentGroup memberWithDisplayName:pathItem];
        if ([group isKindOfClass:[XFGroup class]])
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

#pragma mark Command Lines


#pragma mark targets

- (XFTarget*)currentTarget
{
    return [self targetWithKey:_currentTargetKey];
}

/**
 * Returns the target with the given key.
 */
- (XFTarget*)targetWithKey:(NSString*)key
{
    for (XFTarget* target in [self targets])
    {
        if ([[target key] isEqualToString:key])
        {
            return target;
        }
    }
    return nil;
}

- (NSArray*)targets
{
    if (_targets == nil)
    {
        _targets = [[NSMutableArray alloc] init];
        [[self objects]
         enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
        {
            if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXNativeTargetType)
            {
                XFTarget* target =
                [XFTarget targetWithProject:self
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



//- (void)addTarget:(XFTarget *)target
//{
//    [self targets];
//    [_targets addObject:target];
//}


- (XFTarget*)targetWithName:(NSString*)name
{
    for (XFTarget* target in [self targets])
    {
        if ([[target name] isEqualToString:name])
        {
            return target;
        }
    }
    return nil;
}
//
//- (void)save
//{
//    [_fileOperationQueue commitFileOperations];
//    [_dataStore writeToFile:_filePath atomically:YES];
////    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]
////                 atomically:YES];
//
//    NSLog(@"Saved xfastProject");
//}

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
//    [_dataStore writeToFile:_filePath atomically:YES];
    [[_dataStore description] writeToFile:_filePath
                               atomically:YES
                                 encoding:NSUTF8StringEncoding error:nil];
    //    [self writeWithDictionary:_dataStore
    //                       ToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]];
    //    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"] atomically:YES];
}

- (void)saveToPBXProj
{
    [_fileOperationQueue commitFileOperations];
//    [_dataStore writeToFile:_filePath atomically:YES];
    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]
                 atomically:YES];
    
    NSLog(@"Saved xfastProject");
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


- (NSDictionary*)configurations
{
    if (_configurations == nil)
    {
        NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
        
        NSString* buildConfigurationRootSectionKey = [obj objectForKey:@"buildConfigurationList"];
        
        NSDictionary* buildConfigurationDictionary = [[self objects] objectForKey:buildConfigurationRootSectionKey];
        
        _configurations =
            [[XFBuildConfiguration
              buildConfigurationsFromArray:[buildConfigurationDictionary objectForKey:@"buildConfigurations"]
              inProject:self] mutableCopy];
        
        _defaultConfigurationName = [[buildConfigurationDictionary objectForKey:@"defaultConfigurationName"] copy];
    }

    return [_configurations copy];
}

- (NSDictionary*)configurationWithName:(NSString*)name
{
    return [[self configurations] objectForKey:name];
}

- (XFBuildConfiguration*)defaultConfiguration
{
    return [[self configurations] objectForKey:_defaultConfigurationName];
}

- (NSMutableDictionary *)defaultBuildsettings
{
    // 1. Root key
    // 2. Configuration list key
    // 3. Build configuration key
    // 4. Build settings
    return [self buildsettingsOfTargetWithKey:[self rootObjectKey]];
//    
//    NSMutableDictionary *project = [[self objects] objectForKey:];
//    NSString *confListKey = [project objectForKey:@"buildConfigurationList"];
//    NSMutableDictionary *buildConfList = [[self objects] objectForKey:confListKey];
//    NSString *buildConfKey = [[buildConfList objectForKey:@"buildConfigurations"] firstObject];
//    NSMutableDictionary *buildConf = [[self objects] objectForKey:buildConfKey];
//    NSMutableDictionary *buildSettings = [buildConf objectForKey:@"buildSettings"];
//    return buildSettings;
}

- (NSMutableDictionary *)buildsettingsOfTargetWithKey:(NSString *)key
{
    // 1. Root key
    // 2. Configuration list key
    // 3. Build configuration key
    // 4. Build settings
    
    NSMutableDictionary *project = [[self objects] objectForKey:key];
    NSString *confListKey = [project objectForKey:@"buildConfigurationList"];
    NSMutableDictionary *buildConfList = [[self objects] objectForKey:confListKey];
    NSString *buildConfKey = [[buildConfList objectForKey:@"buildConfigurations"] firstObject];
    NSMutableDictionary *buildConf = [[self objects] objectForKey:buildConfKey];
    NSMutableDictionary *buildSettings = [buildConf objectForKey:@"buildSettings"];
    return buildSettings;
}

- (NSString *)textWithKey:(NSString *)key
{
    XFSourceFile *file = [self fileWithKey:key];
    NSString *filePath = [file absolutePath];
    NSString *text = [NSString stringWithContentsOfFile:filePath
                                               encoding:NSUTF8StringEncoding
                                                  error:nil];
    return text;
}

- (NSString *)pathWithKey:(NSString *)key
{
    XFSourceFile *file = [self fileWithKey:key];
    return [file absolutePath];
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

- (NSArray*)projectFilesOfType:(XfastSourceFileType)projectFileType
{
    NSMutableArray* results = [NSMutableArray array];
    for (XFSourceFile* file in [self files])
    {
//        if ([file type] == projectFileType)
        if (1)
        {
            [results addObject:file];
        }
    }
    return results;
}

// File Type
- (XfastSourceFileType)fileTypeWithKey:(NSString *)key
{
    NSDictionary *obj = [[self objects] objectForKey:key];
    switch ([[obj valueForKey:@"isa"] asMemberType]) {
        case XFPBXProjectType:
            return XfastProject;
            break;
        case XFPBXNativeTargetType:
            return XfastTarget;
            break;
        case XFPBXFileReferenceType:
            return XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);
//            XfastSourceFileType XFSourceFileTypeFromStringRepresentation(NSString* string)
            break;
        default:
            return XFTEXT;
            break;
    }
}

/**
 *  Returns the type of the current Xfast.
 *
 *  @return The type of the current Xfast
 */
- (NSString *)type
{
    // 1. Find the root project as a dictionary.
    // 2. Find the type in the project.
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    return [obj objectForKey:@"type"];
}

/**
 *  Returns the operation of the Xfast.
 *
 *  @return The operation of the Xfast.
 */
- (NSString *)operation
{
    // 1. Find the root project as a dictionary.
    // 2. Find the type in the project.
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    return [obj objectForKey:@"operation"];
}



- (NSString *)pathToDatabaseForXfast
{
    NSString *path = [NSString stringWithFormat:@"%@/%@/%@",
                      [self databaseDirectory],
                      [self userName],
                      [self xcprojectFilename]];
    return path;
}



- (void)setDatabaseDirectory:(NSString *)aDatabaseDirectory
{
    NSMutableDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    [obj setObject:aDatabaseDirectory forKey:@"database"];
}

- (NSString *)databaseDirectory
{
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    return [obj objectForKey:@"database"];
}


- (void)setProjectDirectory:(NSString *)aProjectDirectory
{
    NSMutableDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    [obj setObject:aProjectDirectory forKey:@"project"];
}


- (NSString *)projectDirectory
{
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    return [obj objectForKey:@"project"];

}

- (void)setUserName:(NSString *)aUserName
{
    NSMutableDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    [obj setObject:aUserName forKey:@"user"];
}

- (NSString *)userName
{
    NSDictionary *obj = [[self objects] objectForKey:[self rootObjectKey]];
    return [obj objectForKey:@"user"];

}

- (NSString *)xcprojectFilename
{
    return [[self projectDirectory] lastPathComponent];
}

/**
 *  Prints the content of the Xfast.
 */
- (void)print
{
    // Project info.
    NSLog(@"\n");
    NSLog(@"--- Xfast Begin ---");
    NSLog(@"Xfast path: %@", [self filePath]);
    NSLog(@"Xfast name: %@", [self name]);
    NSLog(@"Xfast database: %@", [self databaseDirectory]);
    NSLog(@"Xfast directory: %@", [self projectDirectory]);
    NSLog(@"Xfast user: %@", [self userName]);
    NSLog(@"XCProject: %@", [self xcprojectFilename]);
    
    // Configurations
    for (id key in [self configurations]) {
        XFBuildConfiguration *conf = [[self configurations] objectForKey:key];
        NSLog(@"Configuration: %@", key);
        [conf print];
    }
    
    // Input file references
    NSLog(@"--- All input file Begin ---");
    for (XFSourceFile *file in [self files]) {
        [file print];
        NSLog(@"path : [%@]", [self pathToFile:[file path]
                                      withUser:[file user]
                                   withProject:[file project]
                                     withXfast:[file xfast]
                                    withSource:[file sourceTree]]);
    }
    NSLog(@"--- All input file End ---");
    
    // Output file references
    NSLog(@"--- All output file Begin ---");
    for (XFSourceFile *file in [self outfiles]) {
        [file print];
        NSLog(@"path : [%@]", [self pathToFile:[file path]
                                      withUser:[file user]
                                   withProject:[file project]
                                     withXfast:[file xfast]
                                    withSource:[file sourceTree]]);
    }
    NSLog(@"--- All output file End ---");
    
    // Targets
    for (XFTarget* target in [self targets])
    {
        [target print];
    }
    NSLog(@"--- Xfast End   ---");
}

@end




































