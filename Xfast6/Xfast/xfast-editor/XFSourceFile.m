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
#import "XFSourceFile.h"
#import "XFProject.h"
#import "XFKeyBuilder.h"
#import "XFGroup.h"

@implementation XFSourceFile

@synthesize sourceType = _sourceType;
@synthesize key = _key;
@synthesize sourceTree = _sourceTree;

#pragma mark - Class Methods

+ (XFSourceFile*)sourceFileWithProject:(XFProject*)project
                                   key:(NSString*)key
                                  type:(XfastSourceFileType)type
                                  name:(NSString*)name
                            sourceTree:(NSString*)_tree
                                  path:(NSString*)path
{
    return [[XFSourceFile alloc] initWithProject:project
                                             key:key
                                            type:type
                                            name:name
                                      sourceTree:_tree
                                            path:path];
}

+ (XFSourceFile*)sourceFileWithProject:(XFProject*)xfproject
                                   key:(NSString*)key
                            sourceType:(XfastSourceFileType)sourceType
                                  name:(NSString*)name
                            sourceTree:(NSString*)tree
                                  path:(NSString*)path
                                  user:(NSString *)user
                               project:(NSString *)project
                                 xfast:(NSString *)xfast
                                  type:(NSString *)type
{
    return [[XFSourceFile alloc] initWithProject:xfproject
                                             key:key
                                      sourceType:sourceType
                                            name:name
                                      sourceTree:tree
                                            path:path
                                            user:user
                                         project:project
                                           xfast:xfast
                                            type:type];
}

#pragma mark - Initialization & Destruction

- (id)initWithProject:(XFProject*)xfproject
                  key:(NSString*)key
                 type:(XfastSourceFileType)type
                 name:(NSString*)name
           sourceTree:(NSString*)tree
                 path:(NSString*)path
{

    self = [super init];
    if (self)
    {
        _xfproject = xfproject;
        _key = [key copy];
        _sourceType = type;
        _name = [name copy];
        _sourceTree = [tree copy];
        _path = [path copy];
    }
    return self;
}

- (id)initWithProject:(XFProject*)xfproject
                  key:(NSString*)key
           sourceType:(XfastSourceFileType)sourceType
                 name:(NSString*)name

           sourceTree:(NSString*)tree
                 path:(NSString *)path
                 user:(NSString *)user
              project:(NSString *)project
                xfast:(NSString *)xfast
                 type:(NSString *)type
{

    self = [super init];
    if (self)
    {
        _xfproject = xfproject;
        _key = [key copy];
        _sourceType = sourceType;
        _name = [name copy];
        _sourceTree = [tree copy];
        _path = [path copy];
        [self setUser:user];
        [self setProject:project];
        [self setXfast:xfast];
        [self setType:type];
    }
    return self;
}

#pragma mark - Interface Methods

// Goes to the entry for this object in the project and sets a value for one of the keys, such as name, path, etc.
- (void)setValue:(id)val forProjectItemPropertyWithKey:(NSString*)key
{
    NSMutableDictionary* obj = [[[_xfproject objects] objectForKey:_key] mutableCopy];
    if (nil == obj)
    {
        [NSException raise:@"Project item not found" format:@"Project item with key %@ not found.", _key];
    }
    [obj setValue:val forKey:key];
    [[_xfproject objects] setValue:obj forKey:_key];
}


- (NSString*)name
{
    return _name;
}

- (void)setName:(NSString*)name
{
    id __unused old = _name;
    _name = [name copy];

    [self setValue:name forProjectItemPropertyWithKey:@"name"];
}


- (NSString*)path
{
    return _path;
}

- (void)setPath:(NSString*)path
{
    id __unused old = _path;
    _path = [path copy];

    [self setValue:path forProjectItemPropertyWithKey:@"path"];
}

/**
 * Returns YES if the current file is a part of build files.
 */
- (BOOL)isBuildFile
{
    if ([self canBecomeBuildFile] && _isBuildFile == nil)
    {
        id __unused old = _isBuildFile;
        _isBuildFile = [[NSNumber numberWithBool:NO] copy];
        [[_xfproject objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key,
                                                                NSDictionary* obj,
                                                                BOOL* stop)
        {
            if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXBuildFileType)
            {
                if ([[obj valueForKey:@"fileRef"] isEqualToString:_key])
                {
                    _isBuildFile = nil;
                    _isBuildFile = [[NSNumber numberWithBool:YES] copy];
                    *stop = YES;
                }
            }
        }];
    }
    return [_isBuildFile boolValue];
}

/**
 * Returns YES if the file type is one can be processed by a build. There are
 * much more types of files that can be processed by Xfast. These are sequence
 * files and others.
 */
- (BOOL)canBecomeBuildFile
{
    return YES;
//    return (_sourceType == XFSourceCodeObjC
//            || _type == XFSourceCodeObjCPlusPlus
//            || _type == XFSourceCodeCPlusPlus
//            || _type == XFXibFile
//            || _type == XFFramework
//            || _type == XFImageResourcePNG
//            || _type == XFHTML
//            || _type == XFTEXT
//            || _type == XFBundle
//            || _type == XFArchive);
}


- (XcodeMemberType)buildPhase
{
    return XFPBXSourcesBuildPhaseType;
//    if (_type == XFSourceCodeObjC || _type == XFSourceCodeObjCPlusPlus
//        || _type == XFSourceCodeCPlusPlus || _type == XFXibFile
//        || _type == XFTEXT)
//    {
//        return XFPBXSourcesBuildPhaseType;
//    }
//    else if (_type == XFFramework || _type == XFArchive)
//    {
//        return XFPBXFrameworksBuildPhaseType;
//    }
//    else if (_type == XFImageResourcePNG || _type == XFHTML || _type == XFBundle)
//    {
//        return XFPBXResourcesBuildPhaseType;
//    }
//    return XFPBXNilType;
}

- (NSString*)buildFileKey
{
    if (_buildFileKey == nil)
    {
        [[_xfproject objects] enumerateKeysAndObjectsUsingBlock:
         ^(NSString* key, NSDictionary* obj, BOOL* stop)
        {
            if ([[obj valueForKey:@"isa"] asMemberType] == XFPBXBuildFileType)
            {
                if ([[obj valueForKey:@"fileRef"] isEqualToString:_key])
                {
                    _buildFileKey = [key copy];
                }
            }
        }];
    }
    return [_buildFileKey copy];

}

/**
 * Adapt the current file to a build file.
 * It checks if the current file is a build file. If not, add a build file for
 * the current file. This current file must have been added to the project file
 * as a file reference.
 */
- (void)becomeBuildFile
{
    if (![self isBuildFile])
    {
        if ([self canBecomeBuildFile])
        {
            NSMutableDictionary* sourceBuildFile = [NSMutableDictionary dictionary];
            [sourceBuildFile setObject:[NSString stringFromMemberType:XFPBXBuildFileType] forKey:@"isa"];
            [sourceBuildFile setObject:_key forKey:@"fileRef"];
            NSString* buildFileKey = [[XFKeyBuilder forItemNamed:[_name stringByAppendingString:@".buildFile"]] build];
            [[_xfproject objects] setObject:sourceBuildFile forKey:buildFileKey];
        }
        else if (_sourceType == XFFramework)
        {
            [NSException raise:NSInvalidArgumentException format:@"Add framework to target not implemented yet."];
        }
        else
        {
            [NSException raise:NSInvalidArgumentException format:@"Project file of type %@ can't become a build file.",
                                                                 NSStringFromXFSourceFileType(_sourceType)];
        }

    }
}

/**
 *  Makes the source file a build file that can be part of a target.
 */
- (void)makeBuildFile
{
    if (![self isBuildFile])
    {
        NSMutableDictionary* sourceBuildFile = [NSMutableDictionary dictionary];
        [sourceBuildFile setObject:[NSString stringFromMemberType:XFPBXBuildFileType] forKey:@"isa"];
        [sourceBuildFile setObject:_key forKey:@"fileRef"];
        NSString *buildFileKey = [[XFKeyBuilder createUnique] build];
        
//        
//        NSString *path = [self absolutePath];
//        NSString* buildFileKey = [[XFKeyBuilder forItemNamed:[path stringByAppendingString:@".buildFile"]] build];

        [[_xfproject objects] setObject:sourceBuildFile forKey:buildFileKey];
    }
}

#pragma mark - Protocol Methods

- (XcodeMemberType)groupMemberType
{
    return XFPBXFileReferenceType;
}

- (NSImage*)displayImage
{
    return [NSImage imageNamed:NSImageNameBookmarksTemplate];
}

- (NSString*)displayName
{
    return _name;
}

- (NSString*)pathRelativeToProjectRoot
{
    NSString* parentPath = [[_xfproject groupForGroupMemberWithKey:_key] pathRelativeToProjectRoot];
    NSString* result = [parentPath stringByAppendingPathComponent:_name];
    return result;
}



/**
 * Returns the full path to the file.
 */
- (NSString *)absolutePath
{
    // 1. database file
    
    
    // 2. xfast file
    
    // 3. absolute path file
    
    // Others.
    
    // 1. Find the file reference.
    
    return [_xfproject pathToFile:[self path]
                         withUser:[self user]
                      withProject:[self project]
                        withXfast:[self xfast]
                       withSource:[self sourceTree]];
//    
//    
//    NSDictionary *fileReference = [[_xfproject objects] objectForKey:_key];
//    if (fileReference) {
//        if ([(NSString *)[fileReference valueForKey:@"sourceTree"] compare:@"<absolute>"] == NSOrderedSame) {
//            return [fileReference valueForKey:@"path"];
//        } else {
//            return nil;
//        }
//    } else {
//        return nil;
//    }
}

/* ====================================================================================================================================== */
#pragma mark - Utility Methods

- (NSString*)description
{
    return [NSString stringWithFormat:@"Project file: key=%@, name=%@, fullPath=%@", _key, _name, [self pathRelativeToProjectRoot]];
}

/**
 *  Prints the content of a file.
 */
- (void)print
{
    NSLog(@" - file Bgn - ");
    NSLog(@" path   : %@", [self path]);
    NSLog(@" key    : %@", [self key]);
    NSLog(@" source : %@", [self sourceTree]);
    NSLog(@" user   : %@", [self user]);
    NSLog(@" project: %@", [self project]);
    NSLog(@" xfast  : %@", [self xfast]);
    NSLog(@" type   : %@", [self type]);
    
    NSLog(@" - file End - ");
    
}

@end

