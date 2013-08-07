/*
     File: ATDesktopEntity.h 
 Abstract: A sample model object. A base abstract class (ATDesktopEntity) implements caching of a file URL. One concrete subclass implements the ability to have an array of children (ATDesktopFolderEntity). Another (ATDesktopImageEntity) represents an image suitable for the desktop wallpaper.  
*/

#import <Cocoa/Cocoa.h>

// Base abstract class that wraps a file system URL 
@interface ATDesktopEntity : NSObject<NSPasteboardWriting, NSPasteboardReading> {
@private
    NSURL *_fileURL;
}

- (id)initWithFileURL:(NSURL *)fileURL;

@property (retain, readonly) NSString *title;
@property (retain) NSURL *fileURL;

+ (ATDesktopEntity *)entityForURL:(NSURL *)url;

@end

// Concrete subclass of ATDesktopEntity that loads children from a folder
@interface ATDesktopFolderEntity : ATDesktopEntity {
@private
    NSMutableArray *_children;
}

@property(retain) NSMutableArray *children;

@end

// Concrete subclass of ATDesktopEntity that adds support for loading an image at the given URL and stores a fillColor property
@interface ATDesktopImageEntity : ATDesktopEntity {
@private
    BOOL _imageLoading;
    NSString *_title;
    NSImage *_image;
    NSImage *_thumbnailImage;
    NSColor *_fillColor;
    NSString *_fillColorName;
}

@property (retain) NSColor *fillColor;
@property (copy) NSString *fillColorName;

@property (retain, readwrite) NSString *title;

// Access to the image. This property can be observed to find out when it changes and is fully loaded.
@property (retain) NSImage *image;
@property (readonly, retain, nonatomic) NSImage *thumbnailImage;

// Asynchronously loads the image (if not already loaded). A KVO notification is sent out when the image is loaded.
- (void)loadImage;

// A nil image isn't loaded (or couldn't be loaded). An image that is in the process of loading has imageLoading set to YES
@property (readonly) BOOL imageLoading;

@end

// Declared constants to avoid typos in KVO. Common prefixes are used for easy code completion.
extern NSString *const ATEntityPropertyNamedFillColor;
extern NSString *const ATEntityPropertyNamedFillColorName;
extern NSString *const ATEntityPropertyNamedImage;
extern NSString *const ATEntityPropertyNamedThumbnailImage;

