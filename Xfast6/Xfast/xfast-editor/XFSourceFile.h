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



#import <Foundation/Foundation.h>
#import "XfastGroupMember.h"
#import "XfastSourceFileType.h"

@class XFProject;

/**
* Represents a file resource in an xcode project.
*/
@interface XFSourceFile : NSObject <XfastGroupMember>
{

@private
    XFProject* _xfproject;

    NSNumber* _isBuildFile;
    NSString* _buildFileKey;
    NSString* _name;
    NSString* _sourceTree;
    NSString* _key;
    NSString* _path;
    XfastSourceFileType _sourceType;
}


@property(nonatomic, readonly) XfastSourceFileType sourceType;
@property(nonatomic, strong, readonly) NSString* key;
@property(nonatomic, strong) NSString* name;
@property(nonatomic, strong, readonly) NSString* sourceTree;
@property(nonatomic, strong) NSString* path;

@property(nonatomic, copy) NSString* user;
@property(nonatomic, copy) NSString* project;
@property(nonatomic, copy) NSString* xfast;
@property(nonatomic, copy) NSString *type;

+ (XFSourceFile*)sourceFileWithProject:(XFProject*)project key:(NSString*)key type:(XfastSourceFileType)type name:(NSString*)name
    sourceTree:(NSString*)tree path:(NSString*)path;

- (id)initWithProject:(XFProject*)project key:(NSString*)key type:(XfastSourceFileType)type name:(NSString*)name sourceTree:(NSString*)tree
    path:(NSString*)path;

+ (XFSourceFile*)sourceFileWithProject:(XFProject*)xfproject
                                   key:(NSString*)key
                            sourceType:(XfastSourceFileType)sourceType
                                  name:(NSString*)name
                            sourceTree:(NSString*)tree
                                  path:(NSString*)path
                                  user:(NSString *)user
                               project:(NSString *)project
                                 xfast:(NSString *)xfast
                                  type:(NSString *)type;


- (id)initWithProject:(XFProject*)xfproject
                  key:(NSString*)key
           sourceType:(XfastSourceFileType)sourceType
                 name:(NSString*)name
           sourceTree:(NSString*)tree
                 path:(NSString *)path
                 user:(NSString *)user
              project:(NSString *)project
                xfast:(NSString *)xfast
                 type:(NSString *)type;

/**
* If yes, indicates the file is able to be included for compilation in an `XFTarget`.
*/
- (BOOL)isBuildFile;

- (BOOL)canBecomeBuildFile;

- (XcodeMemberType)buildPhase;

- (NSString*)buildFileKey;

/**
* Adds this file to the project as an `xcode_BuildFile`, ready to be included in targets.
*/
- (void)becomeBuildFile;

- (void)makeBuildFile;

- (void)print;

@end
