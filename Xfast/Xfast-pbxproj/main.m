//
//  main.m
//  Xfast-pbxproj
//
//  Created by Sang Chul Choi on 7/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "XcodeEditor.h"
int main(int argc, const char * argv[])
{

    @autoreleasepool {
        NSError *error;
//        NSInputStream *ifile = [NSInputStream inputStreamWithFileAtPath:@"/Users/goshng/Dropbox/Documents/Projects/cocoa/Xfast/Xfast/test.plist"];
        NSInputStream *ifile = [NSInputStream inputStreamWithFileAtPath:@"/Users/goshng/Dropbox/Documents/Projects/cocoa/Xfast/Xfast.xcodeproj/project.pbxproj"];
        [ifile open];
        
        NSMutableDictionary *rootDic;
        rootDic =[NSPropertyListSerialization propertyListWithStream:ifile options:0 format:NULL error:&error];
        [ifile close];
        
        XCProject* project = [[XCProject alloc] initWithFilePath:@"MyProject.xcodeproj"];
        XCGroup* group = [project groupWithPathFromRoot:@"Main"];
        XCClassDefinition* classDefinition = [[XCClassDefinition alloc] initWithName:@"MyNewClass"];
        [classDefinition setHeader:@"<some-header-text>"];
        [classDefinition setSource:@"<some-impl-text>"];
        
        [group addClass:classDefinition];
        
        NSArray* objcFiles = [project objectiveCFiles];
        NSLog(@"Implementation Files: %@", objcFiles);
        
        NSArray* groups = [project groups];
        
        for (XCGroup* group in groups)
        {
            NSLog(@"Name: %@, full path: %@", [group displayName], [group pathRelativeToProjectRoot]);
            for (id <XcodeGroupMember> member  in [group members])
            {
                NSLog(@"\t%@", [member displayName]);
            }
        }
        
        XCSourceFile* sourceFile = [project fileWithName:@"MyNewClass.m"];
        XCTarget* examples = [project targetWithName:@"Examples"];
        [examples addMember:sourceFile];
        
        
        group = [project rootGroup];
        NSLog(@"Here the group: %@", group);
        NSLog(@"Name: %@, full path: %@", [group displayName], [group pathRelativeToProjectRoot]);
        for (id <XcodeGroupMember> member  in [group members])
        {
            NSLog(@"\t%@", [member displayName]);
        }
        
        [project save];
        
    }
    return 0;
}

