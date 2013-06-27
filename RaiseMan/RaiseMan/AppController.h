//
//  AppController.h
//  RaiseMan
//
//  Created by Sang Chul Choi on 3/30/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import <Foundation/Foundation.h>

@class PreferenceController;

@interface AppController : NSObject
{
    PreferenceController *preferenceController;
}
- (IBAction)showPrefreencePanel:(id)sender;
@end
