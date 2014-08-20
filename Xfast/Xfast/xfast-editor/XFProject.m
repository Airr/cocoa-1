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

@interface XFProject ()

- (NSString *)stringObjectsWithDictionary:(NSDictionary *)data rootKey:(NSString *)key;

- (NSDictionary *)objectsWithRootKey:(NSString *)rootKey;

- (NSDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
                      buildConfigurationList:(NSString *)buildKey;

- (NSDictionary *)objectBuildConfiguration;

- (NSDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys;

- (NSDictionary *)objectGroup;

- (NSDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files;

- (NSDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases;

@end

@implementation XFProject


@synthesize fileOperationQueue = _fileOperationQueue;

#pragma mark - Class Methods

+ (XFProject*)projectWithNewFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    return [[XFProject alloc] initWithNewFilePath:filePath baseDirectory:dirPath];
}

+ (XFProject*)projectWithFilePath:(NSString*)filePath
         baseDirectory:(NSString *)dirPath
{
    return [[XFProject alloc] initWithFilePath:filePath baseDirectory:dirPath];
}

#pragma mark - Initialization & Destruction

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
        [data setObject:[self objectsWithRootKey:rootObjectKey]
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

/* ====================================================================================================================================== */
#pragma mark - Interface Methods
#pragma mark Streams

- (BOOL)writeWithDictionary:(NSDictionary *)data ToFile:(NSString *)path
{
    NSMutableString *contentProjectFile = [NSMutableString stringWithCapacity:1000];
    
    [contentProjectFile appendFormat:@"// !$*UTF8*$!\n"];
    [contentProjectFile appendFormat:@"{\n"];
    
    [contentProjectFile appendFormat:@"\tarchiveVersion = %@;\n",
     [data objectForKey:@"archiveVersion"]];
    
    [contentProjectFile appendFormat:@"\tclasses = {\n"];
    [contentProjectFile appendFormat:@"\t};\n"];
    
    [contentProjectFile appendFormat:@"\tobjectVersion = %@;\n",
     [data objectForKey:@"objectVersion"]];
    
    [contentProjectFile appendFormat:@"\tobjects = {\n"];
    [contentProjectFile appendFormat:@"\n"];
    
    NSString *objects = [self stringObjectsWithDictionary:[data objectForKey:@"objects"]
                                            rootKey:[data objectForKey:@"rootObject"]];
    [contentProjectFile appendString:objects];
    
    [contentProjectFile appendFormat:@"\t};\n"];
    
    [contentProjectFile appendFormat:@"\trootObject = %@;\n",
     [data objectForKey:@"rootObject"]];
    
    [contentProjectFile appendFormat:@"}\n"];
    
    [contentProjectFile writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
    return YES;
}

- (NSString *)stringObjectsWithDictionary:(NSDictionary *)data rootKey:(NSString *)key
{
    NSMutableString *content = [NSMutableString stringWithCapacity:1000];
    
    [content appendFormat:@"/* Begin XFPBXProject section */\n"];
    NSDictionary *project = [data objectForKey:key];
    [content appendFormat:@"\t\t%@ /* Project object */ = {\n", key];
    [content appendFormat:@"\t\t\tisa = %@;\n", [project objectForKey:@"isa"]];
    [content appendFormat:@"\t\t\tmainGroup = %@;\n", [project objectForKey:@"mainGroup"]];
    [content appendFormat:@"\t\t\tbuildConfigurationList = %@;\n", [project objectForKey:@"buildConfigurationList"]];
    [content appendFormat:@"\t\t\tcompatibilityVersion = \"%@\";\n",
     [project objectForKey:@"compatibilityVersion"]];
    [content appendFormat:@"\t\t};\n"];
    [content appendFormat:@"/* End XFPBXProject section */\n\n"];
    
    [content appendFormat:@"/* Begin XFBuildConfiguration section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFBuildConfigurationType)
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
    [content appendFormat:@"/* End XFBuildConfiguration section */\n\n"];
  
    [content appendFormat:@"/* Begin XFConfigurationList section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFConfigurationListType)
        {
            [content appendFormat:@"\t\t%@ /* Build configuration list for XFPBXProject */ = {\n", key];
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
    [content appendFormat:@"/* End XFConfigurationList section */\n\n"];
    
    [content appendFormat:@"/* Begin XFPBXGroup section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXGroupType)
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
    [content appendFormat:@"/* End XFPBXGroup section */\n\n"];
    
    [content appendFormat:@"/* Begin XFPBXSourcesBuildPhase section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXSourcesBuildPhaseType)
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
    [content appendFormat:@"/* End XFPBXSourcesBuildPhase section */\n\n"];
    
    [content appendFormat:@"/* Begin XFPBXNativeTarget section */\n"];
    [data enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXNativeTargetType)
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
    [content appendFormat:@"/* End XFPBXNativeTarget section */\n\n"];
   
    
    return content;
}

- (NSDictionary *)objectsWithRootKey:(NSString *)rootKey
{
    
    NSDictionary *buildConfiguration = [self objectBuildConfiguration];
    NSString *buildConfigurationPath = [rootKey stringByAppendingString:@"buildConfiguration"];
    NSString *buildConfigurationKey = [[XFKeyBuilder forItemNamed:buildConfigurationPath] build];
    
    NSArray *buildConfigurations = [NSArray arrayWithObjects:
                                    buildConfigurationKey,
                                    nil];
    NSDictionary *configurationList = [self objectConfigurationListWithConfigurations:buildConfigurations];
    NSString *configurationListPath = [rootKey stringByAppendingString:@"configurationList"];
    NSString *configurationListKey = [[XFKeyBuilder forItemNamed:configurationListPath] build];
    
    NSDictionary *group = [self objectGroup];
    NSString *groupPath = [rootKey stringByAppendingString:@"group"];
    NSString *groupKey = [[XFKeyBuilder forItemNamed:groupPath] build];
    
    NSDictionary *project = [self objectProjectWithMainGroup:groupKey
                                      buildConfigurationList:configurationListKey];
    
    NSDictionary *sourceBuildPhase = [self objectSourceBuildPhaseWithFiles:nil];
    NSString *sourceBuildPhasePath = [rootKey stringByAppendingString:@"sourceBuildPhase"];
    NSString *sourceBuildPhaseKey = [[XFKeyBuilder forItemNamed:sourceBuildPhasePath] build];
    
    NSArray *buildPhases = [NSArray arrayWithObjects:
                            sourceBuildPhaseKey,
                            nil];
    NSDictionary *target = [self objectTargetWithBuildConfigurationList:configurationListKey
                                                            buildPhases:buildPhases];
    NSString *targetPath = [rootKey stringByAppendingString:@"target"];
    NSString *targetKey = [[XFKeyBuilder forItemNamed:targetPath] build];
    
    NSDictionary *objects = [NSDictionary dictionaryWithObjectsAndKeys:
                             project, rootKey,
                             buildConfiguration, buildConfigurationKey,
                             configurationList, configurationListKey,
                             group, groupKey,
                             sourceBuildPhase, sourceBuildPhaseKey,
                             target, targetKey,
                             nil];
    return objects;
}

- (NSDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
                      buildConfigurationList:(NSString *)buildKey
{
    NSDictionary *project = [NSDictionary dictionaryWithObjectsAndKeys:
                             @"XFPBXProject", @"isa",
                             @"Xfast 1.0", @"compatibilityVersion",
                             groupKey, @"mainGroup",
                             @"xxx", @"productRefGroup",
                             @"\"\"", @"projectDirPath",
                             @"\"\"", @"projectRoot",
                             buildKey, @"buildConfigurationList",
                             nil];
    return project;
}

- (NSDictionary *)objectBuildConfiguration
{
    NSDictionary *buildSettings = [NSDictionary dictionaryWithObjectsAndKeys:
                                   @"NO", @"ALWAYS_SEARCH_USER_PATHS",
                                   @"\"gnu++0x\"", @"CLANG_CXX_LANGUAGE_STANDARD",
                                   @"\"lxbc++\"", @"CLANG_CXX_LIBRARY",
                                   @"10.9", @"MACOSX_DEPLOYMENT_TARGET",
                                   @"macosx", @"SDKROOT",
                                   nil];
    NSDictionary *buildConfiguration = [NSDictionary dictionaryWithObjectsAndKeys:
                                        @"XFBuildConfiguration", @"isa",
                                        buildSettings, @"buildSettings",
                                        @"Release", @"name",
                                        nil];
    return buildConfiguration;
}

- (NSDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys
{
    NSDictionary *configurationList = [NSDictionary dictionaryWithObjectsAndKeys:
                                       @"XFConfigurationList", @"isa",
                                       @0, @"defaultConfigurationIsVisible",
                                       @"Release", @"defaultConfigurationName",
                                       keys, @"buildConfigurations",
                                       nil];
    return configurationList;
}

- (NSDictionary *)objectGroup
{
    NSArray *children = [NSArray array];
    NSDictionary *group = [NSDictionary dictionaryWithObjectsAndKeys:
                           @"XFPBXGroup", @"isa",
                           children, @"children",
                           @"\"<group>\"", @"sourceTree",
                           nil];
    return group;
}

- (NSDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files
{
    NSDictionary *sourceBuildPhase = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"XFPBXSourcesBuildPhase", @"isa",
                                      @0, @"runOnlyForDeploymentPostprocessing",
                                      files, @"files",
                                      nil];
    return sourceBuildPhase;
}

- (NSDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases
{
    NSDictionary *target = [NSDictionary dictionaryWithObjectsAndKeys:
                            @"XFPBXNativeTarget", @"isa",
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

#pragma mark Files

- (NSArray*)files
{
    NSMutableArray* results = [NSMutableArray array];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXFileReferenceType)
        {
            XfastSourceFileType fileType = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);
            NSString* path = [obj valueForKey:@"path"];
            NSString* sourceTree = [obj valueForKey:@"sourceTree"];
            [results addObject:[XFSourceFile
                                sourceFileWithProject:self
                                key:key
                                type:fileType
                                name:path
                                sourceTree:(sourceTree ? sourceTree : @"<group>")
                                path:nil]];
        }
    }];
    return results;
}

- (XFSourceFile*)fileWithKey:(NSString*)key
{
    NSDictionary* obj = [[self objects] valueForKey:key];
    if (obj &&
        ([[obj valueForKey:@"isa"] asMemberType] == XFPBXFileReferenceType
         || [[obj valueForKey:@"isa"] asMemberType] == XFPBXReferenceProxyType))
    {
        XfastSourceFileType fileType = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);

        NSString* name = [obj valueForKey:@"name"];
        NSString* sourceTree = [obj valueForKey:@"sourceTree"];

        if (name == nil)
        {
            name = [obj valueForKey:@"path"];
        }
        return [XFSourceFile sourceFileWithProject:self
                                               key:key
                                              type:fileType
                                              name:name
                                        sourceTree:(sourceTree ? sourceTree : @"<group>")
                                              path:[obj valueForKey:@"path"]];
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

/* ====================================================================================================================================== */
#pragma mark Groups

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

- (void)addTarget:(XFTarget *)target
{
    [self targets];
    [_targets addObject:target];
    
//    if (_targets == nil)
//    {
//        _targets = [[NSMutableArray alloc] init];
//        
//        [[self objects]
//         enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
//         {
//             if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXNativeTargetType)
//             {
//                 XFTarget* target =
//                 [XFTarget targetWithProject:self
//                                         key:key
//                                        name:[obj valueForKey:@"name"]
//                                 productName:[obj valueForKey:@"productName"]
//                            productReference:[obj valueForKey:@"productReference"]];
//                 [_targets addObject:target];
//             }
//         }];
//    }
//    return _targets;
}


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

- (void)save
{
    [_fileOperationQueue commitFileOperations];
    [_dataStore writeToFile:_filePath atomically:YES];
//    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]
//                 atomically:YES];

    NSLog(@"Saved xfastProject");
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
        NSString* buildConfigurationRootSectionKey =
            [[[self objects] objectForKey:[self rootObjectKey]] objectForKey:@"buildConfigurationList"];
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

/* ====================================================================================================================================== */
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
        if ([file type] == projectFileType)
        {
            [results addObject:file];
        }
    }
    return results;
}

@end
