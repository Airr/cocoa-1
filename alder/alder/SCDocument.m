//
//  SCDocument.m
//  alder
//
//  Created by Sang Chul Choi on 8/2/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCDocument.h"
#import "SCWindowController.h"
#import "SCPerson.h"
#import "SCFastq.h"
//#import "SCRepositoryAnalysis.h"


@interface SCDocument ()
@property NSAttributedString *mString;
@end

@implementation SCDocument

@synthesize mString;

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
    }
    return self;
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"SCDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

+ (BOOL)autosavesInPlace
{
    return NO;
}

- (void)makeWindowControllers
{
    SCWindowController *aWindowController = [[SCWindowController alloc] init];
    [aWindowController setManagedObjectContext:[self managedObjectContext]];
    [self addWindowController:aWindowController];
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL result = TRUE;

    NSPersistentStoreCoordinator *psc = [[self managedObjectContext] persistentStoreCoordinator];
    NSPersistentStore *store = [psc addPersistentStoreWithType:NSSQLiteStoreType
                                                 configuration:nil
                                                           URL:absoluteURL
                                                       options:nil
                                                         error:outError];
    [store isReadOnly];
    return result;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation originalContentsURL:(NSURL *)absoluteOriginalContentsURL error:(NSError **)outError
{
    NSLog(@"updateMOC");
    if (absoluteOriginalContentsURL == nil)
    {
        NSPersistentStoreCoordinator *psc = [[self managedObjectContext] persistentStoreCoordinator];
        NSPersistentStore *store = [psc addPersistentStoreWithType:NSSQLiteStoreType
                                                     configuration:nil
                                                               URL:absoluteURL
                                                           options:nil
                                                             error:outError];
        [store isReadOnly];
        absoluteOriginalContentsURL = absoluteURL;
        
        return [super writeToURL:absoluteURL
                          ofType:typeName
                forSaveOperation:saveOperation
             originalContentsURL:absoluteOriginalContentsURL
                           error:outError];
    }
    return TRUE;
}

- (BOOL)isDocumentEdited
{
    return NO;
}

@end
