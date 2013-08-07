//
//  SCRepositoryAnalysis.h
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SCRepositoryAnalysis;
@protocol SCRepositoryAnalysisDelegate
- (void) SCRepositoryAnalysisDelegateMethod: (SCRepositoryAnalysis *) sender;
@end



@interface SCRepositoryAnalysis : NSObject

@property NSString * name;
@property BOOL isChecked;
@property (nonatomic, weak) id <SCRepositoryAnalysisDelegate> delegate;

@end

