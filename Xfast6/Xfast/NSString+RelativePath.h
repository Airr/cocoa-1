//
//  NSString+RelativePath.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/21/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (RelativePath)

- (NSString *)resolveParentPath;
- (NSString *)absolutePathFromBaseDirPath:(NSString *)baseDirPath;
- (NSString *)relativePathFromBaseDirPath:(NSString *)baseDirPath;

@end