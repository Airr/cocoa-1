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

#import <Cocoa/Cocoa.h>
#import "XFGroup.h"
#import "XFKeyBuilder.h"
#import "XFTarget.h"
#import "XFSourceFile.h"
#import "XFProject.h"
#import "XFBuildConfiguration.h"

@interface XFTarget ()



@end

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

- (NSArray *)sourceMembers
{
    if (_sourceMembers == nil) {
        _sourceMembers = [self membersWithIndex:0];
    }
    return _sourceMembers;
}

- (NSArray *)outputMembers
{
    if (_outputMembers == nil) {
        _outputMembers = [self membersWithIndex:1];
    }
    return _outputMembers;
}

- (NSMutableArray*)membersWithIndex:(NSUInteger)index
{
    NSMutableArray *sourceMembers = [[NSMutableArray alloc] init];
    NSDictionary* target = [[_project objects] objectForKey:_key];
    
    NSString* buildPhaseKey = [[target objectForKey:@"buildPhases"] objectAtIndex:index];
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        {
            for (NSString *buildFileKey in [buildPhase objectForKey:@"files"]) {
                XFSourceFile* targetMember = [self buildFileWithKey:buildFileKey];
                
                if (targetMember)
                {
                    [sourceMembers addObject:[_project fileWithKey:targetMember.key]];
                }
            }
        }
    }
    return sourceMembers;
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

- (void)addMember:(XFSourceFile*)member withIndex:(NSUInteger)index
{
    [member makeBuildFile];
    NSDictionary* target = [[_project objects] objectForKey:_key];
    
    NSString* buildPhaseKey = [[target objectForKey:@"buildPhases"] objectAtIndex:index];
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
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


- (void)addSourceMember:(XFSourceFile*)member
{
    [self addMember:member withIndex:0];
}

- (void)addOutputMember:(XFSourceFile*)member
{
    [self addMember:member withIndex:1];
}


/**
 * Creates a dictionary of PBXBuildFile with file references as keys. 
 *  
 *  Use this dictionary to find the build file key of a reference file key.
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

/**
 *  Removes a source build file from the target.
 *
 *  @param key The key of a file reference.
 *
 *  Note that we need the file reference not the build file reference. 
 * The reference file stays in the project, and one of the build file of the
 * referenece file is removed from the current target.
 */

- (BOOL)existSourceMemberWithKey:(NSString*)key
{
    return [self existMemberWithKey:key withIndex:0];
}

- (BOOL)existOutputMemberWithKey:(NSString*)key
{
    return [self existMemberWithKey:key withIndex:1];
}

- (void)removeSourceMemberWithKey:(NSString*)key
{
    [self removeMemberWithKey:key withIndex:0];
}

- (void)removeSourceMemberWithName:(NSString*)name
{
    [self removeMemberWithName:name withIndex:0];
}

- (void)removeAllSourceMembers
{
    [self removeAllMembersWithIndex:0];
}

- (void)removeOutputMemberWithKey:(NSString*)key
{
    [self removeMemberWithKey:key withIndex:1];
}

- (void)removeOutputMemberWithName:(NSString*)name
{
    [self removeMemberWithName:name withIndex:1];
}

- (void)removeAllOutputMembers
{
    [self removeAllMembersWithIndex:1];
}

- (BOOL)existMemberWithKey:(NSString*)key withIndex:(NSUInteger)index
{
    NSDictionary* target = [[_project objects] objectForKey:_key];
    NSString* buildPhaseKey = [[target objectForKey:@"buildPhases"] objectAtIndex:index];
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        NSMutableArray* files = [buildPhase objectForKey:@"files"];
        
        // Check if each build file points to the reference file with key.
        // Find the build file.
        BOOL isFound = NO;
        for (NSString *buildFileKey in files) {
            NSDictionary *buildFile = [[_project objects] objectForKey:buildFileKey];
            NSString *refFileKey = [buildFile objectForKey:@"fileRef"];
            if ([refFileKey compare:key] == NSOrderedSame) {
                isFound = YES;
                break;
            }
        }
        return isFound;
    }
    
//    
//    NSDictionary* buildRefWithFileRef = [self buildRefWithFileRefKey];
////    NSDictionary* target = [[_project objects] objectForKey:_key];
//    NSString* buildRef = [buildRefWithFileRef objectForKey:key];
//    
//    if (!buildRef) {
//        return NO;
//    } else {
//        return YES;
//    }
}

- (void)removeMemberWithKey:(NSString*)key withIndex:(NSUInteger)index
{
    NSDictionary* buildRefWithFileRef = [self buildRefWithFileRefKey];
    NSDictionary* target = [[_project objects] objectForKey:_key];
    NSString* buildRef = [buildRefWithFileRef objectForKey:key];
    
    if (!buildRef)
    {
        return;
    }
    
    NSString* buildPhaseKey = [[target objectForKey:@"buildPhases"] objectAtIndex:index];
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        NSMutableArray* files = [buildPhase objectForKey:@"files"];
        
        [files removeObjectIdenticalTo:buildRef];
        [buildPhase setObject:files forKey:@"files"];
    }
    [self flagMembersAsDirty];
}


- (void)removeMemberWithName:(NSString*)name withIndex:(NSUInteger)index
{
    XFSourceFile *file = [_project fileWithName:name];
    NSString *key = [file key];
    [self removeMemberWithKey:key withIndex:index];
}

- (void)removeAllMembersWithIndex:(NSUInteger)index
{
    NSDictionary* target = [[_project objects] objectForKey:_key];
    
    NSString* buildPhaseKey = [[target objectForKey:@"buildPhases"] objectAtIndex:index];
    {
        NSMutableDictionary* buildPhase = [[_project objects] objectForKey:buildPhaseKey];
        NSMutableArray* files = [buildPhase objectForKey:@"files"];
        
        [files removeAllObjects];
        [buildPhase setObject:files forKey:@"files"];
    }
    [self flagMembersAsDirty];
}


- (void)removeAll
{
    NSMutableArray *keys = [NSMutableArray arrayWithObject:[self key]];
    
    NSDictionary *target = [[_project objects] objectForKey:_key];
    
    [keys addObjectsFromArray:[target objectForKey:@"buildPhases"]];
    NSString *buildConfigurationListKey = [target objectForKey:@"buildConfigurationList"];
    [keys addObject:[target objectForKey:@"buildConfigurationList"]];
    NSDictionary *buildConfigurationList = [[_project objects] objectForKey:buildConfigurationListKey];
    [keys addObjectsFromArray:[buildConfigurationList objectForKey:@"buildConfigurations"]];
    [[_project objects] removeObjectsForKeys:keys];
    [self flagMembersAsDirty];
}

#pragma mark - Update OutputBuildFiles

/**
 *  Updates the output build files given Xfast type, build configuration, 
 * input source build files.
 */
- (void)updateOutputBuildFiles
{

    NSString *operation = [_project operation];
    if ([operation compare:@"OneToOne"] == NSOrderedSame) {
        //
        [self updateOutputBuildFilesOneToOne];
    }
    //
}

- (void)updateOutputBuildFilesOneToOne
{
    [self removeAllOutputMembers];
    NSArray *sourceMembers = [self sourceMembers];
    for (XFSourceFile *file in sourceMembers) {
        // Create a new source file and add it to the target.
        NSString *path = [[file absolutePath] lastPathComponent];
        NSString *outPath = [NSString stringWithFormat:@"%@/%@",
                             [self pathToOutput],
                             path];
//        [_project addFileOfType:@"text" withDataPath:@""];
        [_project addFileOfType:@"text" withAbsolutePath:outPath];
        XFSourceFile *outfile = [_project fileWithPath:outPath];
        [self addOutputMember:outfile];
    }
}

- (void)updateOutputBuildFilesManyToOne
{
    [self removeAllOutputMembers];
    NSString *path = @"file";
    NSString *outPath = [NSString stringWithFormat:@"%@/%@",
                         [self pathToOutput],
                         path];
    [_project addFileOfType:@"text" withAbsolutePath:outPath];
    XFSourceFile *outfile = [_project fileWithPath:outPath];
    [self addOutputMember:outfile];
}

/**
 *  Returns the path to the output directory.
 *
 *  @return Path to the output directory as NSString.
 *
 *  The output directory is:
 * DatabaseDirectory/Username/Project/Xfast/Target/output
 *  
 *  We need to know the first four names.
 */
- (NSString *)pathToOutput
{
    NSString *path = [NSString stringWithFormat:@"%@/%@/%@/out",
                      [_project pathToDatabaseForXfast],
                      [_project name],
                      [self name]];
    return path;
//    return @"/tmp/Xfast/goshng/1/Xfast1.cat.xfast/Target1/output";
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

/**
 *  Returns the file pointed by the build key.
 *
 *  @param theKey The build file key.
 *
 *  @return The pointed file.
 */
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
    _sourceMembers = nil;
    _outputMembers = nil;
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

/**
 *  Prints the content of a Target.
 */
- (void)print
{
    // Target info.
    NSLog(@" *** target Bgn *** ");
    NSLog(@" name   : %@", [self name]);
    NSLog(@" key    : %@", [self key]);
    
    // Configurations
    for (id key in [self configurations]) {
        XFBuildConfiguration *conf = [[self configurations] objectForKey:key];
        NSLog(@"Configuration: %@", key);
        [conf print];
    }
    
    // Target infiles.
    NSLog(@"  - target input file Bgn - ");
    for (XFSourceFile *file in [self sourceMembers]) {
        [file print];
        NSLog(@"path : [%@]", [_project pathToFile:[file path]
                                          withUser:[file user]
                                       withProject:[file project]
                                         withXfast:[file xfast]
                                        withSource:[file sourceTree]]);
    }
    NSLog(@"  - target input file End - ");
    
    
    // Target outfiles.
    NSLog(@"  - target output file Bgn - ");
    for (XFSourceFile *file in [self outputMembers]) {
        [file print];
        NSLog(@"path : [%@]", [_project pathToFile:[file path]
                                          withUser:[file user]
                                       withProject:[file project]
                                         withXfast:[file xfast]
                                        withSource:[file sourceTree]]);
    }
    NSLog(@"  - target output file End - ");
    
    NSLog(@" *** target End *** ");
}

/**
 *  Adds output files in an array.
 *
 *  @return The array.
 */
- (NSArray *)arrayOfOutfiles
{
    NSMutableArray *array = [NSMutableArray array];
    for (XFSourceFile *file in [self outputMembers]) {
        NSDictionary *fileInfo = [NSDictionary dictionaryWithObjectsAndKeys:
                                  [[file name] lastPathComponent], @"name",
                                  [file key], @"key",
                                  [_project pathToFile:[file path]
                                              withUser:[file user]
                                           withProject:[file project]
                                             withXfast:[file xfast]
                                            withSource:[file sourceTree]], @"path",
                                  nil];
        [array addObject:fileInfo];
    }
    return array;
}


@end

























