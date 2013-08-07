//
//  SCRepositoryAnalysis.m
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCRepositoryAnalysis.h"

@implementation SCRepositoryAnalysis

@synthesize name;
@synthesize isChecked;
@synthesize delegate; //synthesise  MyClassDelegate delegate

- (void) myMethodToDoStuff {
    [self.delegate SCRepositoryAnalysisDelegateMethod:self];
}

@end
