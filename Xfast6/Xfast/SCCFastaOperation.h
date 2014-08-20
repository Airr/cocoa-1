//
//  SCCFastaOperation.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
@class XFProject;
@interface SCCFastaOperation : NSOperation

@property (weak, readonly) XFProject *project;
@property (copy) NSString *commandLine;

-(id)initWithXFProject:(XFProject *)project;

@end
