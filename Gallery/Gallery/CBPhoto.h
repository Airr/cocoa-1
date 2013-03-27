//
//  CBPhoto.h
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <CoreData/CoreData.h>
@class CBAlbum;
@interface CBPhoto : NSManagedObject

// attributes.
@property (retain) NSString* filePath; @property (retain) NSString* uniqueID; @property (retain) NSNumber* orderIndex;

// relationships.
@property (retain) CBAlbum* album;

// non-modeled properties.
@property (readonly) NSImage* largeThumbnail;
// methods.
+ (id) photoInDefaultContext;

@end
