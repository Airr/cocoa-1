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
    XFFileTypeNil,             // Unknown filetype 
    XFFramework,               // .framework
    XFPropertyList,            // .plist 
    XFSourceCodeHeader,        // .h     
    XFSourceCodeObjC,          // .m     
    XFSourceCodeObjCPlusPlus,  // .mm
    XFSourceCodeCPlusPlus,     // .cpp
    XFXibFile,                 // .xib   
    XFImageResourcePNG,        // .png
    XFBundle,                  // .bundle  .octet
    XFArchive,                 // .a files
    XFHTML,                    // HTML file
    XFTEXT,                    // Some text file
    XfastTarget,               // .target
    XfastProject               // .xcodeproj
} XfastSourceFileType;

NSString* NSStringFromXFSourceFileType(XfastSourceFileType type);

XfastSourceFileType XFSourceFileTypeFromStringRepresentation(NSString* string);

XfastSourceFileType XFSourceFileTypeFromFileName(NSString* fileName);

