/*
     File: ATTableCellView.m
 Abstract: A basic subclass of NSTableCellView that adds some properties strictly for allowing access to the items in code. 
 */

#import "ATTableCellView.h"
#import "ATColorView.h"

@implementation ATTableCellView

@synthesize subTitleTextField;
@synthesize colorView;
@synthesize progessIndicator;

- (void)layoutViewsForSmallSize:(BOOL)smallSize animated:(BOOL)animated {
    if (_isSmallSize != smallSize) {
        _isSmallSize = smallSize;
        CGFloat targetAlpha = _isSmallSize ? 0 : 1;
        if (animated) {
            [[removeButton animator] setAlphaValue:targetAlpha];
            [[colorView animator] setAlphaValue:targetAlpha];
            [[subTitleTextField animator] setAlphaValue:targetAlpha];
        } else {
            [removeButton setAlphaValue:targetAlpha];
            [colorView setAlphaValue:targetAlpha];
            [subTitleTextField setAlphaValue:targetAlpha];
        }
    }
}

- (NSArray *)draggingImageComponents {
    // Start with what is already there (this is an image and text component)
    NSMutableArray *result = [[super draggingImageComponents] mutableCopy];

    // Snapshot the color view and add it in
    NSRect viewBounds = [colorView bounds];
    NSBitmapImageRep *imageRep = [colorView bitmapImageRepForCachingDisplayInRect:viewBounds];
    [colorView cacheDisplayInRect:viewBounds toBitmapImageRep:imageRep];
    
    NSImage *draggedImage = [[NSImage alloc] initWithSize:[imageRep size]];
    [draggedImage addRepresentation:imageRep];

    // Add in another component
    NSDraggingImageComponent *colorComponent = [NSDraggingImageComponent draggingImageComponentWithKey:@"Color"];
    colorComponent.contents = draggedImage;
    
    // Convert the frame to our coordinate system
    viewBounds = [self convertRect:viewBounds fromView:colorView];
    colorComponent.frame = viewBounds;
    
    [result addObject:colorComponent];
    return result;
}

@end
