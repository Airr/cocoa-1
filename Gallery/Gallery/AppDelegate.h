//
//  AppDelegate.h
//  Gallery
//
//  Created by Sang Chul Choi on 3/26/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CBMainWindow;
@class CBAlbum;

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (retain) CBMainWindow* mainWindowController;
@property (retain) CBAlbum* selectedAlbum;
- (IBAction) newAlbum: (id)sender;

@property (assign) IBOutlet NSWindow *window;

@property (readonly, strong, nonatomic) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (readonly, strong, nonatomic) NSManagedObjectModel *managedObjectModel;
@property (readonly, strong, nonatomic) NSManagedObjectContext *managedObjectContext;

- (IBAction)saveAction:(id)sender;

@end
