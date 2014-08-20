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



#import "XFBuildConfiguration.h"
#import "XFGroup.h"
#import "XFKeyBuilder.h"
#import "XFProject.h"
#import "XFSourceFile.h"
#import "OCLogTemplate.h"

@implementation XFBuildConfiguration

/* ====================================================================================================================================== */
#pragma mark - Class Methods

+ (NSDictionary*)buildConfigurationsFromArray:(NSArray*)array inProject:(XFProject*)project
{
    NSMutableDictionary* configurations = [NSMutableDictionary dictionary];

    for (NSString* buildConfigurationKey in array)
    {
        NSDictionary* buildConfiguration = [[project objects] objectForKey:buildConfigurationKey];

        if ([[buildConfiguration valueForKey:@"isa"] asMemberType] == XFBuildConfigurationType)
        {
            XFBuildConfiguration* configuration = [configurations objectForKey:[buildConfiguration objectForKey:@"name"]];
            if (!configuration)
            {
                configuration = [[XFBuildConfiguration alloc] initWithProject:project key:buildConfigurationKey];

                [configurations setObject:configuration forKey:[buildConfiguration objectForKey:@"name"]];
            }


            XFSourceFile* configurationFile = [project fileWithKey:[buildConfiguration objectForKey:@"baseConfigurationReference"]];
            if (configurationFile)
            {
                NSString* path = configurationFile.path;

                if (![[NSFileManager defaultManager] fileExistsAtPath:path])
                {
                    XFGroup* group = [project groupWithSourceFile:configurationFile];
                    path = [[group pathRelativeToParent] stringByAppendingPathComponent:path];
                }

                if (![[NSFileManager defaultManager] fileExistsAtPath:path])
                {
                    path = [[[project filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:path];
                }

                if (![[NSFileManager defaultManager] fileExistsAtPath:path])
                {
                    path = [[[project filePath] stringByDeletingLastPathComponent] stringByAppendingPathComponent:configurationFile.path];
                }

                if (![[NSFileManager defaultManager] fileExistsAtPath:path])
                {
                    [NSException raise:@"XFConfig not found" format:@"Unable to find XFConfig file at %@", path];
                }

            }

            [configuration addBuildSettings:[buildConfiguration objectForKey:@"buildSettings"]];
        }
    }

    return configurations;
}

+ (NSString*)duplicatedBuildConfigurationListWithKey:(NSString*)buildConfigurationListKey inProject:(XFProject*)project
    withBuildConfigurationVisitor:(void (^)(NSMutableDictionary*))buildConfigurationVisitor
{

    NSDictionary* buildConfigurationList = project.objects[buildConfigurationListKey];
    NSMutableDictionary* dupBuildConfigurationList = [buildConfigurationList mutableCopy];

    NSMutableArray* dupBuildConfigurations = [NSMutableArray array];

    for (NSString* buildConfigurationKey in buildConfigurationList[@"buildConfigurations"])
    {
        [dupBuildConfigurations addObject:[self duplicatedBuildConfigurationWithKey:buildConfigurationKey inProject:project
            withBuildConfigurationVisitor:buildConfigurationVisitor]];
    }

    dupBuildConfigurationList[@"buildConfigurations"] = dupBuildConfigurations;

    NSString* dupBuildConfigurationListKey = [[XFKeyBuilder createUnique] build];

    project.objects[dupBuildConfigurationListKey] = dupBuildConfigurationList;

    return dupBuildConfigurationListKey;
}

/* ====================================================================================================================================== */
#pragma mark - Initialization & Destruction

- (instancetype)initWithProject:(XFProject*)project key:(NSString*)key
{
    self = [super init];
    if (self)
    {
        _project = project;
        _key = [key copy];

        _buildSettings = [[NSMutableDictionary alloc] init];
        _xcconfigSettings = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (id)init
{
    return [self initWithProject:nil key:nil];
}


/* ====================================================================================================================================== */
#pragma mark - Interface Methods

- (NSDictionary*)specifiedBuildSettings
{
    return [_buildSettings copy];
}

- (void)addBuildSettings:(NSDictionary*)buildSettings
{
    [_xcconfigSettings removeObjectsForKeys:[buildSettings allKeys]];
    [_buildSettings addEntriesFromDictionary:buildSettings];
}

- (void)addOrReplaceSetting:(id <NSCopying>)setting forKey:(NSString*)key
{
    NSDictionary* settings = [NSDictionary dictionaryWithObject:setting forKey:key];
    [self addBuildSettings:settings];
    
    LogDebug(@"$$$$$$$$$$$ before: %@", [_project.objects objectForKey:_key]);
    
    NSMutableDictionary* dict = [[[_project objects] objectForKey:_key] mutableCopy];
    [dict setValue:_buildSettings forKey:@"buildSettings"];
    [_project.objects setValue:dict forKey:_key];
    
    LogDebug(@"The settings: %@", [_project.objects objectForKey:_key]);
    
}


- (id <NSCopying>)valueForKey:(NSString*)key
{
    id <NSCopying> value = [_buildSettings objectForKey:key];
    if (!value)
    {
        value = [_xcconfigSettings objectForKey:key];
    }
    return value;
}

/* ====================================================================================================================================== */
#pragma mark - Utility Methods

- (NSString*)description
{
    NSMutableString* description = [[super description] mutableCopy];

    [description appendFormat:@"build settings: %@, inherited: %@", _buildSettings, _xcconfigSettings];

    return description;
}


/* ====================================================================================================================================== */
#pragma mark - Private Methods

+ (NSString*)duplicatedBuildConfigurationWithKey:(NSString*)buildConfigurationKey inProject:(XFProject*)project
    withBuildConfigurationVisitor:(void (^)(NSMutableDictionary*))buildConfigurationVisitor
{
    NSDictionary* buildConfiguration = project.objects[buildConfigurationKey];
    NSMutableDictionary* dupBuildConfiguration = [buildConfiguration mutableCopy];

    buildConfigurationVisitor(dupBuildConfiguration);

    NSString* dupBuildConfigurationKey = [[XFKeyBuilder createUnique] build];

    project.objects[dupBuildConfigurationKey] = dupBuildConfiguration;

    return dupBuildConfigurationKey;
}

@end
