//
//  SCConfigurationViewController.m
//  alder
//
//  Created by Sang Chul Choi on 8/5/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#import "SCConfigurationViewController.h"
#import "SCFastq.h"

@interface SCConfigurationViewController ()
- (IBAction)add:(id)sender;
@end

@implementation SCConfigurationViewController

@synthesize managedObjectContext;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

// Memory leaks in openPanel.
- (IBAction)add:(id)sender
{
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
	
	NSArray* fileTypes = [NSArray arrayWithObjects:@"gz", @"fq", @"fastq", nil];
	[openPanel setAllowsMultipleSelection:NO];
	[openPanel setMessage:@"Choose a FASTQ file:"];
    [openPanel setAllowedFileTypes:fileTypes];
    [openPanel setDirectoryURL:[NSURL fileURLWithPath:@"/Users/goshng"]];
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        
        if (result == NSOKButton || result == NSFileHandlingPanelOKButton)
        {
            if ([[openPanel URL] isFileURL])
            {
//                NSLog(@"%@",[[openPanel URL] path]);
                // Add the file to the list of FASTQ files using managedObjectContext.
                NSManagedObjectContext *moc = managedObjectContext;
                NSPersistentStoreCoordinator *psc = [moc persistentStoreCoordinator];
                NSManagedObjectModel *mom = [psc managedObjectModel];
                NSEntityDescription *fastqEntity = [[mom entitiesByName] objectForKey:@"SCFastq"];
                SCFastq *fastq = [[SCFastq alloc]
                                  initWithEntity:fastqEntity
                                  insertIntoManagedObjectContext:moc];
                fastq.name = [[openPanel URL] path];

                NSArray *psa = [psc persistentStores];
                NSUInteger c = [psa count];
                /*
                 Save the context to commit the new Run instance to the persistent store.
                 */
                NSError *error = nil;
                if (c > 0 && ![moc save: &error]) {
                    NSLog(@"Error while saving\n%@",
                          ([error localizedDescription] != nil) ? [error localizedDescription] : @"Unknown Error");
                }
            }
        }
        
    }];

}
//
//- (IBAction)add:(id)sender
//{
//                //                NSLog(@"%@",[[openPanel URL] path]);
//                // Add the file to the list of FASTQ files using managedObjectContext.
//                NSManagedObjectContext *moc = managedObjectContext;
//                NSPersistentStoreCoordinator *psc = [moc persistentStoreCoordinator];
//                NSManagedObjectModel *mom = [psc managedObjectModel];
//                NSEntityDescription *fastqEntity = [[mom entitiesByName] objectForKey:@"SCFastq"];
//                SCFastq *fastq = [[SCFastq alloc]
//                                  initWithEntity:fastqEntity
//                                  insertIntoManagedObjectContext:moc];
//                fastq.name = @"aaaaa";
//                
//                NSArray *psa = [psc persistentStores];
//                NSUInteger c = [psa count];
//                /*
//                 Save the context to commit the new Run instance to the persistent store.
//                 */
//                NSError *error = nil;
//                if (c > 0 && ![moc save: &error]) {
//                    NSLog(@"Error while saving\n%@",
//                          ([error localizedDescription] != nil) ? [error localizedDescription] : @"Unknown Error");
//                }
//    
//}



- (IBAction)updateManagedObjectContext:(id)sender
{
//    NSLog(@"updateMOC");
    /*
     Save the context to commit the new Run instance to the persistent store.
     */
    NSError *error = nil;
    
    
    NSPersistentStoreCoordinator *psc = [managedObjectContext persistentStoreCoordinator];
    NSArray *psa = [psc persistentStores];
    NSUInteger c = [psa count];
    
    if (c > 0 && ![managedObjectContext save: &error]) {
        NSLog(@"Error while saving\n%@",
              ([error localizedDescription] != nil) ? [error localizedDescription] : @"Unknown Error");
    }
}

@end
