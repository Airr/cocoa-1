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



@class XFProject;

@interface XFBuildConfiguration : NSObject
{
@private
    __weak XFProject* _project;
    NSString* _key;

    NSMutableDictionary* _buildSettings;
    NSMutableDictionary* _xcconfigSettings;
}

@property(nonatomic, readonly) NSDictionary* specifiedBuildSettings;

/**
 * Creates a list of configurations using keys.
 */
+ (NSDictionary*)buildConfigurationsFromArray:(NSArray*)array inProject:(XFProject*)project;

- (instancetype)initWithProject:(XFProject*)project key:(NSString*)key;

/**
 * Add build settings to the build configuration
 */
- (void)addBuildSettings:(NSDictionary*)buildSettings;

/**
 * Add or replace a new setting. This also changes the project.
 */
- (void)addOrReplaceSetting:(id <NSCopying>)setting forKey:(NSString*)key;

/**
 * Gets the setting with a name. It tries to get one in the build setting. 
 * If it does not find one, it tries to search the Xcode setting.
 */
- (id <NSCopying>)valueForKey:(NSString*)key;

+ (NSString*)duplicatedBuildConfigurationListWithKey:(NSString*)buildConfigurationListKey inProject:(XFProject*)project
    withBuildConfigurationVisitor:(void (^)(NSMutableDictionary*))buildConfigurationVisitor;

- (void)print;

@end
