//
//  SCAnalysis.h
//  alder
//
//  Created by Sang Chul Choi on 8/6/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class SCFastq;

@interface SCAnalysis : NSManagedObject

@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSString * options;
@property (nonatomic, retain) NSString * program;
@property (nonatomic, retain) NSSet *fastq;
@end

@interface SCAnalysis (CoreDataGeneratedAccessors)

- (void)addFastqObject:(SCFastq *)value;
- (void)removeFastqObject:(SCFastq *)value;
- (void)addFastq:(NSSet *)values;
- (void)removeFastq:(NSSet *)values;

@end
