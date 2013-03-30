//
//  StretchView.m
//  DrawingFun
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "StretchView.h"

@implementation StretchView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Seed the random number generator
        srandom((unsigned)time(NULL));
        // Create a path object
        path = [NSBezierPath bezierPath];
        [path setLineWidth:3.0];
        NSPoint p = [self randomPoint];
        [path moveToPoint:p];
        int i;
        for (i = 0; i < 15; i++) {
            p = [self randomPoint];
            [path lineToPoint:p];
        }
        [path closePath];
    }
    return self;
}

// randomPoint returns a random point inside the view
- (NSPoint)randomPoint
{
    NSPoint result;
    NSRect r = [self bounds];
    result.x = r.origin.x + random() % (int)r.size.width;
    result.y = r.origin.y + random() % (int)r.size.height;
    return result;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSRect bounds = [self bounds];
    // Fill the view with green
    [[NSColor greenColor] set];
    [NSBezierPath fillRect: bounds];
    // Draw the path in white
    [[NSColor whiteColor] set];
    [path stroke];
}

@end
