/*
     File: BaseNode.h 
 Abstract: Generic multi-use node object used with NSOutlineView and NSTreeController.
  
  Version: 1.5 
  
 Copyright (C) 2014 Apple Inc. All Rights Reserved. 
  
 */

#import <Cocoa/Cocoa.h>

@interface BaseNode : NSObject <NSCoding, NSCopying>

@property (strong) NSString *nodeTitle;
@property (strong) NSImage *nodeIcon;
@property (strong) NSMutableArray *children;
@property (strong) NSString *urlString;
@property (assign) BOOL isLeaf;

- (id)initLeaf;

- (BOOL)isDraggable;

- (NSComparisonResult)compare:(BaseNode *)aNode;

- (NSArray *)mutableKeys;

- (NSDictionary *)dictionaryRepresentation;
- (id)initWithDictionary:(NSDictionary *)dictionary;

- (id)parentFromArray:(NSArray *)array;
- (void)removeObjectFromChildren:(id)obj;
- (NSArray *)descendants;
- (NSArray *)allChildLeafs;
- (NSArray *)groupChildren;
- (BOOL)isDescendantOfOrOneOfNodes:(NSArray *)nodes;
- (BOOL)isDescendantOfNodes:(NSArray *)nodes;
- (NSIndexPath *)indexPathInArray:(NSArray *)array;

@end
