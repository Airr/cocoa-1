//
//  SCCViewControllerTarget.h
//  Xfast
//
//  Created by Sang Chul Choi on 8/14/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MyIconViewBox : NSBox
@end

@interface MyScrollView : NSScrollView
{
    NSGradient *backgroundGradient;
}
@end

@interface SCCViewControllerTarget : NSViewController <NSCollectionViewDelegate>
{
    IBOutlet NSCollectionView *collectionView;
    IBOutlet NSArrayController *arrayController;
    NSMutableArray *images;
    
    NSUInteger sortingMode;
    BOOL alternateColors;
    
    NSArray *savedAlternateColors;
}

@property NSMutableArray *images;
@property (nonatomic, assign) NSUInteger sortingMode;
@property (nonatomic, assign) BOOL alternateColors;

- (NSString *)selectedName;

@end
