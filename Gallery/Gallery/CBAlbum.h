//
//  CBAlbum.h
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <CoreData/CoreData.h>

@interface CBAlbum : NSManagedObject
@property (retain) NSString* title;
@property (retain) NSSet* photos;
+ (id) defaultAlbum;
+ (id) albumInDefaultContext;
@end
