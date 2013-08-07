/*
     File: ATColorView.m
 Abstract: A basic NSView subclass that supports having an animatable background color (NOTE: the animation only works when the view is layer backed). 
 */

#import "ATColorView.h"

#import <Quartz/Quartz.h>

@implementation ATColorView

+ (id)defaultAnimationForKey:(NSString *)key {
    if ([key isEqualToString:@"backgroundColor"]) {
        return [CABasicAnimation animation];
    }
    return [super defaultAnimationForKey:key];
}

@synthesize backgroundColor = _backgroundColor;
@synthesize drawBorder;

- (void)setBackgroundColor:(NSColor *)value {
    _backgroundColor = value;
    self.layer.backgroundColor = _backgroundColor.CGColor;
    [self setNeedsDisplay:YES];    
}

- (void)drawRect:(NSRect)r {
    NSColor *color = [self backgroundColor];
    if (color) {
        [color set];
        NSRectFill(r);
    }
    if (self.drawBorder) {
        [[NSColor lightGrayColor] set];
        NSFrameRectWithWidth(self.bounds, 1.0);
    }
    if (self.window.firstResponder == self) {
        NSSetFocusRingStyle(NSFocusRingOnly);
        NSRectFill(self.bounds);
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if (NSPointInRect(point, self.bounds) && self.action) {
        [NSApp sendAction:self.action to:self.target];
    }
}

+ (Class)cellClass {
    // The cell is a container for the target/action
    return [NSActionCell class];
}

@end
