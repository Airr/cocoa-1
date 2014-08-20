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



#import "XcodeMemberType.h"

@protocol XfastGroupMember <NSObject>

- (NSString*)key;

- (NSString*)displayName;

- (NSImage *)displayImage;

- (NSString*)pathRelativeToProjectRoot;

- (NSString*)absolutePath;

/**
* Group members can either be other groups (XFPBXGroup) or source files (XFPBXFileReference).
*/
- (XcodeMemberType)groupMemberType;
@end
