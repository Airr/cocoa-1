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

#import "XFGroup.h"
#import "XFKeyBuilder.h"
#import "XFTarget.h"
#import "XFSourceFile.h"
#import "XFProject.h"
#import "XFBuildConfiguration.h"

@implementation XFTarget

#pragma mark - Class Methods

+ (XFTarget*)targetWithProject:(XFProject*)project
                           key:(NSString*)key
                          name:(NSString*)name
                   productName:(NSString*)productName
              productReference:(NSString*)productReference
{
    return [[XFTarget alloc] initWithProject:project
                                         key:key
                                        name:name
                                 productName:productName
                            productReference:productReference];
}


#pragma mark - Initialization & Destruction

- (id)initWithProject:(XFProject*)project
                  key:(NSString*)key
                 name:(NSString*)name
          productName:(NSString*)productName
     productReference:(NSString*)productReference
{
    self = [super init];
    if (self)
    {
        _project = project;
        _key = [key copy];
        _name = [name copy];
        _productName = [productName copy];
        _productReference = [productReference copy];
    }
    return self;
}

#pragma mark - Interface Methods

- (NSArray*)resources
{
    if (_resources == nil)
    {
        _resources = [[NSMutableArray alloc] init];
        for (NSString* buildPhaseKey in [[[_project objects] objectForKey:_key] objectForKey:@"buildPhases"])
        {
            NSDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
            if ([[buildPhase valueForKey:@"isa"] asMemberType] == XFPBXResourcesBuildPhaseType)
            {
                for (NSString* buildFileKey in [buildPhase objectForKey:@"files"])
                {
                    XFSourceFile* targetMember = [self buildFileWithKey:buildFileKey];
                    if (targetMember)
                    {
                        [_resources addObject:[self buildFileWithKey:buildFileKey]];
                    }
                }
            }
        }
    }

    return _resources;
}

- (NSDictionary*)configurations
{
    if (_configurations == nil)
    {
        NSString* buildConfigurationRootSectionKey = [[[_project objects] objectForKey:_key] objectForKey:@"buildConfigurationList"];
        NSDictionary* buildConfigurationDictionary = [[_project objects] objectForKey:buildConfigurationRootSectionKey];
        _configurations =
            [[XFBuildConfiguration
              buildConfigurationsFromArray:[buildConfigurationDictionary objectForKey:@"buildConfigurations"]
              inProject:_project]
             mutableCopy];
        _defaultConfigurationName = [[buildConfigurationDictionary objectForKey:@"defaultConfigurationName"] copy];
    }

    return _configurations;
}

- (XFBuildConfiguration*)defaultConfiguration
{
    return [[self configurations] objectForKey:_defaultConfigurationName];
}

- (XFBuildConfiguration*)configurationWithName:(NSString*)name
{
    return [[self configurations] objectForKey:name];
}

- (NSArray*)members
{
    if (_members == nil)
    {
        _members = [[NSMutableArray alloc] init];
        for (NSString* buildPhaseKey in [[[_project objects] objectForKey:_key] objectForKey:@"buildPhases"])
        {
            NSDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
            if ([[buildPhase valueForKey:@"isa"] asMemberType] == XFPBXSourcesBuildPhaseType ||
                [[buildPhase valueForKey:@"isa"] asMemberType] == XFPBXFrameworksBuildPhaseType)
            {
                for (NSString* buildFileKey in [buildPhase objectForKey:@"files"])
                {
                    XFSourceFile* targetMember = [self buildFileWithKey:buildFileKey];
                    if (targetMember)
                    {
                        [_members addObject:[_project fileWithKey:targetMember.key]];
                    }
                }
            }
        }
    }
    return _members;
}

/**
 * 
 * A source file must have had the file reference. The function becomeBuildFile
 * creates a build file for the file reference. It gets the target with the 
 * key. The target contains buildPhases, in which I can find source, framework,
 * and resource buildPhases. I add the build file key to the array of files.
 */
- (void)addMember:(XFSourceFile*)member
{
    [member becomeBuildFile];
    NSDictionary* target = [[_project objects] objectForKey:_key];

    for (NSString* buildPhaseKey in [target objectForKey:@"buildPhases"])
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        if ([[buildPhase valueForKey:@"isa"] asMemberType] == [member buildPhase])
        {

            NSMutableArray* files = [buildPhase objectForKey:@"files"];
            if (![files containsObject:[member buildFileKey]])
            {
                [files addObject:[member buildFileKey]];
            }

            [buildPhase setObject:files forKey:@"files"];
        }
    }
    [self flagMembersAsDirty];
}

/**
 * Creates a dictionary of PBXBuildFile with file references as keys.
 */
- (NSDictionary*)buildRefWithFileRefKey
{
    NSMutableDictionary* buildRefWithFileRefDict = [NSMutableDictionary dictionary];
    NSDictionary* allObjects = [_project objects];
    NSArray* keys = [allObjects allKeys];

    for (NSString* key in keys)
    {
        NSDictionary* dictionaryInfo = [allObjects objectForKey:key];

        NSString* type = [dictionaryInfo objectForKey:@"isa"];
        if (type)
        {
            if ([type isEqualToString:@"XFPBXBuildFile"])
            {
                NSString* fileRef = [dictionaryInfo objectForKey:@"fileRef"];

                if (fileRef)
                {
                    [buildRefWithFileRefDict setObject:key forKey:fileRef];
                }
            }
        }
    }
    return buildRefWithFileRefDict;
}

/**
 * Removes files from the Target.
 * The key must be one for PBXFileReference. BuildPhases conain an array of
 * files. This array consists of BuildFile elements.
 */
- (void)removeMemberWithKey:(NSString*)key
{

    NSDictionary* buildRefWithFileRef = [self buildRefWithFileRefKey];
    NSDictionary* target = [[_project objects] objectForKey:_key];
    NSString* buildRef = [buildRefWithFileRef objectForKey:key];

    if (!buildRef)
    {
        return;
    }

    for (NSString* buildPhaseKey in [target objectForKey:@"buildPhases"])
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        NSMutableArray* files = [buildPhase objectForKey:@"files"];

        [files removeObjectIdenticalTo:buildRef];
        [buildPhase setObject:files forKey:@"files"];
    }
    [self flagMembersAsDirty];
}

- (void)removeMembersWithKeys:(NSArray*)keys
{
    for (NSString* key in keys)
    {
        [self removeMemberWithKey:key];
    }
}

- (void)addDependency:(NSString*)key
{
    NSDictionary* targetObj = [[_project objects] objectForKey:_key];
    NSMutableArray* dependencies = [targetObj valueForKey:@"dependencies"];
    // add only if not already there
    BOOL found = NO;
    for (NSString* dependency in dependencies)
    {
        if ([dependency isEqualToString:key])
        {
            found = YES;
            break;
        }
    }
    if (!found)
    {
        [dependencies addObject:key];
    }
}

- (instancetype)duplicateWithTargetName:(NSString*)targetName productName:(NSString*)productName
{

    NSDictionary* targetObj = _project.objects[_key];
    NSMutableDictionary* dupTargetObj = [targetObj mutableCopy];

    dupTargetObj[@"name"] = targetName;
    dupTargetObj[@"productName"] = productName;

    NSString* buildConfigurationListKey = dupTargetObj[@"buildConfigurationList"];

    void(^visitor)(NSMutableDictionary*) = ^(NSMutableDictionary* buildConfiguration)
    {
        buildConfiguration[@"buildSettings"][@"PRODUCT_NAME"] = productName;
    };

    dupTargetObj[@"buildConfigurationList"] =
        [XFBuildConfiguration duplicatedBuildConfigurationListWithKey:buildConfigurationListKey inProject:_project
            withBuildConfigurationVisitor:visitor];

    [self duplicateProductReferenceForTargetObject:dupTargetObj withProductName:productName];

    [self duplicateBuildPhasesForTargetObject:dupTargetObj];

    [self addReferenceToProductsGroupForTargetObject:dupTargetObj];

    NSString* dupTargetObjKey = [self addTargetToRootObjectTargets:dupTargetObj];

    [_project dropCache];

    return [[XFTarget alloc] initWithProject:_project key:dupTargetObjKey name:targetName productName:productName
        productReference:dupTargetObj[@"productReference"]];
}

/* ====================================================================================================================================== */
#pragma mark - Overridden Methods

- (void)setName:(NSString*)name
{
    _name = name;
    NSDictionary* dictionary = [_project.objects objectForKey:_key];
    [dictionary setValue:_name forKey:@"name"];
}

- (void)setProductName:(NSString*)productName
{
    _productName = productName;
    NSDictionary* dictionary = [_project.objects objectForKey:_key];
    [dictionary setValue:_productName forKey:@"productName"];
}

#pragma mark - Protocol Methods

- (XcodeMemberType)groupMemberType
{
    return XFPBXNativeTargetType;
}

- (NSImage*)displayImage
{
    return [NSImage imageNamed:NSImageNamePreferencesGeneral];
}

- (NSString*)displayName
{
    return _name;
}

- (NSString*)pathRelativeToProjectRoot
{
    NSString* parentPath = [[_project groupForGroupMemberWithKey:_key] pathRelativeToProjectRoot];
    NSString* result = [parentPath stringByAppendingPathComponent:_name];
    return result;
}

/**
 * Returns the full path to the file.
 */
- (NSString *)absolutePath
{
    // 1. Find the file reference.
    NSDictionary *fileReference = [[_project objects] objectForKey:_key];
    if (fileReference) {
        if ([(NSString *)[fileReference valueForKey:@"sourceTree"] compare:@"<absolute>"] == NSOrderedSame) {
            return [fileReference valueForKey:@"path"];
        } else {
            return nil;
        }
    } else {
        return nil;
    }
}


#pragma mark - Utility Methods

- (NSString*)description
{
    return [NSString stringWithFormat:@"Target: name=%@, files=%@", _name, _members];
}


#pragma mark - Private Methods

- (XFSourceFile*)buildFileWithKey:(NSString*)theKey
{
    NSDictionary* obj = [[_project objects] valueForKey:theKey];
    if (obj)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXBuildFileType)
        {
            return [_project fileWithKey:[obj valueForKey:@"fileRef"]];
        }
    }
    return nil;
}

- (void)flagMembersAsDirty
{
    _members = nil;
}

- (void)duplicateProductReferenceForTargetObject:(NSMutableDictionary*)dupTargetObj withProductName:(NSString*)productName
{

    NSString* productReferenceKey = dupTargetObj[@"productReference"];
    NSMutableDictionary* dupProductReference = [_project.objects[productReferenceKey] mutableCopy];

    NSString* path = dupProductReference[@"path"];
    NSString* dupPath = [path stringByDeletingLastPathComponent];
    dupPath = [dupPath stringByAppendingPathComponent:productName];
    dupPath = [dupPath stringByAppendingPathExtension:@"app"];
    dupProductReference[@"path"] = dupPath;

    NSString* dupProductReferenceKey = [[XFKeyBuilder createUnique] build];

    _project.objects[dupProductReferenceKey] = dupProductReference;
    dupTargetObj[@"productReference"] = dupProductReferenceKey;
}

- (void)duplicateBuildPhasesForTargetObject:(NSMutableDictionary*)dupTargetObj
{

    NSMutableArray* buildPhases = [NSMutableArray array];

    for (NSString* buildPhaseKey in dupTargetObj[@"buildPhases"])
    {

        NSMutableDictionary* dupBuildPhase = [_project.objects[buildPhaseKey] mutableCopy];
        NSMutableArray* dupFiles = [NSMutableArray array];

        for (NSString* fileKey in dupBuildPhase[@"files"])
        {

            NSMutableDictionary* dupFile = [_project.objects[fileKey] mutableCopy];
            NSString* dupFileKey = [[XFKeyBuilder createUnique] build];

            _project.objects[dupFileKey] = dupFile;
            [dupFiles addObject:dupFileKey];
        }

        dupBuildPhase[@"files"] = dupFiles;

        NSString* dupBuildPhaseKey = [[XFKeyBuilder createUnique] build];
        _project.objects[dupBuildPhaseKey] = dupBuildPhase;
        [buildPhases addObject:dupBuildPhaseKey];
    }

    dupTargetObj[@"buildPhases"] = buildPhases;
}

- (void)addReferenceToProductsGroupForTargetObject:(NSMutableDictionary*)dupTargetObj
{

    XFGroup* mainGroup = nil;
    NSPredicate* productsPredicate = [NSPredicate predicateWithFormat:@"displayName == 'Products'"];
    NSArray* filteredGroups = [_project.groups filteredArrayUsingPredicate:productsPredicate];

    if (filteredGroups.count > 0)
    {
        mainGroup = filteredGroups[0];
        NSMutableArray* children = [_project.objects[mainGroup.key][@"children"] mutableCopy];
        [children addObject:dupTargetObj[@"productReference"]];
        _project.objects[mainGroup.key][@"children"] = children;
    }
}

- (NSString*)addTargetToRootObjectTargets:(NSMutableDictionary*)dupTargetObj
{
    NSString* dupTargetObjKey = [[XFKeyBuilder createUnique] build];

    _project.objects[dupTargetObjKey] = dupTargetObj;

    NSString* rootObjKey = _project.dataStore[@"rootObject"];
    NSMutableDictionary* rootObj = [_project.objects[rootObjKey] mutableCopy];
    NSMutableArray* rootObjTargets = [rootObj[@"targets"] mutableCopy];
    [rootObjTargets addObject:dupTargetObjKey];

    rootObj[@"targets"] = rootObjTargets;
    _project.objects[rootObjKey] = rootObj;

    return dupTargetObjKey;
}

@end
