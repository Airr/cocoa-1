//
//  SCCJobWelcome.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCJobWelcome.h"
#import "XfastEditor.h"

@interface SCCJobWelcome ()

+ (NSString *)stringObjectsWithDictionary:(NSDictionary *)data
                                  rootKey:(NSString *)key;

+ (NSDictionary *)objectsWithRootKey:(NSString *)rootKey;

+ (NSDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
                      buildConfigurationList:(NSString *)buildKey;

+ (NSDictionary *)objectBuildConfiguration;

+ (NSDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys;

+ (NSDictionary *)objectGroup;

+ (NSDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files;

+ (NSDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases;

@end

@implementation SCCJobWelcome

#pragma mark Inits

+ (NSString *)template
{
    NSMutableDictionary *data = [NSMutableDictionary dictionary];
    [data setObject:@"1" forKey:@"archiveVersion"];
    [data setObject:@"46" forKey:@"objectVersion"];
    NSString* rootObjectKey = [[XFKeyBuilder createUnique] build];
    [data setObject:rootObjectKey forKey:@"rootObject"];
    [data setObject:[SCCJobWelcome objectsWithRootKey:rootObjectKey]
             forKey:@"objects"];
    return [NSString stringWithFormat:@"%@", data];
}

#pragma mark Streams

+ (BOOL)writeWithDictionary:(NSDictionary *)data ToFile:(NSString *)path
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
    
    NSString *objects = [SCCJobWelcome stringObjectsWithDictionary:[data objectForKey:@"objects"]
                                                  rootKey:[data objectForKey:@"rootObject"]];
    [contentProjectFile appendString:objects];
    
    [contentProjectFile appendFormat:@"\t};\n"];
    
    [contentProjectFile appendFormat:@"\trootObject = %@;\n",
     [data objectForKey:@"rootObject"]];
    
    [contentProjectFile appendFormat:@"}\n"];
    
    [contentProjectFile writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
    return YES;
}

+ (NSString *)stringObjectsWithDictionary:(NSDictionary *)data rootKey:(NSString *)key
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


#pragma mark Template

+ (NSDictionary *)objectsWithRootKey:(NSString *)rootKey
{
    
    NSDictionary *buildConfiguration = [SCCJobWelcome objectBuildConfiguration];
    NSString *buildConfigurationPath = [rootKey stringByAppendingString:@"buildConfiguration"];
    NSString *buildConfigurationKey = [[XFKeyBuilder forItemNamed:buildConfigurationPath] build];
    
    NSArray *buildConfigurations = [NSArray arrayWithObjects:
                                    buildConfigurationKey,
                                    nil];
    NSDictionary *configurationList = [SCCJobWelcome objectConfigurationListWithConfigurations:buildConfigurations];
    NSString *configurationListPath = [rootKey stringByAppendingString:@"configurationList"];
    NSString *configurationListKey = [[XFKeyBuilder forItemNamed:configurationListPath] build];
    
    NSDictionary *group = [SCCJobWelcome objectGroup];
    NSString *groupPath = [rootKey stringByAppendingString:@"group"];
    NSString *groupKey = [[XFKeyBuilder forItemNamed:groupPath] build];
    
    NSDictionary *project = [SCCJobWelcome objectProjectWithMainGroup:groupKey
                                      buildConfigurationList:configurationListKey];
    
    NSDictionary *sourceBuildPhase = [SCCJobWelcome objectSourceBuildPhaseWithFiles:nil];
    NSString *sourceBuildPhasePath = [rootKey stringByAppendingString:@"sourceBuildPhase"];
    NSString *sourceBuildPhaseKey = [[XFKeyBuilder forItemNamed:sourceBuildPhasePath] build];
    
    NSArray *buildPhases = [NSArray arrayWithObjects:
                            sourceBuildPhaseKey,
                            nil];
    NSDictionary *target = [SCCJobWelcome objectTargetWithBuildConfigurationList:configurationListKey
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

+ (NSDictionary *)objectProjectWithMainGroup:(NSString *)groupKey
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

+ (NSDictionary *)objectBuildConfiguration
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

+ (NSDictionary *)objectConfigurationListWithConfigurations:(NSArray *)keys
{
    NSDictionary *configurationList = [NSDictionary dictionaryWithObjectsAndKeys:
                                       @"XFConfigurationList", @"isa",
                                       @0, @"defaultConfigurationIsVisible",
                                       @"Release", @"defaultConfigurationName",
                                       keys, @"buildConfigurations",
                                       nil];
    return configurationList;
}

+ (NSDictionary *)objectGroup
{
    NSArray *children = [NSArray array];
    NSDictionary *group = [NSDictionary dictionaryWithObjectsAndKeys:
                           @"XFPBXGroup", @"isa",
                           children, @"children",
                           @"\"<group>\"", @"sourceTree",
                           nil];
    return group;
}

+ (NSDictionary *)objectSourceBuildPhaseWithFiles:(NSArray *)files
{
    NSDictionary *sourceBuildPhase = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"XFPBXSourcesBuildPhase", @"isa",
                                      @0, @"runOnlyForDeploymentPostprocessing",
                                      files, @"files",
                                      nil];
    return sourceBuildPhase;
}

+ (NSDictionary *)objectTargetWithBuildConfigurationList:(NSString *)buildKey
                                             buildPhases:(NSArray *)phases
{
    NSDictionary *target = [NSDictionary dictionaryWithObjectsAndKeys:
                            @"XFPBXNativeTarget", @"isa",
                            buildKey, @"buildConfigurationList",
                            phases, @"buildPhases",
                            nil];
    return target;
}


@end
