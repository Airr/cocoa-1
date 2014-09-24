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



typedef enum
{
    PBXNilType,
    PBXBuildFileType,
    PBXContainerItemProxyType,
    PBXCopyFilesBuildPhaseType,
    PBXFileReferenceType,
    PBXOutFileReferenceType,
    PBXFrameworksBuildPhaseType,
    PBXGroupType,
    PBXNativeTargetType,
    PBXProjectType,
    PBXReferenceProxyType,
    PBXResourcesBuildPhaseType,
    PBXSourcesBuildPhaseType,
    PBXTargetDependencyType,
    PBXVariantGroupType,
    XCBuildConfigurationType,
    XCConfigurationListType,
    
    XFPBXNilType,
    XFPBXBuildFileType,
    XFPBXFontainerItemProxyType,
    XFPBXFopyFilesBuildPhaseType,
    XFPBXFileReferenceType,
    XFPBXOutFileReferenceType,
    XFPBXFrameworksBuildPhaseType,
    XFPBXGroupType,
    XFPBXNativeTargetType,
    XFPBXProjectType,
    XFPBXReferenceProxyType,
    XFPBXResourcesBuildPhaseType,
    XFPBXSourcesBuildPhaseType,
    XFPBXOutputBuildPhaseType,
    XFPBXTargetDependencyType,
    XFPBXVariantGroupType,
    XFBuildConfigurationType,
    XFConfigurationListType
} XcodeMemberType;

@interface NSString (XcodeMemberTypeExtensions)

+ (NSString*)stringFromMemberType:(XcodeMemberType)nodeType;

- (XcodeMemberType)asMemberType;

@end


