//
//  SCCDocument.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/17/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCDocument.h"
#import "SCCWindowController.h"
#import "XcodeEditor.h"

@interface SCCDocument ()

@property XCProject* project;

@end

@implementation SCCDocument

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
    }
    return self;
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

+ (BOOL)autosavesInPlace
{
    return YES;
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
    @throw exception;
    return nil;
}

//- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
//{
//    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
//    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
//    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
//    NSException *exception = [NSException exceptionWithName:@"UnimplementedMethod" reason:[NSString stringWithFormat:@"%@ is unimplemented", NSStringFromSelector(_cmd)] userInfo:nil];
//    @throw exception;
//    return YES;
//}

NSString *TextFileName = @"project.pbxproj";

- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper
                     ofType:(NSString *)typeName
                      error:(NSError **)outError
{
    NSLog(@"Project file is %@", [fileWrapper filename]);
    NSLog(@"Project file is %@", [[self fileURL] path]);
    [self setProject:[[XCProject alloc] initWithFilePath:[[self fileURL] path]]];
    
    return YES;
}

- (BOOL)isDocumentEdited {
    return NO;
}

#pragma mark *** Overrides of NSDocument Methods ***

- (void)makeWindowControllers {
    // Start off with one document window.
    if (_project) {
        SCCWindowController *windowController = [[SCCWindowController alloc] initWithProject:_project];
        [self addWindowController:windowController];
    } else {
        SCCWindowController *windowController = [[SCCWindowController alloc] init];
        [self addWindowController:windowController];
    }
}

- (void)calledByView
{
    NSLog(@"called by a wiew");
}
@end
