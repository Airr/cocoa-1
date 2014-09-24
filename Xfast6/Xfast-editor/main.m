//
//  main.m
//  Xfast-editor
//
//  Created by Sang Chul Choi on 7/23/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "XfastEditor.h"

XFProject * getProject(void)
{
//    NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads", NSHomeDirectory()];
//    NSString *projectName = @"00.xfast";
//    NSString *projectPath = [NSString stringWithFormat:@"%@/%@", downloadPath, projectName];
    
    NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads/0/0.xfastproj", NSHomeDirectory()];
    NSString *projectName = @"Xfast1.cat.xfast";
    NSString *projectPath = [NSString stringWithFormat:@"%@/%@", downloadPath, projectName];
    XFProject *project = [XFProject projectWithFilePath:projectPath];
    if (project == nil) {
        [NSException raise:NSGenericException format:@"Error: Create project failed %@", projectPath];
    }
    return project;
}

int main(int argc, const char * argv[])
{
    // Test 1. Create a Xfast projcet.
    @autoreleasepool {
//        NSBundle *bundle = [NSBundle mainBundle];
        NSString *path = @"/Users/goshng/Dropbox/Documents/Projects/cocoa/Xfast6/Xfast/cat.dict";
        
        NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads/0/0.xfastproj", NSHomeDirectory()];
        NSString *projectName = @"Xfast1.cat.xfast";
        NSString *projectPath = [NSString stringWithFormat:@"%@/%@", downloadPath, projectName];
        XFProject *project = [XFProject projectWithNewFilePath:projectPath withTemplate:path];
        if (project == nil) {
            [NSException raise:NSGenericException format:@"Error: Create project failed %@", projectPath];
        }
        
        [project setProjectDirectory:@"/Users/goshng/Downloads/0"];
        [project setDatabaseDirectory:@"/Users/goshng/Downloads/0database"];
        [project setUserName:@"goshng"];
        
        [project save];
    }

    // Read the Xfast file
    @autoreleasepool {
        XFProject *project = getProject();
        
        [project print];
        [project save];
    }

    
    // 2. Read the Xfast file, and add files.
    @autoreleasepool {
        XFProject *project = getProject();
        
        
//        [project addFileOfType:@"text" withAbsolutePath:@"/path/to/README"];
//        [project addFileOfType:@"fasta" withXfastPath:@"target.fa"];
//        [project addFileOfType:@"fastq" withDataPath:@"target.fq"];
//        [project addFileOfType:@"folder" withDataPath:@"dir"];
//        
//        [project addOutFileOfType:@"text" withAbsolutePath:@"/path/to/README"];
//        [project addOutFileOfType:@"fasta" withXfastPath:@"target.fa"];
//        [project addOutFileOfType:@"fastq" withDataPath:@"target.fq"];
//        [project addOutFileOfType:@"folder" withDataPath:@"dir"];
        [project addFileOfType:@"text"
                      withPath:@"README"
                      withUser:@"goshng"
                   withProject:@"project-name"
                     withXfast:@"xfast-name"
                    withSource:@"<xfast>"
                withMemberType:XFPBXOutFileReferenceType
                       withKey:nil];
        
        
        
        [project print];
        [project save];
    }

    // Add a target.
    @autoreleasepool {
        XFProject *project = getProject();
        [project addTargetOperationOfManyToOne:@"Target1"];
        [project print];
        [project save];
    }
    
    
    // Add three input files.
    @autoreleasepool {
        XFProject *project = getProject();
        XFTarget *target = [project targetWithName:@"Target1"];
        
        // Add files.
        [project addFileOfType:@"text"
                      withPath:@"input1.txt"
                      withUser:@"goshng"
                   withProject:@"project-name"
                     withXfast:@"xfast-name"
                    withSource:@"<xfast>"
                withMemberType:XFPBXFileReferenceType
                       withKey:nil];
        [project addFileOfType:@"text"
                      withPath:@"input2.txt"
                      withUser:@"goshng"
                   withProject:@"project-name"
                     withXfast:@"xfast-name"
                    withSource:@"<data>"
                withMemberType:XFPBXFileReferenceType
                       withKey:nil];
        [project addFileOfType:@"text"
                      withPath:@"/input3.txt"
                      withUser:@"goshng"
                   withProject:@"project-name"
                     withXfast:@"xfast-name"
                    withSource:@"<absolute>"
                withMemberType:XFPBXFileReferenceType
                       withKey:nil];
        
        // Retrieve the files and add them to the target.
        XFSourceFile *file1 = [project xfastFileWithPath:@"input1.txt" withProject:@"project-name" withXfast:@"xfast-name"];
        
        XFSourceFile *file2 = [project dataFileWithPath:@"input2.txt"
                                               withUser:@"goshng"
                                            withProject:@"project-name"
                                              withXfast:@"xfast-name"];
        XFSourceFile *file3 = [project absoluteFileWithPath:@"/input3.txt"];
        
        [target addSourceMember:file1];
        [target addSourceMember:file2];
        [target addSourceMember:file3];

        [project print];
        [project save];
    }
    
    
    
    // Add a target.
    @autoreleasepool {
        XFProject *project = getProject();
        
        [project addTarget:@"Target2"];

        [project save];
    }
    
    
    
    
    
    // Add a reference file or folder to a target.
    @autoreleasepool {
        XFProject *project = getProject();
        XFTarget *target = [project targetWithName:@"Target1"];
        
        XFSourceFile *file = [project fileWithName:@"README"];
        XFSourceFile *outfile = [project fileWithName:@"target.fa"];
        [target addSourceMember:file];
        [target addOutputMember:outfile];
        [project save];
    }
    
    // List Targets
    @autoreleasepool {
        XFProject *project = getProject();
        for (XFTarget *target in [project targets]) {
            NSLog(@"Target Name: %@", [target displayName]);
            for (XFSourceFile *file in [target sourceMembers]) {
                NSLog(@"\tsource file: %@", [file displayName]);
            }
            for (XFSourceFile *file in [target outputMembers]) {
                NSLog(@"\toutput file: %@", [file displayName]);
            }
        }
        [project save];
    }
    
    // List reference files.
    @autoreleasepool {
        XFProject *project = getProject();
        for (XFSourceFile *file in [project files]) {
            NSLog(@"File Name: %@", [file displayName]);
        }
        [project save];
    }
    
    // Remove a source file from a tarce.
    @autoreleasepool {
        XFProject *project = getProject();
        XFTarget *target = [project targetWithName:@"Target1"];
        XFSourceFile *file = [project fileWithName:@"README"];
        XFSourceFile *outfile = [project fileWithName:@"target.fa"];
        [target removeSourceMemberWithKey:[file key]];
        [target removeOutputMemberWithKey:[outfile key]];
        [project save];
    }
    
    // Remove a targe.
    @autoreleasepool {
        XFProject *project = getProject();
        [project removeTarget:@"Target1"];
        [project removeTarget:@"Target2"];
        [project save];
    }
    
    return 0;
}









































