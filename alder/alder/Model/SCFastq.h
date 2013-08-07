//
//  SCFastq.h
//  
//
//  Created by Sang Chul Choi on 8/6/13.
//
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

@class Analysis;

@interface SCFastq : NSManagedObject

@property (nonatomic, retain) NSDate * created;
@property (nonatomic, retain) NSNumber * id;
@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSString * path;
@property (nonatomic, retain) Analysis *analysis;

@end
