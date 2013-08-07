/*
     File: ATColorView.h 
 Abstract: A basic NSView subclass that supports having an animatable background color (NOTE: the animation only works when the view is layer backed).  
*/

#import <Cocoa/Cocoa.h>

@interface ATColorView : NSControl {
@private
    NSColor *_backgroundColor;
}

@property(retain, nonatomic) NSColor *backgroundColor;
@property BOOL drawBorder;

@end
