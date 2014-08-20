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

@class XFProject;
@class XFClassDefinition;
@class XFSourceFile;
@class XFXibDefinition;
@class XFFileOperationQueue;
@class XFFrameworkDefinition;
@class XFSourceFileDefinition;
@class XFSubProjectDefinition;


/**
* Represents a group container in an Xfast project. A group can contain members of type `XFSourceFile` or other
* groups.
*/
@interface XFGroup : NSObject <XfastGroupMember>
{

    NSString* _pathRelativeToParent;
    NSString* _key;
    NSString* _alias;


@private
    NSString* _pathRelativeToProjectRoot;
    NSMutableArray* _children;
    NSMutableArray* _members;

    XFFileOperationQueue* _fileOperationQueue; // weak
    XFProject* _project;

}


/**
 * The alias of the group, which can be used to give the group a name other than the last path component.
 *
 * See: [XfastGroupMember displayName]
 */
@property(nonatomic, strong, readonly) NSString* alias;

/**
 * The path of the group relative to the group's parent.
 *
 * See: [XfastGroupMember displayName]
*/
@property(nonatomic, strong, readonly) NSString* pathRelativeToParent;

/**
 * The group's unique key.
*/
@property(nonatomic, strong, readonly) NSString* key;

/**
 * An array containing the groups members as `XfastGroupMember` types.
*/
@property(nonatomic, strong, readonly) NSMutableArray* children;


#pragma mark Initializers

+ (XFGroup*)groupWithProject:(XFProject*)project key:(NSString*)key alias:(NSString*)alias path:(NSString*)path children:(NSArray*)children;

- (id)initWithProject:(XFProject*)project key:(NSString*)key alias:(NSString*)alias path:(NSString*)path children:(NSArray*)children;

#pragma mark Parent group

- (void)removeFromParentGroup;

- (void)removeFromParentDeletingChildren:(BOOL)deleteChildren;

- (XFGroup*)parentGroup;

- (BOOL)isRootGroup;

#pragma mark Adding children
/**
 * Adds a class to the group, as specified by the ClassDefinition. If the group already contains a class by the same
 * name, the contents will be updated.
*/
- (void)addClass:(XFClassDefinition*)classDefinition;

/**
 * Adds a class to the group, making it a member of the specified [targets](XFTarget).
*/
- (void)addClass:(XFClassDefinition*)classDefinition toTargets:(NSArray*)targets;

/**
* Adds a framework to the group. If the group already contains the framework, the contents will be updated if the
* framework definition's copyToDestination flag is yes, otherwise it will be ignored.
*/
- (void)addFramework:(XFFrameworkDefinition*)frameworkDefinition;

/**
* Adds a group with a path relative to this group.
*/
- (XFGroup*)addGroupWithPath:(NSString*)path;

/**
* Adds a framework to the group, making it a member of the specified targets.
*/
- (void)addFramework:(XFFrameworkDefinition*)framework toTargets:(NSArray*)targets;

/**
* Adds a source file of arbitrary type - image resource, header, etc.
*/
- (void)addSourceFile:(XFSourceFileDefinition*)sourceFileDefinition;


/**
 * Adds a xib file to the group. If the group already contains a class by the same name, the contents will be updated.
*/
- (void)addXib:(XFXibDefinition*)xibDefinition;

/**
 * Adds a xib to the group, making it a member of the specified [targets](XFTarget).
*/
- (void)addXib:(XFXibDefinition*)xibDefinition toTargets:(NSArray*)targets;

/**
 * Adds a sub-project to the group. If the group already contains a sub-project by the same name, the contents will be
 * updated.
 * Returns boolean success/fail; if method fails, caller should assume that project file is corrupt (or file format has
 * changed).
*/
- (void)addSubProject:(XFSubProjectDefinition*)projectDefinition;

/**
* Adds a sub-project to the group, making it a member of the specified [targets](XFTarget).
*/
- (void)addSubProject:(XFSubProjectDefinition*)projectDefinition toTargets:(NSArray*)targets;

- (void)removeSubProject:(XFSubProjectDefinition*)projectDefinition;

- (void)removeSubProject:(XFSubProjectDefinition*)projectDefinition fromTargets:(NSArray*)targets;

- (void)addTarget:(XFSourceFileDefinition*)sourceFileDefinition;

- (void)addFile:(XFSourceFileDefinition*)sourceFileDefinition;

#pragma mark Locating children
/**
 * Instances of `XFSourceFile` and `XFGroup` returned as the type `XfastGroupMember`.
*/
- (NSArray*)members;

/**
* Instances of `XFSourceFile` from this group and any child groups.
*/
- (NSArray*)recursiveMembers;


- (NSArray*)buildFileKeys;

/**
 * Returns the child with the specified key, or nil.
*/
- (id <XfastGroupMember>)memberWithKey:(NSString*)key;

/**
* Returns the child with the specified name, or nil.
*/
- (id <XfastGroupMember>)memberWithDisplayName:(NSString*)name;


@end
