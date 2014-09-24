//
//  SCCTreeAdditionObj.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/28/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface SCCTreeAdditionObj : NSObject
{
    NSIndexPath *__unsafe_unretained indexPath;
    NSString	*__unsafe_unretained nodeURL;
    NSString	*__unsafe_unretained nodeName;
    NSString	*__unsafe_unretained nodeKey;
    NSString	*__unsafe_unretained nodePathRelativeToParent;
    BOOL		selectItsParent;
}

@property (unsafe_unretained, readonly) NSIndexPath *indexPath;
@property (unsafe_unretained, readonly) NSString *nodeURL;
@property (unsafe_unretained, readonly) NSString *nodeName;
@property (unsafe_unretained, readonly) NSString *nodeKey;
@property (unsafe_unretained, readonly) NSString *nodePathRelativeToParent;
@property (readonly) BOOL selectItsParent;

- (id)initWithURL:(NSString *)url
         withName:(NSString *)name
          withKey:(NSString *)key
withPathRelativeToParent:(NSString *)path
  selectItsParent:(BOOL)select;

@end
