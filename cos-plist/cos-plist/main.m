//
//  main.m
//  cos-plist
//
//  Created by Sang Chul Choi on 7/18/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

int main(int argc, const char * argv[])
{

    @autoreleasepool {
        NSMutableDictionary *rootObj = [NSMutableDictionary dictionaryWithCapacity:2];
        NSDictionary *innerDict;
        NSString *name;
        NSDate *dob;
        NSArray *scores;
        
        scores = [NSArray arrayWithObjects:[NSNumber numberWithInt:6],
                  [NSNumber numberWithFloat:4.6], [NSNumber numberWithLong:6.0000034], nil];
        name = @"George Washington";
        dob = [NSDate dateWithString:@"1732-02-17 04:32:00 +0300"];
        innerDict = [NSDictionary dictionaryWithObjects:
                     [NSArray arrayWithObjects: name, dob, scores, nil]
                                                forKeys:[NSArray arrayWithObjects:@"Name", @"DOB", @"Scores", nil]];
        [rootObj setObject:innerDict forKey:@"Washington"];
        
        scores = [NSArray arrayWithObjects:[NSNumber numberWithInt:8],
                  [NSNumber numberWithFloat:4.9],
                  [NSNumber numberWithLong:9.003433], nil];
        name = @"Abraham Lincoln";
        dob = [NSDate dateWithString:@"1809-02-12 13:18:00 +0400"];
        innerDict = [NSDictionary dictionaryWithObjects:
                     [NSArray arrayWithObjects: name, dob, scores, nil]
                                                forKeys:[NSArray arrayWithObjects:@"Name", @"DOB", @"Scores", nil]];
        [rootObj setObject:innerDict forKey:@"Lincoln"];
        
        NSOutputStream *ofile = [NSOutputStream outputStreamToFileAtPath:@"./a.plist" append:NO];
        [ofile open];
        NSError *error;
        NSInteger nbyte = [NSPropertyListSerialization writePropertyList:(id)rootObj toStream:ofile format:NSPropertyListXMLFormat_v1_0 options:0 error:&error];
        // Tried this because I did not konw how to use NSOutputStream
//        NSString *errorstr;
//        NSData *data = [NSPropertyListSerialization dataFromPropertyList:(id)rootObj format:NSPropertyListXMLFormat_v1_0 errorDescription:&errorstr];
        // Deprecated
//        id plist = [NSPropertyListSerialization dataFromPropertyList:(id)rootObj
//                                                              format:NSPropertyListXMLFormat_v1_0 errorDescription:&error];
        
        NSLog(@"%@: %lu: %@", ofile, nbyte, error);
        [ofile close];
        
        NSInputStream *ifile = [NSInputStream inputStreamWithFileAtPath:@"./a.plist"];
        [ifile open];
        
        NSMutableDictionary *rootDic;
        rootDic =[NSPropertyListSerialization propertyListWithStream:ifile options:0 format:NULL error:&error];
        [ifile close];
        
    }
    return 0;
}

