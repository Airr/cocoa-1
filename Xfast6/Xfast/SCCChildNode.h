
#import <Cocoa/Cocoa.h>

#import "SCCBaseNode.h"

@interface SCCChildNode : SCCBaseNode
{
	NSString *description;
	NSTextStorage *text;
}

- (void)setDescription:(NSString *)newDescription;
- (NSString *)description;
- (void)setText:(id)newText;
- (NSTextStorage *)text;

@end
