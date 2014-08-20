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

@interface XFSourceFileDefinition : XFAbstractDefinition
{

    NSString* _sourceFileName;
    XfastSourceFileType _type;
    NSData* _data;

}

@property(nonatomic, strong, readonly) NSString* sourceFileName;
@property(nonatomic, strong, readonly) NSData* data;
@property(nonatomic, readonly) XfastSourceFileType type;

+ (XFSourceFileDefinition*)sourceDefinitionWithName:(NSString*)name
                                               text:(NSString*)text
                                               type:(XfastSourceFileType)type;

+ (XFSourceFileDefinition*)sourceDefinitionWithName:(NSString*)name
                                               data:(NSData*)data
                                               type:(XfastSourceFileType)type;

- (id)initWithName:(NSString*)name
              text:(NSString*)text
              type:(XfastSourceFileType)type;

- (id)initWithName:(NSString*)name
              data:(NSData*)data
              type:(XfastSourceFileType)type;


@end
