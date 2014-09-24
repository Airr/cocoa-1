//
//  main.m
//  Xfast-pbxproj
//
//  Created by Sang Chul Choi on 7/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "XcodeEditor.h"
void getNameAndPath(NSString **projectName_p, NSString **projectFilePath_p)
{
    NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads", NSHomeDirectory()];
    *projectName_p = @"0";
    *projectFilePath_p = [NSString stringWithFormat:@"%@/%@/%@.xfastproj", downloadPath, *projectName_p, *projectName_p];
}

XCProject * getProject(void)
{
    NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads", NSHomeDirectory()];
    NSString *projectName = @"0";
    NSString *projectFilePath = [NSString stringWithFormat:@"%@/%@/%@.xfastproj", downloadPath, projectName, projectName];
    XCProject *project = [XCProject projectWithFilePath:projectFilePath];
    if (project == nil) {
        [NSException raise:NSGenericException format:@"Error: Create project failed %@", projectFilePath];
    }
    return project;
}

int main(int argc, const char * argv[])
{
    
    // Test 1. Create a Xfast projcet.
    @autoreleasepool {
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *path = [bundle pathForResource:@"cat" ofType: @"dict"];
        
        NSString *downloadPath = [NSString stringWithFormat:@"%@/Downloads", NSHomeDirectory()];
        NSString *projectName = @"0";
        NSString *projectPath = [NSString stringWithFormat:@"%@/%@", downloadPath, projectName];
        XCProject *project = [XCProject projectWithNewFilePath:projectPath ];
        if (project == nil) {
            [NSException raise:NSGenericException format:@"Error: Create project failed %@", projectPath];
        }
        
        [project save];
    }
    
    // 2. Read the Xfast projcet, and add groups.
    @autoreleasepool {
        XCProject *project = getProject();
        
        XCGroup *rootGroup = [project rootGroup];
        XCGroup *projectGroup = [rootGroup addGroupWithPath:[project name]];
        XCGroup *mainGroup = [projectGroup addGroupWithAlias:[project name]];
        XCGroup *modelGroup = [mainGroup addGroupWithAlias:@"Model"];
        XCGroup *xeGroup = [modelGroup addGroupWithPath:@"xcode-editor"];
        XCGroup *xe2Group = [mainGroup addGroupWithPath:@"xcode2-path"];
        XCGroup *productGroup = [projectGroup addGroupWithAlias:@"Products"];
        
        NSLog(@"rootGroup path: %@", [rootGroup pathRelativeToProjectRoot]);
        NSLog(@"projectGroup path: %@", [projectGroup pathRelativeToProjectRoot]);
        NSLog(@"mainGroup path: %@", [mainGroup pathRelativeToProjectRoot]);
        NSLog(@"modelGroup path: %@", [modelGroup pathRelativeToProjectRoot]);
        NSLog(@"xeGroup path: %@", [xeGroup pathRelativeToProjectRoot]);
        NSLog(@"xe2Group path: %@", [xe2Group pathRelativeToProjectRoot]);
        NSLog(@"productGroup path: %@", [productGroup pathRelativeToProjectRoot]);
        [project save];
    }
    
    // 3. Read the Xfast project, and change a build setting in a configuration.
    @autoreleasepool {
        XCProject *project = getProject();
        
        XCBuildConfiguration *buildConf = [project defaultConfiguration];
        [buildConf addOrReplaceSetting:@"YES" forKey:@"ALWAYS_SEARCH_USER_PATHS"];
        
        [project save];
    }
    
    // 4. Add a new configuration.
    @autoreleasepool {
        XCProject *project = getProject();

        
        XCBuildConfiguration *buildConf = [[XCBuildConfiguration alloc] initWithProject:project
                                                                                    key:[[XCKeyBuilder createUnique] build]
                                                                                   name:@"Debug"];
        [project addBuildConfiguration:buildConf];
        // Add the build setting to the project. The configration needs a name.
        [buildConf addOrReplaceSetting:@"NO" forKey:@"ALWAYS_SEARCH_USER_PATHS"];
        
        [project save];
    }
    
    // 5. Add a new target: source build phase, build configuration,
    //    build conf list, and a target.
    @autoreleasepool {
        XCProject *project = getProject();
        
        // Add two groups and the project group first.
        // Then, call [project addTarge:@"target1"];
        [project addTarget:@"target1"];
        
        [project save];
    }
    
    // 6. Add a file to the project.
    //    A file reference and it belongs to a group.
    @autoreleasepool {
        XCProject *project = getProject();
        XCGroup *projectGroup = [project groupWithPathFromRoot:@"0"];
        XCGroup *mainGroup = [project groupWithPathFromRoot:@"0/0"];
        XCGroup *productGroup = [project groupWithPathFromRoot:@"0/Products"];
        
        XCSourceFileDefinition* readmeFile = [[XCSourceFileDefinition alloc]
                                              initWithName:@"README"
                                              text:@""
                                              type:TEXT];
        [projectGroup addSourceFile:readmeFile];
        
        [project save];
    }
    
    // 7. Add a file to a target.
    @autoreleasepool {
        XCProject *project = getProject();
        XCSourceFile* sourceFile = [project fileWithName:@"README"];
        XCTarget* examples = [project targetWithName:@"target1"];
        [examples addMember:sourceFile];
        
        [project save];
    }
    
    // 8. Add a Xfast to the project.
    @autoreleasepool {
        XCProject *project = getProject();
        XCGroup *modelGroup = [project groupWithPathFromRoot:@"0/0/Model"];
        
        XCSourceFileDefinition* xfast = [[XCSourceFileDefinition alloc]
                                         initWithName:@"xfast1.xfast"
                                         text:@""
                                         type:SourceCodeXfast];
        [modelGroup addSourceFile:xfast];
        
        [project save];
    }
    
    // 9. Create a Xfast-Target (a build file for a Xfast with target name).
    //    Add the Xfast-Target.
    @autoreleasepool {
        XCProject *project = getProject();
        XCSourceFile* sourceFile = [project fileWithName:@"xfast1.xfast"];
        XCTarget* examples = [project targetWithName:@"target1"];
        [examples addMember:sourceFile target:@"xfast1.Target1"];
        
        [project save];
    }
    
    // 10. Tree view
    @autoreleasepool {
        XCProject *project = getProject();
        [project printTree];
    }
 
    
    @autoreleasepool {
        NSString *path1 = @"/0/1/file";
        NSString *path2 = @"/1/file";
        NSString *path3 = [XCProject relativePath:path1 relativeTo:path2];
        NSLog(@"path is: %@", path3);
    }
    
    // 11. Move a file from a group to another group.
    //     Remove it from a group, and add it to another.
    //     Change fileRef's path.
    //
    @autoreleasepool {
        XCProject *project = getProject();
        XCSourceFile* sourceFile = [project fileWithName:@"xfast1.xfast"];
        NSString *pathSourceFile = [sourceFile pathRelativeToProjectRoot];
        NSLog(@"xfast1.xfast path: %@", [sourceFile pathRelativeToProjectRoot]);
        // Find the parent group.
        XCGroup *fromGroup = [project groupWithSourceFile:sourceFile];
        // Remove it from the parent group.
        [fromGroup deleteGroupMemberWithKey:[sourceFile key]];
        // Add it to another group.
        XCGroup *toGroup = [project groupWithPathFromRoot:@"0/0/Model/xcode-editor"];
        [toGroup insertGroupMemberWithKey:[sourceFile key]];
        // Change fileRef.
        NSLog(@"Wrong Path -> xfast1.xfast path: %@", [sourceFile pathRelativeToProjectRoot]);
        NSString *wrongPathSourceFile = [sourceFile pathRelativeToProjectRoot];
        
        NSString *adjustedPathSourceFile = [XCProject relativePath:pathSourceFile relativeTo:wrongPathSourceFile];
        NSLog(@"Adjusted Path is: %@", adjustedPathSourceFile);
//        [project printTree];
        [sourceFile setPath:adjustedPathSourceFile];
        NSLog(@"Correct Path -> xfast1.xfast path: %@", [sourceFile pathRelativeToProjectRoot]);
        
        [project save];
        [project printTree];
    }
    
    // 12. Move a group from a group to another group.
    @autoreleasepool {
        XCProject *project = getProject();
        XCGroup *model = [project groupWithPathFromRoot:@"0/0/Model"];
        NSString *fromPath = [model pathRelativeToProjectRoot];
        XCGroup *fromGroup = [model parentGroup];
        [fromGroup deleteGroupMemberWithKey:[model key]];
        XCGroup *toGroup = [project groupWithPathFromRoot:@"0/Products"];
        [toGroup insertGroupMemberWithKey:[model key]];
        // Change group ref.
        NSString *toPath = [model pathRelativeToProjectRoot];
        NSLog(@"Old: %@", fromPath);
        NSLog(@"New: %@", toPath);
        
        [project save];
        [project printTree];
        
        fromGroup = toGroup;
        NSString *fromPath2 = toPath;
        toGroup = [project groupWithPathFromRoot:@"0/0/xcode2-path"];
        [model flagPathRelativeToProjectRootAsDirty];
        [fromGroup deleteGroupMemberWithKey:[model key]];
        [toGroup insertGroupMemberWithKey:[model key]];
        NSString *toPath2 = [model pathRelativeToProjectRoot];
        NSLog(@"Old: %@", fromPath2);
        NSLog(@"New: %@", toPath2);
        
        NSString *adjustedPath = [XCProject relativeDir:fromPath2 relativeTo:toPath2];
        NSLog(@"Adjusted: %@", adjustedPath);
        [model setPathRelativeToParent:adjustedPath];
        [model setAlias:@"Model"];
        [model flagPathRelativeToProjectRootAsDirty];
        [project setObject:model];
        
        [project save];
        [project printTree];
        
        XCGroup *xcodeEditorGroup = [project groupWithPathFromRoot:@"0/0/xcode2-path/Model/xcode-editor"];
        XCSourceFile *file = [project fileWithName:@"xfast1.xfast"];
        
        NSLog(@"xfast1.xfast path: %@", [file pathRelativeToProjectRoot]);
        NSLog(@"xcode-editor path: %@", [xcodeEditorGroup pathRelativeToProjectRoot]);
        
        NSLog(@"modelGroup path: %@", [model pathRelativeToProjectRoot]);
        NSLog(@"xe2Group path: %@", [toGroup pathRelativeToProjectRoot]);
    }
    
    // 13. Delete a file
    
    
    // 14. Delete a target
    
    // 15. Delete a group
    
    // 16. Remove all group
    @autoreleasepool {
        XCProject *project = getProject();
        [project removeAllGroup];
        
        [project save];
    }
    
    
    

    return 0;
}

