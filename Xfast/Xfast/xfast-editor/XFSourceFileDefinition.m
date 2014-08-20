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



#import "XFSourceFileDefinition.h"

@implementation XFSourceFileDefinition

@synthesize sourceFileName = _sourceFileName;
@synthesize type = _type;
@synthesize data = _data;

#pragma mark - Class Methods

+ (XFSourceFileDefinition*)sourceDefinitionWithName:(NSString*)name
                                               text:(NSString*)text
                                               type:(XfastSourceFileType)type
{

    return [[XFSourceFileDefinition alloc] initWithName:name
                                                   text:text
                                                   type:type];
}

+ (XFSourceFileDefinition*)sourceDefinitionWithName:(NSString*)name
                                               data:(NSData*)data
                                               type:(XfastSourceFileType)type
{

    return [[XFSourceFileDefinition alloc] initWithName:name
                                                   data:data
                                                   type:type];
}


#pragma mark - Initialization & Destruction

- (id)initWithName:(NSString*)name
              text:(NSString*)text
              type:(XfastSourceFileType)type
{
    self = [super init];
    if (self)
    {
        _sourceFileName = [name copy];
        _data = [[text dataUsingEncoding:NSUTF8StringEncoding] copy];
        _type = type;
    }
    return self;
}

- (id)initWithName:(NSString*)name
              data:(NSData*)data
              type:(XfastSourceFileType)type
{
    self = [super init];
    if (self)
    {
        _sourceFileName = [name copy];
        _data = [data copy];
        _type = type;
    }
    return self;

}

@end
