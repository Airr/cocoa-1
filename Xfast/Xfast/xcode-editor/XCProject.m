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
#import "XCKeyBuilder.h"

@interface XCProject ()

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

@implementation XCProject


@synthesize fileOperationQueue = _fileOperationQueue;

#pragma mark - Class Methods

+ (XCProject*)projectWithNewFilePath:(NSString*)filePath
{
    return [[XCProject alloc] initWithNewFilePath:filePath];
}

+ (XCProject*)projectWithFilePath:(NSString*)filePath
{
    return [[XCProject alloc] initWithFilePath:filePath];
}

#pragma mark - Initialization & Destruction

- (id)initWithNewFilePath:(NSString*)filePath
{
    if ((self = [super init]))
    {
        NSFileManager* fileManager = [NSFileManager defaultManager];
//        if ([fileManager fileExistsAtPath:filePath]) {
//            return nil;
//        }
        if (![fileManager createDirectoryAtPath:filePath withIntermediateDirectories:YES attributes:nil error:nil])
        {
//            [NSException raise:NSInvalidArgumentException format:@"Error: Create folder failed %@", filePath];
        }
        
        NSString *projectName = [filePath lastPathComponent];
        NSString *projectMainPath = [filePath stringByAppendingPathComponent:projectName];
        NSString *projectXfastPath = [filePath stringByAppendingPathComponent:@"xfastfolder"];
        NSString *projectPath = [projectMainPath stringByAppendingPathExtension:@"xfastproj"];
        [fileManager createDirectoryAtPath:projectMainPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        [fileManager createDirectoryAtPath:projectXfastPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        [fileManager createDirectoryAtPath:projectPath
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        
        NSMutableDictionary *data = [NSMutableDictionary dictionary];
        [data setObject:@"1" forKey:@"archiveVersion"];
        [data setObject:@"46" forKey:@"objectVersion"];
        NSString* rootObjectKey = [[XCKeyBuilder forItemNamed:projectPath] build];
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
            [NSException raise:XCProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
        }

        _fileOperationQueue = [[XCFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];

    }
    return self;
}

- (id)initWithFilePath:(NSString*)filePath
{
    if ((self = [super init]))
    {
        _filePath = [filePath copy];
        _dataStore = [[NSMutableDictionary alloc]
                      initWithContentsOfFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"]];

        if (!_dataStore)
        {
            [NSException raise:XCProjectNotFoundException format:@"Project file not found at file path %@", _filePath];
        }

        _fileOperationQueue = [[XCFileOperationQueue alloc] initWithBaseDirectory:[_filePath stringByDeletingLastPathComponent]];

    }
    return self;
}

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

- (NSDictionary *)objectsWithRootKey:(NSString *)rootKey
{
    
    NSDictionary *buildConfiguration = [self objectBuildConfiguration];
    NSString *buildConfigurationPath = [rootKey stringByAppendingString:@"buildConfiguration"];
    NSString *buildConfigurationKey = [[XCKeyBuilder forItemNamed:buildConfigurationPath] build];
    
    NSArray *buildConfigurations = [NSArray arrayWithObjects:
                                    buildConfigurationKey,
                                    nil];
    NSDictionary *configurationList = [self objectConfigurationListWithConfigurations:buildConfigurations];
    NSString *configurationListPath = [rootKey stringByAppendingString:@"configurationList"];
    NSString *configurationListKey = [[XCKeyBuilder forItemNamed:configurationListPath] build];
    
    NSDictionary *group = [self objectGroup];
    NSString *groupPath = [rootKey stringByAppendingString:@"group"];
    NSString *groupKey = [[XCKeyBuilder forItemNamed:groupPath] build];
    
    NSDictionary *project = [self objectProjectWithMainGroup:groupKey
                                      buildConfigurationList:configurationListKey];
    
    NSDictionary *sourceBuildPhase = [self objectSourceBuildPhaseWithFiles:nil];
    NSString *sourceBuildPhasePath = [rootKey stringByAppendingString:@"sourceBuildPhase"];
    NSString *sourceBuildPhaseKey = [[XCKeyBuilder forItemNamed:sourceBuildPhasePath] build];
    
    NSArray *buildPhases = [NSArray arrayWithObjects:
                            sourceBuildPhaseKey,
                            nil];
    NSDictionary *target = [self objectTargetWithBuildConfigurationList:configurationListKey
                                                            buildPhases:buildPhases];
    NSString *targetPath = [rootKey stringByAppendingString:@"target"];
    NSString *targetKey = [[XCKeyBuilder forItemNamed:targetPath] build];
    
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
                                        @"XCBuildConfiguration", @"isa",
                                        buildSettings, @"buildSettings",
                                        @"Release", @"name",
                                        nil];
    return buildConfiguration;
}

- (NSDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys
{
    NSDictionary *configurationList = [NSDictionary dictionaryWithObjectsAndKeys:
                                       @"XCConfigurationList", @"isa",
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
                           @"PBXGroup", @"isa",
                           children, @"children",
                           @"\"<group>\"", @"sourceTree",
                           nil];
    return group;
}

- (NSDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files
{
    NSDictionary *sourceBuildPhase = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"PBXSourcesBuildPhase", @"isa",
                                      @0, @"runOnlyForDeploymentPostprocessing",
                                      files, @"files",
                                      nil];
    return sourceBuildPhase;
}

- (NSDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases
{
    NSDictionary *target = [NSDictionary dictionaryWithObjectsAndKeys:
                            @"PBXNativeTarget", @"isa",
                            buildKey, @"buildConfigurationList",
                            phases, @"buildPhases",
                            nil];
    return target;
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
            NSString* sourceTree = [obj valueForKey:@"sourceTree"];
            [results addObject:[XCSourceFile sourceFileWithProject:self key:key type:fileType name:path
                sourceTree:(sourceTree ? sourceTree : @"<group>") path:nil]];
        }
    }];
    return results;
}

- (XCSourceFile*)fileWithKey:(NSString*)key
{
    NSDictionary* obj = [[self objects] valueForKey:key];
    if (obj && ([[obj valueForKey:@"isa"] asMemberType] == PBXFileReferenceType || [[obj valueForKey:@"isa"] asMemberType] ==
        PBXReferenceProxyType))
    {
        XcodeSourceFileType fileType = XCSourceFileTypeFromStringRepresentation([obj valueForKey:@"lastKnownFileType"]);

        NSString* name = [obj valueForKey:@"name"];
        NSString* sourceTree = [obj valueForKey:@"sourceTree"];

        if (name == nil)
        {
            name = [obj valueForKey:@"path"];
        }
        return [XCSourceFile sourceFileWithProject:self key:key type:fileType name:name sourceTree:(sourceTree ? sourceTree : @"<group>")
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

/* ====================================================================================================================================== */
#pragma mark Groups

- (NSArray*)groups
{

    NSMutableArray* results = [[NSMutableArray alloc] init];
    [[_dataStore objectForKey:@"objects"] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {

        if ([[obj valueForKey:@"isa"] asMemberType] == PBXGroupType || [[obj valueForKeyPath:@"isa"] asMemberType] == PBXVariantGroupType)
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
    if (obj && ([[obj valueForKey:@"isa"] asMemberType] == PBXGroupType || [[obj valueForKey:@"isa"] asMemberType] == PBXVariantGroupType))
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

//TODO: This could fail if the path attribute on a given group is more than one directory. Start with candidates and
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


/* ====================================================================================================================================== */
#pragma mark targets

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
                    [XCTarget targetWithProject:self key:key name:[obj valueForKey:@"name"] productName:[obj valueForKey:@"productName"]
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

- (void)save
{
    [_fileOperationQueue commitFileOperations];
    [_dataStore writeToFile:[_filePath stringByAppendingPathComponent:@"project.pbxproj"] atomically:YES];

    NSLog(@"Saved project");
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