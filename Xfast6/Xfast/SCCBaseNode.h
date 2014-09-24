
#import <Cocoa/Cocoa.h>

@interface SCCBaseNode : NSObject <NSCoding, NSCopying>

@property (strong) NSString *nodeTitle;
@property (strong) NSImage *nodeIcon;
@property (strong) NSMutableArray *children;
@property (strong) NSString *urlString;
@property (strong) NSString *key;
@property (assign) BOOL isLeaf;

- (id)initLeaf;

- (BOOL)isDraggable;

- (NSComparisonResult)compare:(SCCBaseNode *)aNode;

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
