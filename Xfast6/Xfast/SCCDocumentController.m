
//  SCCDocumentController.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/30/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCDocumentController.h"
#import "xcode-editor/XcodeEditor.h"
#import "xfast-editor/XfastEditor.h"

@implementation SCCDocumentController


- (void)newDocument:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    [panel setMessage:@"Please select a path where to create Xfast project."];
    [panel setAllowsOtherFileTypes:YES];
    [panel setExtensionHidden:YES];
    [panel setCanCreateDirectories:YES];
    [panel setTitle:@"New Xfast Project"];
    NSInteger result = [panel runModal];
    NSError *error = nil;
    
    if (result == NSOKButton) {
        // Project base directory: /base
        NSString *projectBasePath = [[panel URL] path];
        // Take the name of the project: project
        NSString *projectName     = [projectBasePath lastPathComponent];
        // Project directory: /base/project
        NSString *projectMainPath = [projectBasePath stringByAppendingPathComponent:projectName];
        // Project package path: /base/project.xfastproj
        NSString *projectPath     = [projectMainPath stringByAppendingPathExtension:@"xfastproj"];
        
        NSURL *projectURL = [NSURL fileURLWithPath:projectPath isDirectory:YES];
        
        // Creat and save a Xfast project.
        XCProject *project    = [XCProject projectWithNewFilePath:projectBasePath];
        XCGroup *rootGroup    = [project rootGroup];
//        XCGroup *projectGroup = [rootGroup addGroupWithPath:[project name]];
        [rootGroup addGroupWithAlias:[project name]];
        [rootGroup addGroupWithAlias:@"Products"];
        [project save];
        
        [self openDocumentWithContentsOfURL:projectURL display:YES completionHandler:nil];
        
        if (error) {
            [NSApp presentError:error];
        }
    }
}


- (IBAction)aaaDocController
{
    NSLog(@"aaaDocController");
    
}

- (IBAction)aaaDocController:(id)sender
{
    NSLog(@"aaaDocController: %@", sender);
}

@end
