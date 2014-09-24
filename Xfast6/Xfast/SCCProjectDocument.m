//
//  SCCProjectDocument.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCProjectDocument.h"
#import "SCCProjectWindowController.h"
#import "SCCProjectKeys.h"
#import "XcodeEditor.h"
#import "XfastEditor.h"
#import "SCCChildNode.h"
#import "SCCTargetWindowController.h"
#import "SCCCommand.h"
#import "SCCOperationOfwc.h"
#import "SCCDefaultsKeys.h"

static NSString *SpndTransactionsContext = @"com.apple.iSpend.transactions";
static NSString *SpndAmountContext = @"com.apple.iSpend.amount";
static NSString *SpndTypeContext = @"com.apple.iSpend.type";
static NSString *SpndAccountTypeContext = @"com.apple.iSpend.accountType";

@interface SCCProjectDocument ()
@property XCProject *project;
@property XFProject *xfast;
@property NSMutableArray *contentsOfProjectNavigator;
@property NSMutableDictionary *data;
@property NSMutableArray *contentsOfXfastOutline;
@property SCCProjectWindowController *windowController;
@property SCCTargetWindowController *targetWindowController;

@property NSOperationQueue *queue;

@end

@implementation SCCProjectDocument

static NSArray *contentXfastMessages;

+ (NSArray *)contentXfastMessages
{
    if (nil == contentXfastMessages) {
        contentXfastMessages = [NSArray arrayWithObjects:
//                                @"nodeIsSelected",
                                @"addFileReference",
                                nil];
    }
    return contentXfastMessages;
}

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        _contentsOfProjectNavigator = [[NSMutableArray alloc] init];
        _contentsOfXfastOutline = [[NSMutableArray alloc] init];
        _queue = [[NSOperationQueue alloc] init];
        
    }
    return self;
}

- (void)dealloc
{
    [[self windowController] removeObserver:self forKeyPath:@"contentOutlineViewController"];
    [[self windowController] removeObserver:self forKeyPath:@"navigatorViewController"];
    [[self targetWindowController] removeObserver:self forKeyPath:@"xfastType"];
    
    
    for (NSString *contentXfastMessage in [SCCProjectDocument contentXfastMessages]) {
        [[self windowController] removeObserver:self
                                           forKeyPath:contentXfastMessage];
    }
    

}

#pragma mark - Files

/**
 *  Adds file references.
 *
 *  @param filePaths The array of file paths.
 *
 *  @return The array of the file path keys.
 */
- (NSArray *)addFileReference:(NSMutableArray *)filePaths
{
    NSMutableArray *keys = [NSMutableArray array];
    
    NSMutableIndexSet *discardedItems = [NSMutableIndexSet indexSet];
    NSUInteger index = 0;
    
    for (NSString *filePath in filePaths) {
        XFSourceFile *file = [_xfast fileWithPath:filePath];
        if (file) {
            // The item should be deleted.
            [discardedItems addIndex:index];
        } else {
            // Create a file reference.
            NSString *key = [[XFKeyBuilder createUnique] build];
            [_xfast addFileOfType:@"text" withAbsolutePath:filePath withKey:key];
            [keys addObject:key];
        }
        index++;
    }
    
    [filePaths removeObjectsAtIndexes:discardedItems];
    
    [_xfast save];
    
    return keys;
}

#pragma mark - Add Source Build Files To Target

/**
 *  Adds source files to the target.
 *
 *  @param newNodes  The nodes that are added to the target.
 *  @param targetKey Target key.
 */
- (void)addSourceFiles:(NSMutableArray *)newNodes toTarget:(NSString *)targetKey
{
    XFTarget *target = [_xfast targetWithKey:targetKey];
    
    NSMutableIndexSet *discardedItems = [NSMutableIndexSet indexSet];
    NSUInteger index = 0;

    for (NSTreeNode *node in newNodes) {
        SCCChildNode *sourceNode = [node representedObject];
        // Check if the node is in the source member.
        if ([target existSourceMemberWithKey:[sourceNode key]]) {
            // The item is in the source build of the target.
            [discardedItems addIndex:index];
        } else {
            // Add the file reference to the target later.
        }
        index++;
    }
    [newNodes removeObjectsAtIndexes:discardedItems];
    
    // Add the remained file referenec.
    for (NSTreeNode *node in newNodes) {
        SCCChildNode *sourceNode = [node representedObject];
        XFSourceFile *file = [_xfast fileWithKey:[sourceNode key]];
        [target addSourceMember:file];
    }
    
    [_xfast save];
    
    // Add output build files to the target depending on the target's behavior.
    // Use the command class to find out which output files are needed.
    // We use
    // 1. Xfast
    // 2. Target's configuration
    // 3. Input build files
    // to update the
    // output build files.
    // We do this in Xfast-Editor classes.
    // So, the target class should have this output file logic.
    [target updateOutputBuildFiles];
    
    [_xfast save];
}

- (NSArray *)xfastOutputBuildFilesWithTargetKey:(NSString *)aTargetKey
{
    XFTarget *target = [_xfast targetWithKey:aTargetKey];
    NSMutableArray *array = [NSMutableArray array];
    for (XFSourceFile *file in [target outputMembers]) {
        NSDictionary *dictionay = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [file key], @"key",
                                   [file absolutePath], @"path",
                                   nil];
        [array addObject:dictionay];
    }
    
    return array;
}


/**
 *  Add a target to the current Xfast. If the same named target exists, it 
 * will fail to add a new target. Otherwise, a new target will be added.
 *
 *  @param aName The target name.
 *
 *  @return The target key.
 */
- (NSString *)addTarget:(NSString *)aName
{
    XFTarget *target = [_xfast targetWithName:aName];
    if (target) {
        return nil;
    } else {
        [_xfast addTarget:aName];
        target = [_xfast targetWithName:aName];
        
        // Creates Target directory
        NSFileManager* fileManager = [NSFileManager defaultManager];
        NSString *databaseDirectory = [[NSUserDefaults standardUserDefaults] stringForKey:DatabaseDirectory];
        NSString *userName = [[NSUserDefaults standardUserDefaults] stringForKey:UserName];
        NSString *targetDirectoryInDatabase = [NSString stringWithFormat:@"%@/%@/%@/%@/%@/out",
                                               databaseDirectory, userName,
                                               [[self project] name],
                                               [_xfast name],
                                               aName];
        [fileManager createDirectoryAtPath:targetDirectoryInDatabase
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        // Creates a target directory
        NSString *targetDirectoryInProject = [NSString stringWithFormat:@"%@/%@/%@/%@/out",
                                              [[[self fileURL] path] stringByDeletingLastPathComponent],
                                              [[self project] name],
                                              [_xfast name],
                                              aName];
        [fileManager createDirectoryAtPath:targetDirectoryInProject
               withIntermediateDirectories:YES
                                attributes:nil
                                     error:nil];
        
        return [target key];
    }
}

/**
 *  Returns files for the output in a target.
 *
 *  @param aKey The key of a target.
 *
 *  @return The array of dictionaries.
 */
- (NSArray *)outFilesOfTargetWithKey:(NSString *)aKey
{
    XFTarget *target = [_xfast targetWithKey:aKey];
    return [target arrayOfOutfiles];
}

- (void)printTree:(NSMutableArray *)array
{
    for (SCCChildNode *node in array) {
        NSLog(@"N: %@ - %@", [node nodeTitle], [node key]);
        if (![node isLeaf]) {
            [self printTree:[node children]];
        }
    }
}

- (void)buildGroup:(NSArray *)array
           withKey:(NSString *)key
          withName:(NSString *)name
{
    NSMutableArray *keys = [NSMutableArray array];
    for (SCCChildNode *node in array) {
//        NSLog(@"N: %@ - %@", [node nodeTitle], [node key]);
        if (![node isLeaf]) {
            [self buildGroup:[node children]
                     withKey:[node key]
                    withName:[node nodeTitle]];
        }
        [keys addObject:[node key]];
    }
    [[[self project] objects] setObject:[[self project] objectGroupWithKeys:keys withName:name]
                                 forKey:key];
    
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    
    if (object == [[self windowControllers] firstObject]) {
        if ([keyPath compare:@"navigatorViewController"] == NSOrderedSame) {
            NSLog(@"Document: project navigator view is called.");
            // Load the Xfast outline view.
            // Load the content view with a default selection of the Xfast outline view.
            // How can I know which one is selected in the navigator outline view?
            // I may use the main window controller.
            //
            //
            // Check which item is clicked in the navigator.
            //
//            [[self windowController] showContentOfXfast];
            NSString *selectedNodeKey = [[self windowController] selectedNodeKeyOfProjectNavigator];
            if (selectedNodeKey) {
                // Show the content for the node.
                // Check the type of the key.
                XcodeMemberType type = [[self project] memberTypeWithKey:selectedNodeKey];
                if (type == PBXProjectType) {
                    // Show project content.
                    [[self windowController] showContentOfProject];
                } else if (type == PBXNativeTargetType) {
                    // Show product content.
                    [[self windowController] showContentOfProduct];
                } else if (type == PBXFileReferenceType) {
                    // Show xfast content.
                    [self showContentOfXfast:selectedNodeKey];
                } else {
                    [[self windowController] showContentOfNothing];
                    // No change.
                }
            } else {
                // No change.
            }
        } else if ([keyPath compare:@"contentOutlineViewController"] == NSOrderedSame) {
            NSLog(@"Document: content outline view is called.");

        } else if ([keyPath compare:@"addFileReference"] == NSOrderedSame) {
            NSLog(@"Document: content outline view is called - add file reference.");
//            _contentsOfXfastOutline.count;
            NSMutableArray *a = [_windowController treeContentOfXfastOutline];
            NSLog(@"%@", a);
        }
    } else if ([keyPath isEqual:@"xfastType"]) {
        NSLog(@"KVO: xfastName is %@", [[_targetWindowController xfastName] stringValue]);
        NSLog(@"KVO: xfastType is %@", [_targetWindowController xfastType]);
        NSString *key = [[XCKeyBuilder createUnique] build];
        [[_windowController window] endSheet:[_targetWindowController window]];
        [_windowController addXfast:[_targetWindowController xfastType]
                           withName:[[_targetWindowController xfastName] stringValue]
                            withKey:key];
        // Update the array.
        [self addXfastToProject:[_targetWindowController xfastType]
                       withName:[[_targetWindowController xfastName]
                                 stringValue] withKey:key];

    } else {
        // the notification wasn't recognized, so it was probably meant for someone else.  Invoke super.
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

/**
 *  Shows something in response to ProjectNavigatorViewController.
 */
- (void)showProjectNavigatorView
{
//    NSLog(@"Document: project navigator view is called.");
    // Load the Xfast outline view.
    // Load the content view with a default selection of the Xfast outline view.
    // How can I know which one is selected in the navigator outline view?
    // I may use the main window controller.
    //
    //
    // Check which item is clicked in the navigator.
    //
    //            [[self windowController] showContentOfXfast];
    NSString *selectedNodeKey = [[self windowController] selectedNodeKeyOfProjectNavigator];
    if (selectedNodeKey) {
        // Show the content for the node.
        // Check the type of the key.
        XcodeMemberType type = [[self project] memberTypeWithKey:selectedNodeKey];
        if (type == PBXProjectType) {
            // Show project content.
            [[self windowController] showContentOfProject];
        } else if (type == PBXNativeTargetType) {
            // Show product content.
            [[self windowController] showContentOfProduct];
        } else if (type == PBXFileReferenceType) {
            // Show xfast content.
            [self showContentOfXfast:selectedNodeKey];
        } else {
            [[self windowController] showContentOfNothing];
            // No change.
        }
    } else {
        // No change.
    }
}


- (NSString *)windowNibName {
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"SCCProjectDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController {
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
}

- (void)updateXfastProject
{
    [self printTree:[self contentsOfProjectNavigator]];
    [[self project] removeAllGroup];
    [self buildGroup:[self contentsOfProjectNavigator]
             withKey:[_project mainGroupKey]
            withName:nil];
    XCGroup *rootGroup = [[self project] rootGroup];
    XCGroup *projectGroup = [[self project] groupWithKey:
                             [[[rootGroup members] firstObject] key]];
    [projectGroup setPathRelativeToParent:[projectGroup alias]];
    [[self project] setObject:projectGroup];
    [[self project] save];
}

- (void)saveDocumentWithDelegate:(id)delegate
                 didSaveSelector:(SEL)didSaveSelector
                     contextInfo:(void *)contextInfo
{
    [self pushProject];
}


- (void)pushProject
{
    //    [self printTree:[self contentsOfProjectNavigator]];
    [[self project] removeAllGroup];
    NSArray *c = [[[self contentsOfProjectNavigator] firstObject] children];
    [self buildGroup:c
             withKey:[_project mainGroupKey]
            withName:nil];
    
    [[self project] save];
}

/**
 *  Opens a Xfast project file.
 *
 *  @param fileWrapper a
 *  @param typeName    b
 *  @param outError    .
 *
 *  @return r
 */
- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper
                     ofType:(NSString *)typeName
                      error:(NSError **)outError
{
    // Project file (file type: Xfast Project) is 1.xfastproj
    // Project file is /Users/goshng/Downloads/1/1.xfastproj
    NSLog(@"Project file (file type: %@) is %@", typeName, [fileWrapper filename]);
    NSLog(@"Project file is %@", [[self fileURL] path]);
    [self setProject:[XCProject projectWithFilePath:[[self fileURL] path]]];
    // Create the NSMutableArray type data.
    _contentsOfProjectNavigator = [[self project] asTreeData];
    _data = [NSMutableDictionary dictionaryWithObjectsAndKeys:
             _contentsOfProjectNavigator, SCC_DATA_PROJECT_NAVIGATOR,
             nil];
    
    // Creates Xfast directory
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString *databaseDirectory = [[NSUserDefaults standardUserDefaults] stringForKey:DatabaseDirectory];
    NSString *userName = [[NSUserDefaults standardUserDefaults] stringForKey:UserName];
    NSString *projectDirectoryInDatabase = [NSString stringWithFormat:@"%@/%@/%@",
                                            databaseDirectory, userName,
                                            [[self project] name]];
    
    [fileManager createDirectoryAtPath:projectDirectoryInDatabase
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];
    
    return YES;
}

//+ (BOOL)autosavesInPlace {
//    return YES;
//}

- (void)makeWindowControllers
{

    [self setWindowController:[[SCCProjectWindowController alloc]
                                                    initWithData:_data]];
    
    [[self windowController] addObserver:self forKeyPath:@"navigatorViewController"
                                 options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                 context:nil];
    [[self windowController] addObserver:self forKeyPath:@"contentOutlineViewController"
                                 options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                 context:nil];
    
    
    for (NSString *contentXfastMessage in [SCCProjectDocument contentXfastMessages]) {
        [[self windowController] addObserver:self
                                  forKeyPath:contentXfastMessage
                                     options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld
                                     context:nil];
    }
    

    [self addWindowController:[self windowController]];
}

#pragma mark - Add Xfast

- (IBAction)addXfast:(id)sender
{
    if (_targetWindowController == nil) {
        _targetWindowController = [[SCCTargetWindowController alloc] initWithWindowNibName:@"SCCTargetWindowController"];
        [_targetWindowController addObserver:self
                                  forKeyPath:@"xfastType"
                                     options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                                     context:NULL];
        
    }
    
    [[_windowController window] beginSheet:[_targetWindowController window]
                         completionHandler:^(NSModalResponse returnCode)
     {
//         NSLog(@"END: add files complete handler");
     }];
//    NSLog(@"END: add files");
    
}

/**
 *  Adds an Xfast file reference.
 *
 *  @param type The Xfast type.
 *  @param name The Xfast file name.
 *  @param key  The Xfast file reference key.
 */
- (void)addXfastToProject:(NSString *)type
                 withName:(NSString *)name
                  withKey:(NSString *)key
{
    // Add a default Xfast file for the type.
    NSString *xfastName = [NSString stringWithFormat:@"%@.%@.xfast", name, type];
    NSString *xfastPath = [NSString stringWithFormat:@"%@/%@.%@.xfast",
                           [[self project] filePath], name, type];
    // Use Xfast template file to create an Xfast file.
    // The template file needs to have build settings.
    // File in/out patterns:
    //   1. one-to-one
    //   2. one-to-many
    //   3. many-to-one
    //   4. many-to-many
    //   5. some-to-some
    // Convert a build setting to a command line option.
    // Do this conversion in the NSOperation.
    
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *path = [bundle pathForResource:type ofType: @"dict"];
    XFProject *xfast = [XFProject projectWithNewFilePath:xfastPath withTemplate:path];
    if (xfast == nil) {
        [NSException raise:NSGenericException format:@"Error: Create project failed %@", xfastPath];
    }
    // Add
    [xfast save];
    
    // Save the project.
    [[self project] removeAllGroup];
    NSArray *c = [[[self contentsOfProjectNavigator] firstObject] children];
    [self buildGroup:c
             withKey:[_project mainGroupKey]
            withName:nil];
    
    // Add a file reference.
    [[self project] addXfastToProject:type withName:xfastName withKey:key];
    [[self project] save];
    
    
    
    // Creates Xfast directory
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString *databaseDirectory = [[NSUserDefaults standardUserDefaults] stringForKey:DatabaseDirectory];
    NSString *userName = [[NSUserDefaults standardUserDefaults] stringForKey:UserName];
    NSString *xfastDirectoryInDatabase = [NSString stringWithFormat:@"%@/%@/%@/%@",
                                          databaseDirectory, userName,
                                          [[self project] name],
                                          [xfast name]];
    [fileManager createDirectoryAtPath:xfastDirectoryInDatabase
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];
    // Xfast in the project directory
    NSString *xfastDirectoryInProject = [NSString stringWithFormat:@"%@/%@/%@",
                                         [[[self fileURL] path] stringByDeletingLastPathComponent],
                                         [[self project] name],
                                         [xfast name]];
    [fileManager createDirectoryAtPath:xfastDirectoryInProject
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil];
}

/**
 *  Shows the Xfast content outline view.
 *
 *  @param key Xfast file key.
 */
- (void)showContentOfXfast:(NSString *)key
{
    // Read into the Xfast file.
    // Prepare the outline data for displaying.
    // Show the Xfast content.
    XCSourceFile *file = [[self project] fileWithKey:key];
    NSString *projectFilePath = [NSString stringWithFormat:@"%@/%@",
                                 [[self project] filePath], [file name]];
    
    _xfast = [XFProject projectWithFilePath:projectFilePath];
    _xfast.databaseDirectory = [[NSUserDefaults standardUserDefaults] stringForKey:DatabaseDirectory];
    _xfast.userName = [[NSUserDefaults standardUserDefaults] stringForKey:UserName];
    _xfast.projectDirectory = [[[self fileURL] path] stringByDeletingLastPathComponent];
//    _xfast.xcprojectFilename = [[self fileURL] lastPathComponent];
    
    _contentsOfXfastOutline = [[self xfast] asTreeData];
    
    [[self windowController] showContentOfXfast:_contentsOfXfastOutline];
}

/**
 *  Shows the content of the Xfast.
 *
 *  @param key A key of the selected node.
 */
- (void)showXfastProjectSettings:(NSString *)key
{
    // Decide the type of the file.
    XfastSourceFileType type = [_xfast fileTypeWithKey:key];
    if (type == XfastProject) {
        NSMutableDictionary *settings = [_xfast defaultBuildsettings];
        [[self windowController] showXfastProjectSettings:settings];
    } else if (type == XfastTarget) {
        // Show target settings.
        [_xfast setCurrentTargetKey:key];
        NSMutableDictionary *settings = [_xfast buildsettingsOfTargetWithKey:key];
        [[self windowController] showXfastProjectSettings:settings];
    } else if (type == XFTEXT) {
        // Show text file.
        NSString *text = [_xfast textWithKey:key];
        NSString *path = [_xfast pathWithKey:key];
        if (text) {
            [[self windowController] showXfastText:text withPath:path];
        } else {
            [[self windowController] showXfastText:@"Failed to parsing a file"
                                          withPath:@""];
        }
    } else {
        // Show nothing.
    }
}

- (void)pushContentXfast
{
//    NSMutableDictionary *settings = [_xfast defaultBuildsettings];
    
    [[self xfast] save];
}

- (void)saveText:(NSString *)text withPath:(NSString *)path
{
    NSError *err = nil;
    
    if(![text writeToFile:path
               atomically:YES
                 encoding:NSUTF8StringEncoding
                    error:&err]) {
    }
}

- (IBAction)runTarget:(id)sender
{
    NSLog(@"Document: runTarget");
    // Find the current Xfast.
    // Find the current target.
    
    // Find the build settings of the target.
    // Find the source build files in the target.
    // Find the outgut build files in the target.
    
    // Create an operation for the target.
    // Update the log view with the operation.
    // Submit the operation.
    // Notified of the complete of the operation.
    // Update the log view on the complete notification.
    
    NSString *type = [_xfast type];
    NSString *operationType = [_xfast operation];
    // currentTargetKey is a member of Xfast.
    NSString *currentTargetKey = [_xfast currentTargetKey];
    
    // This would be a list of build settings in dictionary.
    // Files must be in full path.
    NSDictionary *buildSettings = [_xfast buildsettingsOfTargetWithKey:currentTargetKey];
    XFTarget *currentTarget = [_xfast currentTarget];

    NSArray *sourceBuildFiles = [currentTarget sourceMembers]; // This would be a list of file names.
    NSArray *outputBuildFiles = [currentTarget outputMembers]; // This would be a list of file names.
    
    NSString *nameOfCommand = [NSString stringWithFormat:@"SCCCommandOf%@",
                               type];
    id<SCCCommand> aCommand = [[NSClassFromString(nameOfCommand) alloc] init];
    
    
    [aCommand setBuildSettings:buildSettings];
    [aCommand setSourceBuildFiles:sourceBuildFiles];
    [aCommand setOutputBuildFiles:outputBuildFiles];
    
    // Choose the type of an operation.
    
    NSString *nameOfOperation = [NSString stringWithFormat:@"SCCOperation%@",
                                 operationType];
    NSOperation *anOperation = [[NSClassFromString(nameOfOperation) alloc] initWithCommand:aCommand];
    
    

    // Update the log view.
    [[self windowController] addEntryWithProject:@"aaa" withXfast:@"bbb" withTarget:@"ccc" withStart:@"start" withEnd:@"end" withStatus:@"started"];
    
    [_queue addOperation:anOperation];
    
    // Later, update the log view,
    
    
}

#pragma mark - Xfast

/**
 *  Returns the type of the current Xfast.
 *
 *  @return The string of type of the current Xfast.
 */
- (NSString *)operationOfXfast
{
    if ([self xfast] == nil) {
        return nil;
    } else {
        return [[self xfast] operation];
    }
}

@end



































