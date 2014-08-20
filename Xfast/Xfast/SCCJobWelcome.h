//
//  SCCJobWelcome.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SCCJobWelcome : NSObject

+ (NSString *)template;

+ (BOOL)writeWithDictionary:(NSDictionary *)data ToFile:(NSString *)path;

@end
