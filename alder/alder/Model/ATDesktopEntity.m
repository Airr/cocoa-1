/*
     File: ATDesktopEntity.m
 Abstract: A sample model object. A base abstract class (ATDesktopEntity) implements caching of a file URL. One concrete subclass implements the ability to have an array of children (ATDesktopFolderEntity). Another (ATDesktopImageEntity) represents an image suitable for the desktop wallpaper.
*/

#import "ATDesktopEntity.h"

#import <Quartz/Quartz.h>

#define THUMBNAIL_HEIGHT 180.0 

// For the purposes of a demo, we intentionally make things slower. Turning off the DEMO_MODE define will make things run at normal speed.
#define DEMO_MODE 0

@implementation ATDesktopEntity

+ (ATDesktopEntity *)entityForURL:(NSURL *)url {
    // We create folder items or image items, and ignore everything else; all based on the UTI we get from the URL
    NSString *typeIdentifier;
    if ([url getResourceValue:&typeIdentifier forKey:NSURLTypeIdentifierKey error:NULL]) {
        NSArray *imageUTIs = [NSImage imageTypes];
        if ([imageUTIs containsObject:typeIdentifier]) {
            return [[ATDesktopImageEntity alloc] initWithFileURL:url];
        } else if ([typeIdentifier isEqualToString:(NSString *)kUTTypeFolder]) {
            return [[ATDesktopFolderEntity alloc] initWithFileURL:url];
        } 
    }
    return nil;
}

@synthesize fileURL = _fileURL;
@dynamic title;

- (id)initWithFileURL:(NSURL *)fileURL {
    self = [super init];
    _fileURL = fileURL;
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    id result = [[[self class] alloc] initWithFileURL:self.fileURL];
    return result;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ : %@", [super description], self.title];
}


- (NSString *)title {
    NSString *result;
    if ([self.fileURL getResourceValue:&result forKey:NSURLLocalizedNameKey error:NULL]) {
        return result;
    }
    return nil;
}

#pragma mark -
#pragma mark NSPasteboardWriting support

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard {
    return [self.fileURL writableTypesForPasteboard:pasteboard];
}

- (id)pasteboardPropertyListForType:(NSString *)type {
    return [self.fileURL pasteboardPropertyListForType:type];
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard {
    if ([self.fileURL respondsToSelector:@selector(writingOptionsForType:pasteboard:)]) {
        return [self.fileURL writingOptionsForType:type pasteboard:pasteboard];
    } else {
        return 0;
    }
}

#pragma mark -
#pragma mark  NSPasteboardReading support

+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard {
    // We allow creation from folder and image URLs only, but there is no way to specify just file URLs that contain images
    return [NSArray arrayWithObjects:(id)kUTTypeFolder, (id)kUTTypeFileURL, nil];
}

+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard {
    return NSPasteboardReadingAsString;
}

- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type {
    // We recreate the appropriate object
//    [self release];
//    self = nil;
    // We only have URLs accepted. Create the URL
    NSURL *url = [[NSURL alloc] initWithPasteboardPropertyList:propertyList ofType:type];
    // Now see what the data type is; if it isn't an image, we return nil
    NSString *urlUTI;
    if ([url getResourceValue:&urlUTI forKey:NSURLTypeIdentifierKey error:NULL]) {
        // We could use UTTypeConformsTo((CFStringRef)type, kUTTypeImage), but we want to make sure it is an image UTI type that NSImage can handle
        if ([[NSImage imageTypes] containsObject:urlUTI]) {
            // We can use it with NSImage
            self = [[ATDesktopImageEntity alloc] initWithFileURL:url]; 
        } else if ([urlUTI isEqualToString:(id)kUTTypeFolder]) {
            // It is a folder
            self = [[ATDesktopFolderEntity alloc] initWithFileURL:url];
        }
    }
    // We may return nil 
    return self;
}

#pragma mark -

- (NSString *)imageUID {
    return [NSString stringWithFormat:@"%p", self];
}

- (NSString *)imageRepresentationType {
    return IKImageBrowserNSURLRepresentationType;
}

- (id)imageRepresentation {
    return self.fileURL;
}

- (NSUInteger)imageVersion {
    return 0;
}

- (NSString *)imageTitle {
    return self.title;
}

- (NSString *)imageSubtitle {
    return nil;
}

- (BOOL)isSelectable {
    return YES;
}


@end

// We create an empty category to add property overrides
@interface ATDesktopImageEntity()

// Private read/write access to the thumbnailImage
@property (readwrite, retain, nonatomic) NSImage *thumbnailImage;  

@property (readwrite) BOOL imageLoading;

@end

static NSOperationQueue *ATSharedOperationQueue() {
    static NSOperationQueue *_ATSharedOperationQueue = nil;
    if (_ATSharedOperationQueue == nil) {
        _ATSharedOperationQueue = [[NSOperationQueue alloc] init];
        // We limit the concurrency to see things easier for demo purposes. The default value NSOperationQueueDefaultMaxConcurrentOperationCount will yield better results, as it will create more threads, as appropriate for your processor
       [_ATSharedOperationQueue setMaxConcurrentOperationCount:2];
    }
    return _ATSharedOperationQueue;    
}

@implementation ATDesktopImageEntity

- (id)initWithFileURL:(NSURL *)fileURL {
    self = [super initWithFileURL:fileURL];
    // Initialize our color to specific given color for testing purposes
    static NSInteger lastColorIndex = 0;
    NSColorList *colorList = [NSColorList colorListNamed:@"Crayons"];
    NSArray *keys = [colorList allKeys];
    if (lastColorIndex >= keys.count) {
        lastColorIndex = 0;
    }
    _fillColorName = [keys objectAtIndex:lastColorIndex++];
    _fillColor = [colorList colorWithKey:_fillColorName];
    self.title = [super title];
    return self;
}

@synthesize fillColor = _fillColor;
@synthesize fillColorName = _fillColorName;
@synthesize imageLoading = _imageLoading;
@synthesize image = _image;
@synthesize title =  _title;

static NSImage *ATThumbnailImageFromImage(NSImage *image) {
    NSSize imageSize = [image size];
    CGFloat imageAspectRatio = imageSize.width / imageSize.height;
    // Create a thumbnail image from this image (this part of the slow operation)
    NSSize thumbnailSize = NSMakeSize(THUMBNAIL_HEIGHT * imageAspectRatio, THUMBNAIL_HEIGHT);
    NSImage *thumbnailImage = [[NSImage alloc] initWithSize:thumbnailSize];
    [thumbnailImage lockFocus];
    [image drawInRect:NSMakeRect(0, 0, thumbnailSize.width, thumbnailSize.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];                
    [thumbnailImage unlockFocus];
    
#if DEMO_MODE
    // We delay things with an explicit sleep to get things slower for the demo!
    usleep(250000);
#endif
    
    return thumbnailImage;
}

// Lazily load the thumbnail image when requested
- (NSImage *)thumbnailImage {
    if (self.image != nil && _thumbnailImage == nil) {
        // Generate the thumbnail right now, synchronously
        _thumbnailImage = ATThumbnailImageFromImage(self.image);
    } else if (self.image == nil && !self.imageLoading) {
        // Load the image lazily
        [self loadImage];
    }        
    return _thumbnailImage;
}

- (void)setThumbnailImage:(NSImage *)img {
    if (img != _thumbnailImage) {
        _thumbnailImage = img;
    }
}

- (void)loadImage {
    @synchronized (self) {
        if (self.image == nil && !self.imageLoading) {
            self.imageLoading = YES;
            // We would have to keep track of the block with an NSBlockOperation, if we wanted to later support cancelling operations that have scrolled offscreen and are no longer needed. That will be left as an exercise to the user.
            [ATSharedOperationQueue() addOperationWithBlock:^(void) {
                NSImage *image = [[NSImage alloc] initWithContentsOfURL:self.fileURL];
                if (image != nil) {
                    NSImage *thumbnailImage = ATThumbnailImageFromImage(image);
                    // We synchronize access to the image/imageLoading pair of variables
                    @synchronized (self) {
                        self.imageLoading = NO;
                        self.image = image;
                        self.thumbnailImage = thumbnailImage;
                    }
//                    [image release];
                } else {
                    @synchronized (self) {
                        self.image = [NSImage imageNamed:NSImageNameTrashFull];
                    }
                }
            }];
        }
    }
}

@end

@implementation ATDesktopFolderEntity

@dynamic children;

- (NSMutableArray *)children {
    NSMutableArray *result = nil;
    // This property is declared as atomic. We use @synchronized to ensure that promise is kept
    @synchronized(self) {
        // It would be nice if this was asycnhronous to avoid any stalls while we look at the file system. A mechanism similar to how the ATDesktopImageEntity loads images could be used here
        if (_children == nil && self.fileURL != nil) {
            NSError *error = nil;
            // Grab the URLs for the folder and wrap them in our entity objects
            NSArray *urls = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:self.fileURL includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLLocalizedNameKey, nil] options:NSDirectoryEnumerationSkipsHiddenFiles | NSDirectoryEnumerationSkipsSubdirectoryDescendants error:&error];
            NSMutableArray *newChildren = [[NSMutableArray alloc] initWithCapacity:urls.count];
            for (NSURL *url in urls) {
                // We create folder items or image items, and ignore everything else; all based on the UTI we get from the URL
                NSString *typeIdentifier;
                if ([url getResourceValue:&typeIdentifier forKey:NSURLTypeIdentifierKey error:NULL]) {
                    ATDesktopEntity *entity = [ATDesktopEntity entityForURL:url];
                    if (entity) {
                        [newChildren addObject:entity];
                    }
                }
            }
            _children = newChildren;
        }
        result = _children;
    }
    return result;
}

- (void)setChildren:(NSMutableArray *)value {
    // This property is declared as atomic. We use @synchronized to ensure that promise is kept
    @synchronized(self) {
        if (_children != value) {
            _children = value;
        }
    }
}

@end

NSString *const ATEntityPropertyNamedFillColor = @"fillColor";
NSString *const ATEntityPropertyNamedFillColorName = @"fillColorName";
NSString *const ATEntityPropertyNamedImage = @"image";
NSString *const ATEntityPropertyNamedThumbnailImage = @"thumbnailImage";

