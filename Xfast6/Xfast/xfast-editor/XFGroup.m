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



#import "XFFrameworkDefinition.h"
#import "XFTarget.h"
#import "XFFileOperationQueue.h"
#import "XFXibDefinition.h"
#import "XFSourceFile.h"
#import "XFGroup.h"
#import "XFProject.h"
#import "XFClassDefinition.h"
#import "XFKeyBuilder.h"
#import "XFSourceFileDefinition.h"
#import "XFSubProjectDefinition.h"
#import "XFProject+SubProject.h"

@interface XFGroup ()

- (NSDictionary *)makeSourcesBuildPhase;

- (NSDictionary *)makeBuildConfiguration;

- (NSDictionary *)makeBuildSettings;

- (NSDictionary *)makeConfigurationListWithBuildConfigurations:(NSArray *)conf;

@end

@implementation XFGroup


/* ====================================================================================================================================== */
#pragma mark - Class Methods

+ (XFGroup*)groupWithProject:(XFProject*)project
                         key:(NSString*)key
                       alias:(NSString*)alias
                        path:(NSString*)path
                    children:(NSArray*)children
{

    return [[XFGroup alloc] initWithProject:project
                                        key:key
                                      alias:alias
                                       path:path
                                   children:children];
}

/* ====================================================================================================================================== */
#pragma mark - Initialization & Destruction

- (id)initWithProject:(XFProject*)project
                  key:(NSString*)key
                alias:(NSString*)alias
                 path:(NSString*)path
             children:(NSArray*)children
{
    self = [super init];
    if (self)
    {
        _project = project;
        _fileOperationQueue = [_project fileOperationQueue];
        _key = [key copy];
        _alias = [alias copy];
        _pathRelativeToParent = [path copy];

        _children = [children mutableCopy];
        if (!_children)
        {
            _children = [[NSMutableArray alloc] init];
        }
    }
    return self;
}

/* ====================================================================================================================================== */
#pragma mark - Interface Methods

#pragma mark Parent group

- (void)removeFromParentGroup
{
    [self removeFromParentDeletingChildren:NO];
}


- (void)removeFromParentDeletingChildren:(BOOL)deleteChildren
{
    if (deleteChildren)
    {
        [_fileOperationQueue queueDeletion:[self pathRelativeToProjectRoot]];
    }
    NSDictionary* dictionary = [[_project objects] objectForKey:_key];
    NSLog(@"Here's the dictionary: %@", dictionary);

    [[_project objects] removeObjectForKey:_key];

    dictionary = [[_project objects] objectForKey:_key];
    NSLog(@"Here's the dictionary: %@", dictionary);

    for (XFTarget* target in [_project targets])
    {
        [target removeMembersWithKeys:[self recursiveMembers]];
    }
    NSLog(@"Done!!!");
}

- (XFGroup*)parentGroup
{
    return [_project groupForGroupMemberWithKey:_key];
}

- (BOOL)isRootGroup
{
    return [self pathRelativeToParent] == nil && [self displayName] == nil;
}


/* ================================================================================================================== */
#pragma mark Adding children


- (void)addClass:(XFClassDefinition*)classDefinition
{

    if ([classDefinition header])
    {
        [self makeGroupMemberWithName:[classDefinition headerFileName]
                             contents:[classDefinition header]
                                 type:XFSourceCodeHeader
                   fileOperationStyle:[classDefinition fileOperationType]];
    }

    if ([classDefinition isObjectiveC])
    {
        [self makeGroupMemberWithName:[classDefinition sourceFileName]
                             contents:[classDefinition source]
                                 type:XFSourceCodeObjC
                   fileOperationStyle:[classDefinition fileOperationType]];
    }
    else if ([classDefinition isObjectiveCPlusPlus])
    {
        [self makeGroupMemberWithName:[classDefinition sourceFileName]
                             contents:[classDefinition source]
                                 type:XFSourceCodeObjCPlusPlus
                   fileOperationStyle:[classDefinition fileOperationType]];
    }

    [[_project objects] setObject:[self asDictionary] forKey:_key];
}


- (void)addClass:(XFClassDefinition*)classDefinition toTargets:(NSArray*)targets
{
    [self addClass:classDefinition];
    XFSourceFile* sourceFile = [_project fileWithName:[classDefinition sourceFileName]];
    [self addSourceFile:sourceFile toTargets:targets];
}

- (void)addFramework:(XFFrameworkDefinition*)frameworkDefinition
{
    if (([self memberWithDisplayName:[frameworkDefinition name]]) == nil)
    {
        NSDictionary* fileReference;
        if ([frameworkDefinition copyToDestination])
        {
            fileReference = [self makeFileReferenceWithPath:[frameworkDefinition name] name:nil type:XFFramework];
            BOOL copyFramework = NO;
            if ([frameworkDefinition fileOperationType] == XFFileOperationTypeOverwrite)
            {
                copyFramework = YES;
            }
            else if ([frameworkDefinition fileOperationType] == XFFileOperationTypeAcceptExisting)
            {
                NSString* frameworkName = [[frameworkDefinition filePath] lastPathComponent];
                if (![_fileOperationQueue fileWithName:frameworkName existsInProjectDirectory:[self pathRelativeToProjectRoot]])
                {
                    copyFramework = YES;
                }

            }
            if (copyFramework)
            {
                [_fileOperationQueue queueFrameworkWithFilePath:[frameworkDefinition filePath]
                    inDirectory:[self pathRelativeToProjectRoot]];
            }
        }
        else
        {
            NSString* path = [frameworkDefinition filePath];
            NSString* name = [frameworkDefinition name];
            fileReference = [self makeFileReferenceWithPath:path name:name type:XFFramework];
        }
        NSString* frameworkKey = [[XFKeyBuilder forItemNamed:[frameworkDefinition name]] build];
        [[_project objects] setObject:fileReference forKey:frameworkKey];
        [self addMemberWithKey:frameworkKey];
    }
    [[_project objects] setObject:[self asDictionary] forKey:_key];
}


- (void)addFramework:(XFFrameworkDefinition*)frameworkDefinition toTargets:(NSArray*)targets
{
    [self addFramework:frameworkDefinition];
    XFSourceFile* frameworkSourceRef = (XFSourceFile*) [self memberWithDisplayName:[frameworkDefinition name]];
    [self addSourceFile:frameworkSourceRef toTargets:targets];
}

- (XFGroup*)addGroupWithPath:(NSString*)path
{
    NSString* groupKeyPath = self.pathRelativeToProjectRoot ? [self.pathRelativeToProjectRoot stringByAppendingPathComponent:path] : path;

    NSString* groupKey = [[XFKeyBuilder forItemNamed:groupKeyPath] build];

    NSArray* members = [self members];
    for (id <XfastGroupMember> groupMember in members)
    {
        if ([groupMember groupMemberType] == XFPBXGroupType || [groupMember groupMemberType] == XFPBXVariantGroupType)
        {

            if ([[[groupMember pathRelativeToProjectRoot] lastPathComponent] isEqualToString:path]
                || [[groupMember displayName] isEqualToString:path]
                || [[groupMember key] isEqualToString:groupKey])
            {
                return nil;
            }
        }
    }

    XFGroup* group = [[XFGroup alloc] initWithProject:_project key:groupKey alias:nil path:path children:nil];
    NSDictionary* groupDict = [group asDictionary];

    [[_project objects] setObject:groupDict forKey:groupKey];
    [_fileOperationQueue queueDirectory:path inDirectory:[self pathRelativeToProjectRoot]];
    [self addMemberWithKey:groupKey];

    NSDictionary* dict = [self asDictionary];
    [[_project objects] setObject:dict forKey:_key];

    return group;
}

/**
 * Adds a target to the group.
 */
- (void)addTarget:(XFSourceFileDefinition*)sourceFileDefinition
{
    // 1. Add the target first.
    [self makeGroupMemberWithTarget:[sourceFileDefinition sourceFileName]
                               type:[sourceFileDefinition type]
                 fileOperationStyle:[sourceFileDefinition fileOperationType]];
    // 2. Set the target parent group with the new target.
    [[_project objects] setObject:[self asDictionary] forKey:_key];
    

}

- (void)addFile:(XFSourceFileDefinition*)sourceFileDefinition
{
    [self makeGroupMemberWithPath:[sourceFileDefinition sourceFileName]
                             type:[sourceFileDefinition type]
               fileOperationStyle:[sourceFileDefinition fileOperationType]];
    [[_project objects] setObject:[self asDictionary] forKey:_key];
}

- (void)addSourceFile:(XFSourceFileDefinition*)sourceFileDefinition
{
    [self makeGroupMemberWithName:[sourceFileDefinition sourceFileName]
                         contents:[sourceFileDefinition data]
                             type:[sourceFileDefinition type]
               fileOperationStyle:[sourceFileDefinition fileOperationType]];
    [[_project objects] setObject:[self asDictionary] forKey:_key];
}

- (void)addXib:(XFXibDefinition*)xibDefinition
{
    [self makeGroupMemberWithName:[xibDefinition xibFileName]
                         contents:[xibDefinition content]
                             type:XFXibFile
               fileOperationStyle:[xibDefinition fileOperationType]];
    [[_project objects] setObject:[self asDictionary] forKey:_key];
}

- (void)addXib:(XFXibDefinition*)xibDefinition toTargets:(NSArray*)targets
{
    [self addXib:xibDefinition];
    XFSourceFile* sourceFile = [_project fileWithName:[xibDefinition xibFileName]];
    [self addSourceFile:sourceFile toTargets:targets];
}


// adds an xcodeproj as a subproject of the current project.
- (void)addSubProject:(XFSubProjectDefinition*)projectDefinition
{
    // set up path to the xcodeproj file as Xfast sees it - path to top level of project + group path if any
    [projectDefinition initFullProjectPath:_project.filePath groupPath:[self pathRelativeToParent]];

    // create XFPBXFileReference for xcodeproj file and add to XFPBXGroup for the current group
    // (will retrieve existing if already there)
    [self makeGroupMemberWithName:[projectDefinition projectFileName]
                             path:[projectDefinition pathRelativeToProjectRoot]
                             type:XfastProject
               fileOperationStyle:[projectDefinition fileOperationType]];
    [[_project objects] setObject:[self asDictionary] forKey:_key];

    // create XFPBXFontainerItemProxies and XFPBXReferenceProxies
    [_project addProxies:projectDefinition];

    // add projectReferences key to XFPBXProject
    [self addProductsGroupToProject:projectDefinition];
}

// adds an xcodeproj as a subproject of the current project, and also adds all build products except for test bundle(s)
// to targets.
- (void)addSubProject:(XFSubProjectDefinition*)projectDefinition toTargets:(NSArray*)targets
{
    [self addSubProject:projectDefinition];

    // add subproject's build products to targets (does not add the subproject's test bundle)
    NSArray* buildProductFiles = [_project buildProductsForTargets:[projectDefinition projectKey]];
    for (XFSourceFile* file in buildProductFiles)
    {
        [self addSourceFile:file toTargets:targets];
    }
    // add main target of subproject as target dependency to main target of project
    [_project addAsTargetDependency:projectDefinition toTargets:targets];
}

// removes an xcodeproj from the current project.
- (void)removeSubProject:(XFSubProjectDefinition*)projectDefinition
{
    if (projectDefinition == nil)
    {
        return;
    }

    // set up path to the xcodeproj file as Xfast sees it - path to top level of project + group path if any
    [projectDefinition initFullProjectPath:_project.filePath groupPath:[self pathRelativeToParent]];

    NSString* xcodeprojKey = [projectDefinition projectKey];

    // Remove from group and remove XFPBXFileReference
    [self removeGroupMemberWithKey:xcodeprojKey];

    // remove XFPBXFontainerItemProxies and XFPBXReferenceProxies
    [_project removeProxies:xcodeprojKey];

    // get the key for the Products group
    NSString* productsGroupKey = [_project productsGroupKeyForKey:xcodeprojKey];

    // remove from the ProjectReferences array of XFPBXProject
    [_project removeFromProjectReferences:xcodeprojKey forProductsGroup:productsGroupKey];

    // remove PDXBuildFile entries
    [self removeProductsGroupFromProject:productsGroupKey];

    // remove Products group
    [[_project objects] removeObjectForKey:productsGroupKey];

    // remove from all targets
    [_project removeTargetDependencies:[projectDefinition name]];
}

- (void)removeSubProject:(XFSubProjectDefinition*)projectDefinition fromTargets:(NSArray*)targets
{
    if (projectDefinition == nil)
    {
        return;
    }

    // set up path to the xcodeproj file as Xfast sees it - path to top level of project + group path if any
    [projectDefinition initFullProjectPath:_project.filePath groupPath:[self pathRelativeToParent]];

    NSString* xcodeprojKey = [projectDefinition projectKey];

    // Remove XFPBXBundleFile entries and corresponding inclusion in XFPBXFrameworksBuildPhase and XFPBXResourcesBuidPhase
    NSString* productsGroupKey = [_project productsGroupKeyForKey:xcodeprojKey];
    [self removeProductsGroupFromProject:productsGroupKey];

    // Remove the XFPBXFontainerItemProxy for this xcodeproj with proxyType 1
    NSString* containerItemProxyKey = [_project containerItemProxyKeyForName:[projectDefinition pathRelativeToProjectRoot] proxyType:@"1"];
    if (containerItemProxyKey != nil)
    {
        [[_project objects] removeObjectForKey:containerItemProxyKey];
    }

    // Remove XFPBXTargetDependency and entry in XFPBXNativeTarget
    [_project removeTargetDependencies:[projectDefinition name]];
}

/* ====================================================================================================================================== */
#pragma mark Members

- (NSArray*)members
{
    if (_members == nil)
    {
        _members = [[NSMutableArray alloc] init];
        for (NSString* childKey in _children)
        {
            XcodeMemberType type = [self typeForKey:childKey];

            @autoreleasepool
            {
                if (type == XFPBXGroupType || type == XFPBXVariantGroupType)
                {
                    [_members addObject:[_project groupWithKey:childKey]];
                }
                else if (type == XFPBXFileReferenceType)
                {
                    [_members addObject:[_project fileWithKey:childKey]];
                }
                else if (type == XFPBXNativeTargetType)
                {
                    [_members addObject:[_project targetWithKey:childKey]];
                }
            }
        }
    }
    return _members;
}

- (NSArray*)recursiveMembers
{
    NSMutableArray* recursiveMembers = [NSMutableArray array];
    for (NSString* childKey in _children)
    {
        XcodeMemberType type = [self typeForKey:childKey];
        if (type == XFPBXGroupType || type == XFPBXVariantGroupType)
        {
            XFGroup* group = [_project groupWithKey:childKey];
            NSArray* groupChildren = [group recursiveMembers];
            [recursiveMembers addObjectsFromArray:groupChildren];
        }
        else if (type == XFPBXFileReferenceType)
        {
            [recursiveMembers addObject:childKey];
        }
    }
    [recursiveMembers addObject:_key];
    return [recursiveMembers arrayByAddingObjectsFromArray:recursiveMembers];
}

- (NSArray*)buildFileKeys
{

    NSMutableArray* arrayOfBuildFileKeys = [NSMutableArray array];
    for (id <XfastGroupMember> groupMember in [self members])
    {

        if ([groupMember groupMemberType] == XFPBXGroupType || [groupMember groupMemberType] == XFPBXVariantGroupType)
        {
            XFGroup* group = (XFGroup*) groupMember;
            [arrayOfBuildFileKeys addObjectsFromArray:[group buildFileKeys]];
        }
        else if ([groupMember groupMemberType] == XFPBXFileReferenceType)
        {
            [arrayOfBuildFileKeys addObject:[groupMember key]];
        }
    }
    return arrayOfBuildFileKeys;
}

- (id <XfastGroupMember>)memberWithKey:(NSString*)key
{
    id <XfastGroupMember> groupMember = nil;

    if ([_children containsObject:key])
    {
        XcodeMemberType type = [self typeForKey:key];
        if (type == XFPBXGroupType || type == XFPBXVariantGroupType)
        {
            groupMember = [_project groupWithKey:key];
        }
        else if (type == XFPBXFileReferenceType)
        {
            groupMember = [_project fileWithKey:key];
        }
    }
    return groupMember;
}

- (id <XfastGroupMember>)memberWithDisplayName:(NSString*)name
{
    for (id <XfastGroupMember> member in [self members])
    {
        if ([[member displayName] isEqualToString:name])
        {
            return member;
        }
    }
    return nil;
}

/* ====================================================================================================================================== */
#pragma mark - Protocol Methods

- (XcodeMemberType)groupMemberType
{
    return [self typeForKey:self.key];
}

- (NSImage*)displayImage
{
    return [NSImage imageNamed:NSImageNameHomeTemplate];
}

- (NSString*)displayName
{
    if (_alias)
    {
        return _alias;
    }
    return [_pathRelativeToParent lastPathComponent];
}

- (NSString*)pathRelativeToProjectRoot
{
    if (_pathRelativeToProjectRoot == nil)
    {
        NSMutableArray* pathComponents = [[NSMutableArray alloc] init];
        XFGroup* group = nil;
        NSString* key = [_key copy];

        while ((group = [_project groupForGroupMemberWithKey:key]) != nil && !([group pathRelativeToParent] == nil))
        {
            [pathComponents addObject:[group pathRelativeToParent]];
            id __unused old = key;
            key = [[group key] copy];
        }

        NSMutableString* fullPath = [[NSMutableString alloc] init];
        for (NSInteger i = (NSInteger) [pathComponents count] - 1; i >= 0; i--)
        {
            [fullPath appendFormat:@"%@/", [pathComponents objectAtIndex:i]];
        }
        _pathRelativeToProjectRoot = [[fullPath stringByAppendingPathComponent:_pathRelativeToParent] copy];
    }
    return _pathRelativeToProjectRoot;
}

- (NSString *)absolutePath
{
    //
    return nil;
}

/* ====================================================================================================================================== */
#pragma mark - Utility Methods

- (NSString*)description
{
    return [NSString stringWithFormat:@"Group: displayName = %@, key=%@", [self displayName], _key];
}

/* ====================================================================================================================================== */
#pragma mark - Private Methods

#pragma mark Private

- (void)addMemberWithKey:(NSString*)key
{

    for (NSString* childKey in _children)
    {
        if ([childKey isEqualToString:key])
        {
            [self flagMembersAsDirty];
            return;
        }
    }
    [_children addObject:key];
    [self flagMembersAsDirty];
}

- (void)flagMembersAsDirty
{
    _members = nil;
}


#pragma mark Xfastproj methods

/**
 * Creates a file in a group.
 */
- (void)makeGroupMemberWithName:(NSString*)name
                       contents:(id)contents
                           type:(XfastSourceFileType)type
             fileOperationStyle:(XFFileOperationType)fileOperationStyle
{
    NSString* filePath;
    XFSourceFile* currentSourceFile = (XFSourceFile*) [self memberWithDisplayName:name];
    if ((currentSourceFile) == nil)
    {
        NSDictionary* reference = [self makeFileReferenceWithPath:name name:nil type:type];
        NSString* fileKey = [[XFKeyBuilder forItemNamed:name] build];
        [[_project objects] setObject:reference forKey:fileKey];
        [self addMemberWithKey:fileKey];
        filePath = [self pathRelativeToProjectRoot];
    }
    else
    {
        filePath = [[currentSourceFile pathRelativeToProjectRoot] stringByDeletingLastPathComponent];
    }

    BOOL writeFile = NO;
    if (fileOperationStyle == XFFileOperationTypeOverwrite)
    {
        writeFile = YES;
        [_fileOperationQueue fileWithName:name existsInProjectDirectory:filePath];
    }
    else if (fileOperationStyle == XFFileOperationTypeAcceptExisting &&
        ![_fileOperationQueue fileWithName:name existsInProjectDirectory:filePath])
    {
        writeFile = YES;
    }
    if (writeFile)
    {
        if ([contents isKindOfClass:[NSString class]])
        {
            [_fileOperationQueue queueTextFile:name inDirectory:filePath withContents:contents];
        }
        else
        {
            [_fileOperationQueue queueDataFile:name inDirectory:filePath withContents:contents];
        }
    }
}


/**
 *
 * creates XFPBXFileReference and adds to group if not already there;  returns 
 * key for file reference.  Locates member via path rather than name, because
 * that is how subprojects are stored by Xfast.
 */
- (void)makeGroupMemberWithName:(NSString*)name
                           path:(NSString*)path
                           type:(XfastSourceFileType)type
             fileOperationStyle:(XFFileOperationType)fileOperationStyle
{
    XFSourceFile* currentSourceFile = (XFSourceFile*) [self memberWithDisplayName:name];
    if ((currentSourceFile) == nil)
    {
        NSDictionary* reference = [self makeFileReferenceWithPath:path name:name type:type];
        NSString* fileKey = [[XFKeyBuilder forItemNamed:name] build];
        [[_project objects] setObject:reference forKey:fileKey];
        [self addMemberWithKey:fileKey];
    }
}

- (void)makeGroupMemberWithPath:(NSString*)path
                           type:(XfastSourceFileType)type
             fileOperationStyle:(XFFileOperationType)fileOperationStyle
{
//    XFSourceFile* currentSourceFile = (XFSourceFile*) [self memberWithDisplayName:name];
//    if ((currentSourceFile) == nil)
    {
        NSDictionary* reference = [self makeFileReferenceWithAbsolutePath:path type:type];
        NSString* fileKey = [[XFKeyBuilder forItemNamed:path] build];
        [[_project objects] setObject:reference forKey:fileKey];
        [self addMemberWithKey:fileKey];
    }
}

- (void)makeGroupMemberWithTarget:(NSString*)path
                           type:(XfastSourceFileType)type
             fileOperationStyle:(XFFileOperationType)fileOperationStyle
{
//    XFSourceFile* currentSourceFile = (XFSourceFile*) [self memberWithDisplayName:name];
//    if ((currentSourceFile) == nil)
    {
        // 1. Create a new target.
        NSDictionary* reference = [self makeTargetWithName:path];
        NSString* fileKey = [[XFKeyBuilder forItemNamed:path] build];
        [[_project objects] setObject:reference forKey:fileKey];
        
        // 2. Add the new target to the parent  target group.
        [self addMemberWithKey:fileKey];
        
        // 3. Add the new target to the project.
        XFTarget* target =
        [XFTarget targetWithProject:_project
                                key:fileKey
                               name:[reference valueForKey:@"name"]
                        productName:[reference valueForKey:@"productName"]
                   productReference:[reference valueForKey:@"productReference"]];
        [_project addTarget:target];
    }
}

// makes a new group called Products and returns its key
- (NSString*)makeProductsGroup:(XFSubProjectDefinition*)xcodeprojDefinition
{
    NSMutableArray* children = [NSMutableArray array];
    NSString* uniquer = @"";
    for (NSString* productName in [xcodeprojDefinition buildProductNames])
    {
        [children addObject:[_project referenceProxyKeyForName:productName]];
        uniquer = [uniquer stringByAppendingString:productName];
    }
    NSString* productKey = [[XFKeyBuilder forItemNamed:[NSString stringWithFormat:@"%@-Products", uniquer]] build];
    XFGroup* productsGroup = [XFGroup groupWithProject:_project key:productKey alias:@"Products" path:nil children:children];
    [[_project objects] setObject:[productsGroup asDictionary] forKey:productKey];
    return productKey;
}

// makes a new Products group (by calling the method above), makes a new projectReferences array for it and
// then adds it to the XFPBXProject object
- (void)addProductsGroupToProject:(XFSubProjectDefinition*)xcodeprojDefinition
{
    NSString* productKey = [self makeProductsGroup:xcodeprojDefinition];

    NSMutableDictionary* XFPBXProjectDict = [_project XFPBXProjectDict];
    NSMutableArray* projectReferences = [XFPBXProjectDict valueForKey:@"projectReferences"];

    NSMutableDictionary* newProjectReference = [NSMutableDictionary dictionary];
    [newProjectReference setObject:productKey forKey:@"ProductGroup"];
    NSString* projectFileKey = [[_project fileWithName:[xcodeprojDefinition pathRelativeToProjectRoot]] key];
    [newProjectReference setObject:projectFileKey forKey:@"ProjectRef"];

    if (projectReferences == nil)
    {
        projectReferences = [NSMutableArray array];
    }
    [projectReferences addObject:newProjectReference];
    [XFPBXProjectDict setObject:projectReferences forKey:@"projectReferences"];
}

// removes XFPBXFileReference from group and project
- (void)removeGroupMemberWithKey:(NSString*)key
{
    NSMutableArray* children = [self valueForKey:@"children"];
    [children removeObject:key];
    [[_project objects] setObject:[self asDictionary] forKey:_key];
    // remove XFPBXFileReference
    [[_project objects] removeObjectForKey:key];
}

// removes the given key from the files arrays of the given section, if found (intended to be used with
// XFPBXFrameworksBuildPhase and XFPBXResourcesBuildPhase)
// they are not required because we are currently not adding these entries;  Xfast is doing it for us. The existing
// code for adding to a target doesn't do it, and I didn't add it since Xfast will take care of it for me and I was
// avoiding modifying existing code as much as possible)
- (void)removeBuildPhaseFileKey:(NSString*)key forType:(XcodeMemberType)memberType
{
    NSArray* buildPhases = [_project keysForProjectObjectsOfType:memberType withIdentifier:nil singleton:NO required:NO];
    for (NSString* buildPhaseKey in buildPhases)
    {
        NSDictionary* buildPhaseDict = [[_project objects] valueForKey:buildPhaseKey];
        NSMutableArray* fileKeys = [buildPhaseDict valueForKey:@"files"];
        for (NSString* fileKey in fileKeys)
        {
            if ([fileKey isEqualToString:key])
            {
                [fileKeys removeObject:fileKey];
            }
        }
    }
}

// removes entries from XFPBXBuildFiles, XFPBXFrameworksBuildPhase and XFPBXResourcesBuildPhase
- (void)removeProductsGroupFromProject:(NSString*)key
{
    // remove product group's build products from PDXBuildFiles
    NSDictionary* productsGroup = [[_project objects] objectForKey:key];
    for (NSString* childKey in [productsGroup valueForKey:@"children"])
    {
        NSArray* buildFileKeys = [_project keysForProjectObjectsOfType:XFPBXBuildFileType withIdentifier:childKey singleton:NO required:NO];
        // could be zero - we didn't add the test bundle as a build product
        if ([buildFileKeys count] == 1)
        {
            NSString* buildFileKey = [buildFileKeys objectAtIndex:0];
            [[_project objects] removeObjectForKey:buildFileKey];
            [self removeBuildPhaseFileKey:buildFileKey forType:XFPBXFrameworksBuildPhaseType];
            [self removeBuildPhaseFileKey:buildFileKey forType:XFPBXResourcesBuildPhaseType];
        }
    }
}

/* ====================================================================================================================================== */

#pragma mark Dictionary Representations

- (NSDictionary*)makeFileReferenceWithPath:(NSString*)path
                                      name:(NSString*)name
                                      type:(XfastSourceFileType)type
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFPBXFileReferenceType] forKey:@"isa"];
    [reference setObject:@"4" forKey:@"fileEncoding"];
    [reference setObject:NSStringFromXFSourceFileType(type) forKey:@"lastKnownFileType"];
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


- (NSDictionary*)makeFileReferenceWithAbsolutePath:(NSString*)path
                                              type:(XfastSourceFileType)type
{
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFPBXFileReferenceType] forKey:@"isa"];
    [reference setObject:@"4" forKey:@"fileEncoding"];
    [reference setObject:NSStringFromXFSourceFileType(type) forKey:@"lastKnownFileType"];
    [reference setObject:[path lastPathComponent] forKey:@"name"];

    if (path != nil)
    {
        [reference setObject:path forKey:@"path"];
    }
    [reference setObject:@"<absolute>" forKey:@"sourceTree"];
    return reference;
}

/**
 * Creates a target and its relatives such as biuldConfiguration and 
 * buildPhases.
 */
- (NSDictionary*)makeTargetWithName:(NSString*)path
{
    // 1. Create PBXSourcesBuildPhase.
    NSDictionary *sourcesBuildPhase = [self makeSourcesBuildPhase];
    NSString *sourcesBuildPhasePath = [path stringByAppendingString:@".sourcesBuildPhase"];
    NSString *sourcesBuildPhaseKey = [[XFKeyBuilder forItemNamed:sourcesBuildPhasePath] build];
    [[_project objects] setObject:sourcesBuildPhase
                           forKey:sourcesBuildPhaseKey];
    
    // 2. Create XFBuildConfiguration.
    NSDictionary *buildConfiguration = [self makeBuildConfiguration];
    NSString *buildConfigurationPath = [path stringByAppendingString:@".buildConfiguration"];
    NSString *buildConfigurationKey = [[XFKeyBuilder forItemNamed:buildConfigurationPath] build];
    [[_project objects] setObject:buildConfiguration
                           forKey:buildConfigurationKey];
    
    // 3. Create XFConfigurationList.
    NSArray *conf = [NSArray arrayWithObjects:buildConfigurationKey, nil];
    NSDictionary *configurationList = [self makeConfigurationListWithBuildConfigurations:conf];
    NSString *configurationListPath = [path stringByAppendingString:@".configurationList"];
    NSString *configurationListKey = [[XFKeyBuilder forItemNamed:configurationListPath] build];
    [[_project objects] setObject:configurationList
                           forKey:configurationListKey];
    
    // 4. Create a target.
    NSArray *buildPhases = [NSArray arrayWithObjects:sourcesBuildPhaseKey, nil];
    NSMutableDictionary* reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFPBXNativeTargetType] forKey:@"isa"];
    [reference setObject:configurationListKey forKey:@"buildConfigurationList"];
    [reference setObject:[NSArray array] forKey:@"buildRules"];
    [reference setObject:buildPhases forKey:@"buildPhases"];
    [reference setObject:[NSArray array] forKey:@"dependencies"];
    [reference setObject:[path lastPathComponent] forKey:@"productName"];
    [reference setObject:[path lastPathComponent] forKey:@"name"];
  
    [reference setObject:@"yet-to-be-implement" forKey:@"productReference"];
    

    return reference;
}

/**
 * Creates an empty PBXSourcesBuildPhase.
 */
- (NSDictionary *)makeSourcesBuildPhase
{
    NSMutableDictionary *reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFPBXSourcesBuildPhaseType]
                  forKey:@"isa"];
    [reference setObject:[NSMutableArray array] forKey:@"files"];
    return reference;
}

/**
 * Creates an empty XFBuildConfiguration.
 */
- (NSDictionary *)makeBuildConfiguration
{
    NSMutableDictionary *reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFBuildConfigurationType]
                  forKey:@"isa"];
    [reference setObject:@"Release" forKey:@"name"];
    [reference setObject:[NSDictionary dictionary] forKey:@"buildSettings"];
    
    return reference;
}

/**
 * Creates an empty buildSettings.
 */
- (NSDictionary *)makeBuildSettings
{
    NSMutableDictionary *reference = [NSMutableDictionary dictionary];
    [reference setObject:@"\"$(TARGET_NAME)\"" forKey:@"PRODUCT_NAME"];
    
    return reference;

}

/**
 * Creates an initial ConfigurationList.
 */
- (NSDictionary *)makeConfigurationListWithBuildConfigurations:(NSArray *)conf
{
    NSMutableDictionary *reference = [NSMutableDictionary dictionary];
    [reference setObject:[NSString stringFromMemberType:XFConfigurationListType]
                  forKey:@"isa"];
    [reference setObject:@"Release" forKey:@"DefaultConfigurationName"];
    [reference setObject:conf forKey:@"buildConfigurations"];
    
    return reference;
}



- (NSDictionary*)asDictionary
{
    NSMutableDictionary* groupData = [NSMutableDictionary dictionary];
    [groupData setObject:[NSString stringFromMemberType:XFPBXGroupType] forKey:@"isa"];
    [groupData setObject:@"<group>" forKey:@"sourceTree"];

    if (_alias != nil)
    {
        [groupData setObject:_alias forKey:@"name"];
    }

    if (_pathRelativeToParent)
    {
        [groupData setObject:_pathRelativeToParent forKey:@"path"];
    }

    if (_children)
    {
        [groupData setObject:_children forKey:@"children"];
    }

    return groupData;
}

- (XcodeMemberType)typeForKey:(NSString*)key
{
    NSDictionary* obj = [[_project objects] valueForKey:key];
    return [[obj valueForKey:@"isa"] asMemberType];
}

- (void)addSourceFile:(XFSourceFile*)sourceFile toTargets:(NSArray*)targets
{
    for (XFTarget* target in targets)
    {
        [target addMember:sourceFile];
    }
}

@end
