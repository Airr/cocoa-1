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
#import "XFProject.h"
#import "XFKeyBuilder.h"
#import "XFGroup.h"

@implementation XFSourceFile

@synthesize type = _type;
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

#pragma mark - Initialization & Destruction

- (id)initWithProject:(XFProject*)project
                  key:(NSString*)key
                 type:(XfastSourceFileType)type
                 name:(NSString*)name
           sourceTree:(NSString*)tree
                 path:(NSString*)path
{

    self = [super init];
    if (self)
    {
        _project = project;
        _key = [key copy];
        _type = type;
        _name = [name copy];
        _sourceTree = [tree copy];
        _path = [path copy];
    }
    return self;
}

#pragma mark - Interface Methods

// Goes to the entry for this object in the project and sets a value for one of the keys, such as name, path, etc.
- (void)setValue:(id)val forProjectItemPropertyWithKey:(NSString*)key
{
    NSMutableDictionary* obj = [[[_project objects] objectForKey:_key] mutableCopy];
    if (nil == obj)
    {
        [NSException raise:@"Project item not found" format:@"Project item with key %@ not found.", _key];
    }
    [obj setValue:val forKey:key];
    [[_project objects] setValue:obj forKey:_key];
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
        [[_project objects] enumerateKeysAndObjectsUsingBlock:^(NSString* key,
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
    return (_type == XFSourceCodeObjC
            || _type == XFSourceCodeObjCPlusPlus
            || _type == XFSourceCodeCPlusPlus
            || _type == XFXibFile
            || _type == XFFramework
            || _type == XFImageResourcePNG
            || _type == XFHTML
            || _type == XFTEXT
            || _type == XFBundle
            || _type == XFArchive);
}


- (XcodeMemberType)buildPhase
{
    if (_type == XFSourceCodeObjC || _type == XFSourceCodeObjCPlusPlus
        || _type == XFSourceCodeCPlusPlus || _type == XFXibFile
        || _type == XFTEXT)
    {
        return XFPBXSourcesBuildPhaseType;
    }
    else if (_type == XFFramework || _type == XFArchive)
    {
        return XFPBXFrameworksBuildPhaseType;
    }
    else if (_type == XFImageResourcePNG || _type == XFHTML || _type == XFBundle)
    {
        return XFPBXResourcesBuildPhaseType;
    }
    return XFPBXNilType;
}

- (NSString*)buildFileKey
{
    if (_buildFileKey == nil)
    {
        [[_project objects] enumerateKeysAndObjectsUsingBlock:
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
            [[_project objects] setObject:sourceBuildFile forKey:buildFileKey];
        }
        else if (_type == XFFramework)
        {
            [NSException raise:NSInvalidArgumentException format:@"Add framework to target not implemented yet."];
        }
        else
        {
            [NSException raise:NSInvalidArgumentException format:@"Project file of type %@ can't become a build file.",
                                                                 NSStringFromXFSourceFileType(_type)];
        }

    }
}

/* ====================================================================================================================================== */
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

/* ====================================================================================================================================== */
#pragma mark - Utility Methods

- (NSString*)description
{
    return [NSString stringWithFormat:@"Project file: key=%@, name=%@, fullPath=%@", _key, _name, [self pathRelativeToProjectRoot]];
}


@end

