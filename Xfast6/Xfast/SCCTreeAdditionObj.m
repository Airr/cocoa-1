//
//  SCCTreeAdditionObj.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/28/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCTreeAdditionObj.h"


// -------------------------------------------------------------------------------
//	TreeAdditionObj
//
//	This object is used for passing data between the main and secondary thread
//	which populates the outline view.
// -------------------------------------------------------------------------------
@implementation SCCTreeAdditionObj

@synthesize indexPath, nodeURL, nodeName, nodeKey, nodePathRelativeToParent, selectItsParent;

// -------------------------------------------------------------------------------
//  initWithURL:url:name:select
// -------------------------------------------------------------------------------
- (id)initWithURL:(NSString *)url
         withName:(NSString *)name
          withKey:(NSString *)key
withPathRelativeToParent:(NSString *)path
  selectItsParent:(BOOL)select
{
    self = [super init];
    if (self) {
        nodeURL = url;
        nodeName = name;
        nodeKey = key;
        nodePathRelativeToParent = path;
        selectItsParent = select;
    }
    return self;
}
@end
