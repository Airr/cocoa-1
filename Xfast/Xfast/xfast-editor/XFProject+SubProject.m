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



#import "XFSourceFile.h"
#import "XFTarget.h"
#import "XFKeyBuilder.h"
#import "XFProject+SubProject.h"
#import "XFSubProjectDefinition.h"


@implementation XFProject (SubProject)


#pragma mark sub-project related public methods

// returns the key for the reference proxy with the given path (nil if not found)
// does not use keysForProjectObjectsOfType:withIdentifier: because the identifier it uses for
// XFPBXReferenceProxy is different.
- (NSString*)referenceProxyKeyForName:(NSString*)name
{
    __block NSString* result = nil;
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXReferenceProxyType)
        {
            if ([[obj valueForKey:@"path"] isEqualTo:name])
            {
                result = key;
                *stop = YES;
            }
        }
    }];
    return result;
}

// returns an array of build products, excluding bundles with extensions other than ".bundle" (which is kind
// of gross, but I didn't see a better way to exclude test bundles without giving them their own XfastSourceFileType)
- (NSArray*)buildProductsForTargets:(NSString*)xcodeprojKey
{
    NSMutableArray* results = [[NSMutableArray alloc] init];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXReferenceProxyType)
        {
            // make sure it belongs to the xcodeproj we're adding
            NSString* remoteRef = [obj valueForKey:@"remoteRef"];
            NSDictionary* containerProxy = [[self objects] valueForKey:remoteRef];
            NSString* containerPortal = [containerProxy valueForKey:@"containerPortal"];
            if ([containerPortal isEqualToString:xcodeprojKey])
            {
                XfastSourceFileType type = XFSourceFileTypeFromStringRepresentation([obj valueForKey:@"fileType"]);
                NSString* path = (NSString*) [obj valueForKey:@"path"];
                if (type != XFBundle || [[path pathExtension] isEqualToString:@"bundle"])
                {
                    [results addObject:[XFSourceFile sourceFileWithProject:self key:key type:type name:path sourceTree:nil path:nil]];
                }
            }
        }
    }];
    return results;
}

// makes XFPBXFontainerItemProxy and XFPBXTargetDependency objects for the xcodeproj, and adds the dependency key
// to all the specified targets
- (void)addAsTargetDependency:(XFSubProjectDefinition*)xcodeprojDefinition toTargets:(NSArray*)targets
{
    for (XFTarget* target in targets)
    {
        // make a new XFPBXFontainerItemProxy
        NSString* key = [[self fileWithName:[xcodeprojDefinition pathRelativeToProjectRoot]] key];
        NSString* containerItemProxyKey =
            [self makeContainerItemProxyForName:[xcodeprojDefinition name] fileRef:key proxyType:@"1" uniqueName:[target name]];
        // make a XFPBXTargetDependency
        NSString* targetDependencyKey =
            [self makeTargetDependency:[xcodeprojDefinition name] forContainerItemProxyKey:containerItemProxyKey uniqueName:[target name]];
        // add entry in each targets dependencies list
        [target addDependency:targetDependencyKey];
    }
}

// returns an array of keys for all project objects (not just files) that match the given criteria.  Since this is
// a convenience method intended to save typing elsewhere, each type has its own field to match to rather than each
// matching on name or path as you might expect.
- (NSArray*)keysForProjectObjectsOfType:(XcodeMemberType)memberType withIdentifier:(NSString*)identifier singleton:(BOOL)singleton
    required:(BOOL)required
{
    __block NSMutableArray* returnValue = [[NSMutableArray alloc] init];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == memberType)
        {
            if (memberType == XFPBXFontainerItemProxyType)
            {
                if ([[obj valueForKey:@"containerPortal"] isEqualToString:identifier])
                {
                    [returnValue addObject:key];
                }
            }
            else if (memberType == XFPBXReferenceProxyType)
            {
                if ([[obj valueForKey:@"remoteRef"] isEqualToString:identifier])
                {
                    [returnValue addObject:key];
                }
            }
            else if (memberType == XFPBXTargetDependencyType || memberType == XFPBXGroupType || memberType == XFPBXVariantGroupType)
            {
                if ([[obj valueForKey:@"name"] isEqualToString:identifier])
                {
                    [returnValue addObject:key];
                }
            }
            else if (memberType == XFPBXNativeTargetType)
            {
                for (NSString* dependencyKey in [obj valueForKey:@"dependencies"])
                {
                    if ([dependencyKey isEqualToString:identifier])
                    {
                        [returnValue addObject:key];
                    }
                }
            }
            else if (memberType == XFPBXBuildFileType)
            {
                if ([[obj valueForKey:@"fileRef"] isEqualToString:identifier])
                {
                    [returnValue addObject:key];
                }
            }
            else if (memberType == XFPBXProjectType)
            {
                [returnValue addObject:key];
            }
            else if (memberType == XFPBXFileReferenceType)
            {
                if ([[obj valueForKey:@"path"] isEqualToString:identifier])
                {
                    [returnValue addObject:key];
                }
            }
            else if (memberType == XFPBXFrameworksBuildPhaseType || memberType == XFPBXResourcesBuildPhaseType)
            {
                [returnValue addObject:key];
            }
            else
            {
                [NSException raise:NSInvalidArgumentException format:@"Unrecognized member type %@",
                                                                     [NSString stringFromMemberType:memberType]];
            }
        }
    }];
    if (singleton && [returnValue count] > 1)
    {
        [NSException raise:NSGenericException format:@"Searched for one instance of member type %@ with value %@, but found %ld",
                                                     [NSString stringFromMemberType:memberType], identifier, [returnValue count]];
    }
    if (required && [returnValue count] == 0)
    {
        [NSException raise:NSGenericException format:@"Searched for instances of member type %@ with value %@, but did not find any",
                                                     [NSString stringFromMemberType:memberType], identifier];
    }
    return returnValue;
}

// returns the dictionary for the XFPBXProject.  Raises an exception if more or less than 1 are found.
- (NSMutableDictionary*)XFPBXProjectDict
{
    NSString* XFPBXProjectKey;
    NSArray* XFPBXProjectKeys = [self keysForProjectObjectsOfType:XFPBXProjectType withIdentifier:nil singleton:YES required:YES];
    XFPBXProjectKey = [XFPBXProjectKeys objectAtIndex:0];
    NSMutableDictionary* XFPBXProjectDict = [[self objects] valueForKey:XFPBXProjectKey];
    return XFPBXProjectDict;
}

// returns the key of the XFPBXFontainerItemProxy for the given name and proxy type. nil if not found.
- (NSString*)containerItemProxyKeyForName:(NSString*)name proxyType:(NSString*)proxyType
{
    NSMutableArray* results = [[NSMutableArray alloc] init];
    [[self objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key, NSDictionary* obj, BOOL* stop)
    {
        if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXFontainerItemProxyType)
        {
            NSString* remoteInfo = [obj valueForKey:@"remoteInfo"];
            NSString* proxy = [obj valueForKey:@"proxyType"];
            if ([remoteInfo isEqualToString:name] && [proxy isEqualToString:proxyType])
            {
                [results addObject:key];
            }
        }
    }];
    if ([results count] > 1)
    {
        [NSException raise:NSGenericException format:@"Searched for one instance of member type %@ with value %@, but found %ld",
                                                     @"XFPBXFontainerItemProxy",
                                                     [NSString stringWithFormat:@"%@ and proxyType of %@", name, proxyType],
                                                     [results count]];
    }
    if ([results count] == 0)
    {
        return nil;
    }
    return [results objectAtIndex:0];
}

/* ====================================================================================================================================== */
#pragma mark - Private Methods
#pragma mark sub-project related private methods

// makes a XFPBXFontainerItemProxy object for a given XFPBXFileReference object.  Replaces pre-existing objects.
- (NSString*)makeContainerItemProxyForName:(NSString*)name fileRef:(NSString*)fileRef proxyType:(NSString*)proxyType
    uniqueName:(NSString*)uniqueName
{
    NSString* keyName;
    if (uniqueName != nil)
    {
        keyName = [NSString stringWithFormat:@"%@-%@", name, uniqueName];
    }
    else
    {
        keyName = name;
    }
    // remove old if it exists
    NSString* existingProxyKey = [self containerItemProxyKeyForName:keyName proxyType:proxyType];
    if (existingProxyKey)
    {
        [[self objects] removeObjectForKey:existingProxyKey];
    }
    // make new one
    NSMutableDictionary* proxy = [NSMutableDictionary dictionary];
    [proxy setObject:[NSString stringFromMemberType:XFPBXFontainerItemProxyType] forKey:@"isa"];
    [proxy setObject:fileRef forKey:@"containerPortal"];
    [proxy setObject:proxyType forKey:@"proxyType"];
    // give it a random key - the keys xcode puts here are not in the project file anywhere else
    NSString* key = [[XFKeyBuilder forItemNamed:[NSString stringWithFormat:@"%@-junk", keyName]] build];
    [proxy setObject:key forKey:@"remoteGlobalIDString"];
    [proxy setObject:name forKey:@"remoteInfo"];
    // add to project. use proxyType to generate key, so that multiple keys for the same name don't overwrite each other
    key = [[XFKeyBuilder forItemNamed:[NSString stringWithFormat:@"%@-containerProxy-%@", keyName, proxyType]] build];
    [[self objects] setObject:proxy forKey:key];

    return key;
}

// makes a XFPBXReferenceProxy object for a given XFPBXFontainerProxy object.  Replaces pre-existing objects.
- (void)makeReferenceProxyForContainerItemProxy:(NSString*)containerItemProxyKey buildProductReference:(NSDictionary*)buildProductReference
{
    NSString* path = [buildProductReference valueForKey:@"path"];
    // remove old if any exists
    NSArray* existingProxyKeys = [self keysForProjectObjectsOfType:XFPBXReferenceProxyType withIdentifier:path singleton:NO required:NO];
    if ([existingProxyKeys count] > 0)
    {
        for (NSString* existingProxyKey in existingProxyKeys)
        {
            [[self objects] removeObjectForKey:existingProxyKey];
        }
    }
    // make new one
    NSMutableDictionary* proxy = [NSMutableDictionary dictionary];
    [proxy setObject:[NSString stringFromMemberType:XFPBXReferenceProxyType] forKey:@"isa"];
    [proxy setObject:[buildProductReference valueForKey:@"explicitFileType"] forKey:@"fileType"];
    [proxy setObject:path forKey:@"path"];
    [proxy setObject:containerItemProxyKey forKey:@"remoteRef"];
    [proxy setObject:[buildProductReference valueForKey:@"sourceTree"] forKey:@"sourceTree"];
    // add to project
    NSString* key = [[XFKeyBuilder forItemNamed:[NSString stringWithFormat:@"%@-referenceProxy", path]] build];
    [[self objects] setObject:proxy forKey:key];
}

// makes a XFPBXTargetDependency object for a given XFPBXFontainerItemProxy.  Replaces pre-existing objects.
- (NSString*)makeTargetDependency:(NSString*)name forContainerItemProxyKey:(NSString*)containerItemProxyKey uniqueName:(NSString*)uniqueName
{
    NSString* keyName;
    if (uniqueName != nil)
    {
        keyName = [NSString stringWithFormat:@"%@-%@", name, uniqueName];
    }
    else
    {
        keyName = name;
    }
    // remove old if it exists
    NSArray* existingDependencyKeys =
        [self keysForProjectObjectsOfType:XFPBXTargetDependencyType withIdentifier:keyName singleton:NO required:NO];
    if ([existingDependencyKeys count] > 0)
    {
        for (NSString* existingDependencyKey in existingDependencyKeys)
        {
            [[self objects] removeObjectForKey:existingDependencyKey];
        }
    }
    // make new one
    NSMutableDictionary* targetDependency = [NSMutableDictionary dictionary];
    [targetDependency setObject:[NSString stringFromMemberType:XFPBXTargetDependencyType] forKey:@"isa"];
    [targetDependency setObject:name forKey:@"name"];
    [targetDependency setObject:containerItemProxyKey forKey:@"targetProxy"];
    NSString* targetDependencyKey = [[XFKeyBuilder forItemNamed:[NSString stringWithFormat:@"%@-targetProxy", keyName]] build];
    [[self objects] setObject:targetDependency forKey:targetDependencyKey];
    return targetDependencyKey;
}

// make a XFPBXFontainerItemProxy and XFPBXReferenceProxy for each target in the subProject
- (void)addProxies:(XFSubProjectDefinition*)xcodeproj
{
    NSString* fileRef = [[self fileWithName:[xcodeproj pathRelativeToProjectRoot]] key];
    for (NSDictionary* target in [xcodeproj.subProject targets])
    {
        NSString* containerItemProxyKey =
            [self makeContainerItemProxyForName:[target valueForKey:@"name"] fileRef:fileRef proxyType:@"2" uniqueName:nil];
        NSString* productFileReferenceKey = [target valueForKey:@"productReference"];
        NSDictionary* productFileReference = [[xcodeproj.subProject objects] valueForKey:productFileReferenceKey];
        [self makeReferenceProxyForContainerItemProxy:containerItemProxyKey buildProductReference:productFileReference];
    }
}

// remove the XFPBXFontainerItemProxy and XFPBXReferenceProxy objects for the given object key (which is the XFPBXFilereference
// for the xcodeproj file)
- (void)removeProxies:(NSString*)xcodeprojKey
{
    NSMutableArray* keysToDelete = [NSMutableArray array];
    // use the xcodeproj's XFPBXFileReference key to get the XFPBXFontainerItemProxy keys
    NSArray* containerItemProxyKeys =
        [self keysForProjectObjectsOfType:XFPBXFontainerItemProxyType withIdentifier:xcodeprojKey singleton:NO required:YES];
    // use the XFPBXFontainerItemProxy keys to get the XFPBXReferenceProxy keys
    for (NSString* key in containerItemProxyKeys)
    {
        [keysToDelete addObjectsFromArray:[self keysForProjectObjectsOfType:XFPBXReferenceProxyType withIdentifier:key singleton:NO
            required:NO]];
        [keysToDelete addObject:key];
    }
    // remove all objects located above
    [keysToDelete enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop)
    {
        [[self objects] removeObjectForKey:obj];
    }];
}

// returns the Products group key for the given XFPBXFileReference key, nil if not found.
- (NSString*)productsGroupKeyForKey:(NSString*)key
{
    NSMutableArray* projectReferences = [[self XFPBXProjectDict] valueForKey:@"projectReferences"];
    NSString* productsGroupKey = nil;
    for (NSDictionary* projectRef in projectReferences)
    {
        if ([[projectRef valueForKey:@"ProjectRef"] isEqualToString:key])
        {
            // it's an error if we find more than one
            if (productsGroupKey != nil)
            {
                [NSException raise:NSGenericException format:@"Found more than one project reference for key %@", key];
            }
            productsGroupKey = [projectRef valueForKey:@"ProductGroup"];
        }
    }
    return productsGroupKey;
}

// removes a file reference from the projectReferences array in XFPBXProject (removing the array itself if this action
// leaves it empty).
- (void)removeFromProjectReferences:(NSString*)key forProductsGroup:(NSString*)productsGroupKey
{
    NSMutableArray* projectReferences = [[self XFPBXProjectDict] valueForKey:@"projectReferences"];
    // remove entry from XFPBXProject's projectReferences
    NSMutableArray* referencesToRemove = [NSMutableArray array];
    for (NSDictionary* projectRef in projectReferences)
    {
        if ([[projectRef valueForKey:@"ProjectRef"] isEqualToString:key])
        {
            [referencesToRemove addObject:projectRef];
        }
    }
    for (NSDictionary* projectRef in referencesToRemove)
    {
        [projectReferences removeObject:projectRef];
    }
    // if that was the last project reference, remove the array from the project
    if ([projectReferences count] == 0)
    {
        [[self XFPBXProjectDict] removeObjectForKey:@"projectReferences"];
    }
}

// removes a specific xcodeproj file from any targets (by name).  It's not an error if no entries are found,
// because we support adding a project file without adding it to any targets.
- (void)removeTargetDependencies:(NSString*)name
{
    // get the key for the XFPBXTargetDependency with name = xcodeproj file name (without extension)
    NSArray* targetDependencyKeys = [self keysForProjectObjectsOfType:XFPBXTargetDependencyType withIdentifier:name singleton:NO required:NO];
    // we might not find any if the project wasn't added to targets in the first place
    if ([targetDependencyKeys count] == 0)
    {
        return;
    }
    NSString* targetDependencyKey = [targetDependencyKeys objectAtIndex:0];
    // use the key for the XFPBXTargetDependency to get the key for any XFPBXNativeTargets that depend on it
    NSArray* nativeTargetKeys =
        [self keysForProjectObjectsOfType:XFPBXNativeTargetType withIdentifier:targetDependencyKey singleton:NO required:NO];
    // remove the key for the XFPBXTargetDependency from the XFPBXNativeTarget's dependencies arrays (leave in place even if empty)
    for (NSString* nativeTargetKey in nativeTargetKeys)
    {
        NSMutableDictionary* nativeTarget = [[self objects] objectForKey:nativeTargetKey];
        NSMutableArray* dependencies = [nativeTarget valueForKey:@"dependencies"];
        [dependencies removeObject:targetDependencyKey];
        [nativeTarget setObject:dependencies forKey:@"dependencies"];
    }
    // remove the XFPBXTargetDependency
    [[self objects] removeObjectForKey:targetDependencyKey];
}

@end
