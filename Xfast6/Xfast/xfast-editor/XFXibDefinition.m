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


#import "XFXibDefinition.h"

@implementation XFXibDefinition

@synthesize name = _name;
@synthesize content = _content;

/* ====================================================================================================================================== */
#pragma mark - Class Methods

+ (XFXibDefinition*)xibDefinitionWithName:(NSString*)name
{
    return [[XFXibDefinition alloc] initWithName:name];
}

+ (XFXibDefinition*)xibDefinitionWithName:(NSString*)name content:(NSString*)content
{
    return [[XFXibDefinition alloc] initWithName:name content:content];
}


/* ====================================================================================================================================== */
#pragma mark - Initialization & Destruction

- (id)initWithName:(NSString*)name
{
    return [self initWithName:name content:nil];
}


- (id)initWithName:(NSString*)name content:(NSString*)content
{
    self = [super init];
    if (self)
    {
        _name = [name copy];
        _content = [content copy];
    }
    return self;
}


/* ====================================================================================================================================== */
#pragma mark - Interface Methods

- (NSString*)xibFileName
{
    return [_name stringByAppendingString:@".xib"];
}

@end
