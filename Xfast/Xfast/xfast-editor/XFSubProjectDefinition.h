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
#import "XFAbstractDefinition.h"
#import "XfastSourceFileType.h"

@class XFProject;


@interface XFSubProjectDefinition : XFAbstractDefinition
{

    NSString* _name;
    NSString* _path;
    XfastSourceFileType _type;
    XFProject* _subProject;
    XFProject* _parentProject;
    NSString* _key;
    NSString* _fullProjectPath;
    NSString* _relativePath;
}


@property(nonatomic, strong, readonly) NSString* name;
@property(nonatomic, strong, readonly) NSString* path;
@property(nonatomic, readonly) XfastSourceFileType type;
@property(nonatomic, strong, readonly) XFProject* subProject;
@property(nonatomic, strong, readonly) XFProject* parentProject;
@property(nonatomic, strong, readonly) NSString* key;
@property(nonatomic, strong, readwrite) NSString* fullProjectPath;

+ (XFSubProjectDefinition*)withName:(NSString*)name path:(NSString*)path parentProject:(XFProject*)parentProject;

- (id)initWithName:(NSString*)name path:(NSString*)path parentProject:(XFProject*)parentProject;

- (NSString*)projectFileName;

- (NSString*)fullPathName;

- (NSArray*)buildProductNames;

- (NSString*)projectKey;

- (NSString*)pathRelativeToProjectRoot;

- (NSString*)description;

- (void)initFullProjectPath:(NSString*)fullProjectPath groupPath:(NSString*)groupPath;

@end
