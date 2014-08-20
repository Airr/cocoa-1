
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

//- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
//{
//    NSLog(@"type - %@\n", inAbsoluteURL);
////    return @"xfastproj";
//    return @"SCCDocument";
//}

- (void)newDocument:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    
    [panel setMessage:@"Please select a path where to create Xfast project."]; // Message inside modal window
    [panel setAllowsOtherFileTypes:YES];
    [panel setExtensionHidden:YES];
    [panel setCanCreateDirectories:YES];
    [panel setTitle:@"New Xfast Project"]; // Window title
    
    NSInteger result = [panel runModal];
    NSError *error = nil;
    
    if (result == NSOKButton) {
        NSString *projectBasePath = [[panel URL] path];
        
        NSString *projectName = [projectBasePath lastPathComponent];
        NSString *projectMainPath = [projectBasePath stringByAppendingPathComponent:projectName];
        NSString *projectXfastPath = [projectBasePath stringByAppendingPathComponent:@"xfastfolder"];
        NSString *projectPath = [projectMainPath stringByAppendingPathExtension:@"xfastproj"];
        NSURL *projectURL = [NSURL fileURLWithPath:projectPath isDirectory:YES];
        
        XCProject *project = [[XCProject alloc] initWithNewFilePath:projectBasePath];
        XCGroup *group = [project rootGroup];
        XCGroup *group1 = [group addGroupWithPath:projectName];
        XCGroup *group11 = [group1 addGroupWithPath:@"Jobs"];
        [group1 addGroupWithPath:@"Products"];
        [project save];

        /* Welcome xfast */
        NSString *welcomePath = [projectPath stringByAppendingPathComponent:@"Welcome"];
        XFProject *xfast = [XFProject projectWithNewFilePath:welcomePath
                                               baseDirectory:projectXfastPath];
        NSString *welcomeXfastPath = [welcomePath stringByAppendingPathComponent:@"Welcome.xfast/project.pbxproj"];
        XFGroup *groupXfast = [xfast rootGroup];
        XFGroup *groupXfast1 = [groupXfast addGroupWithPath:@"Welcome"];
        [groupXfast1 addGroupWithPath:@"Targets"];
        [groupXfast1 addGroupWithPath:@"Files"];
        [xfast saveToPBXProj];
        
        
        NSData *xfastData = [NSData dataWithContentsOfFile:welcomeXfastPath];
        XCSourceFileDefinition* header = [[XCSourceFileDefinition alloc]
                                          initWithName:@"Welcome.xfast"
                                          data:xfastData
                                          type:SourceCodeXfast];
        [group11 addSourceFile:header];
        [project save];
        
        [self openDocumentWithContentsOfURL:projectURL display:YES completionHandler:nil];
        
//        [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:projectURL
//                                                                               display:YES
//                                                                                 error:&error];
        
        if (error) {
            [NSApp presentError:error];
        }
    }
}

@end
