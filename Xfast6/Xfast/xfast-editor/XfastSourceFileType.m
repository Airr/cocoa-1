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



#import "XfastSourceFileType.h"

NSDictionary* NSDictionaryWithXFFileReferenceTypes()
{
    static NSDictionary* dictionary;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^
    {
        dictionary = @{
            @"sourcecode.c.h"        : @(XFSourceCodeHeader),
            @"sourcecode.c.objc"     : @(XFSourceCodeObjC),
            @"wrapper.framework"     : @(XFFramework),
            @"text.plist.strings"    : @(XFPropertyList),
            @"sourcecode.cpp.objcpp" : @(XFSourceCodeObjCPlusPlus),
            @"sourcecode.cpp.cpp"    : @(XFSourceCodeCPlusPlus),
            @"file.xib"              : @(XFXibFile),
            @"image.png"             : @(XFImageResourcePNG),
            @"wrapper.cfbundle"      : @(XFBundle),
            @"archive.ar"            : @(XFArchive),
            @"text.html"             : @(XFHTML),
            @"text"                  : @(XFTEXT),
            @"wrapper.target"        : @(XfastTarget),
            @"wrapper.pb-project"    : @(XfastProject)
        };
    });

    return dictionary;
}

NSString* NSStringFromXFSourceFileType(XfastSourceFileType type)
{
    return [[NSDictionaryWithXFFileReferenceTypes() allKeysForObject:@(type)] objectAtIndex:0];
}

XfastSourceFileType XFSourceFileTypeFromStringRepresentation(NSString* string)
{
    NSDictionary* typeStrings = NSDictionaryWithXFFileReferenceTypes();

    if ([typeStrings objectForKey:string])
    {
        return (XfastSourceFileType) [[typeStrings objectForKey:string] intValue];
    }
    else
    {
        return XFFileTypeNil;
    }
}


XfastSourceFileType XFSourceFileTypeFromFileName(NSString* fileName)
{
    if ([fileName hasSuffix:@".h"] || [fileName hasSuffix:@".hh"]
        || [fileName hasSuffix:@".hpp"] || [fileName hasSuffix:@".hxx"])
    {
        return XFSourceCodeHeader;
    }
    if ([fileName hasSuffix:@".c"] || [fileName hasSuffix:@".m"])
    {
        return XFSourceCodeObjC;
    }
    if ([fileName hasSuffix:@".mm"])
    {
        return XFSourceCodeObjCPlusPlus;
    }
    if ([fileName hasSuffix:@".cpp"])
    {
        return XFSourceCodeCPlusPlus;
    }
    return XFFileTypeNil;
}

