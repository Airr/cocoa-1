//
//  SCCViewControllerModule.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/21/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCViewControllerModule.h"
#import "SCCConfiguration.h"
#import "SCCConfigurationGroup.h"
#import "XfastEditor.h"

@interface SCCViewControllerModule ()

@end

@implementation SCCViewControllerModule
{
	NSArray *_configurations;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (id)initWithXFProject:(XFProject *)xfastProject
{
    self = [super initWithNibName:@"SCCViewControllerModule" bundle:nil];
    if (self) {
        SCCConfigurationGroup *identity = [SCCConfigurationGroup configurationWithTitle:@"Identity and Type"];
		identity.children = @[
			[SCCConfiguration configurationWithNibName:@"SCCViewProjectIdenty" height:200.f]
		];
		
        SCCConfigurationGroup *fileTable = [SCCConfigurationGroup configurationWithTitle:@"Files"];
		fileTable.children = @[
			[SCCConfiguration configurationWithNibName:@"SCCViewFileTable" height:270.f]
		];
        
		_configurations = @[
			identity,
            fileTable,
			[SCCConfigurationGroup configurationWithTitle:@"Target Membership"]
		];
        _xfastProject = xfastProject;
    }
    return self;
}

- (void)loadView
{
    [super loadView];
    [self.configurationCollection setFloatsGroupRows:NO];
	[self.configurationCollection registerNib:[[NSNib alloc] initWithNibNamed:@"SCCViewProjectIdenty" bundle:nil] forIdentifier:@"SCCViewProjectIdenty"];
	[self.configurationCollection registerNib:[[NSNib alloc] initWithNibNamed:@"SCCViewFileTable" bundle:nil] forIdentifier:@"SCCViewFileTable"];
    [_configurationCollection expandItem:nil expandChildren:YES];
    [_version setStringValue:[_xfastProject name]];
}

#pragma mark - Outline view configuration

- (BOOL) outlineView:(NSOutlineView *)outlineView
         isGroupItem:(id)item
{
	id representedObject = [item representedObject];
	
	return [representedObject isKindOfClass:[SCCConfigurationGroup class]];
}

- (NSView *) outlineView:(NSOutlineView *)outlineView
      viewForTableColumn:(NSTableColumn *)tableColumn
                    item:(id)item
{
	if ([self outlineView:outlineView isGroupItem:item]) {
		return [outlineView makeViewWithIdentifier:@"HeaderView" owner:self];
	}
	
	SCCConfiguration *config = [item representedObject];
	if (!config.nibName) {
		return [outlineView makeViewWithIdentifier:@"DefaultCell" owner:self];
	}
	
	return [outlineView makeViewWithIdentifier:config.nibName owner:self];	
}

- (CGFloat) outlineView:(NSOutlineView *)outlineView
      heightOfRowByItem:(id)item
{
	if ([self outlineView:outlineView isGroupItem:item]) {
		return 18.f;
	}
	
	SCCConfiguration *config = [item representedObject];
	if (config.nibName) {
		return config.height;
	}
	
	return 27.0f;
}

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView
                  rowViewForItem:(id)item
{
    return nil;
//	if (![self outlineView:outlineView isGroupItem:item]) {
//		return nil;
//	}
//	
//	SCCConfigurationHeaderRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:nil];
//	if (!rowView) {
//		rowView = [[SCCConfigurationHeaderRowView alloc] initWithFrame:CGRectZero];
//		rowView.identifier = @"HeaderRowView";
//	}
//	
//	return rowView;
}

- (void) outlineView:(NSOutlineView *)outlineView
       didAddRowView:(NSTableRowView *)rowView
              forRow:(NSInteger)row
{
//	NSInteger nearestGroup = row;
//	while (nearestGroup >= 0) {
//		if ([self outlineView:outlineView isGroupItem:[outlineView itemAtRow:nearestGroup]]) {
//			break;
//		}
//		
//		nearestGroup -= 1;
//	}
//	
//	NSInteger rowIndex = row - nearestGroup;
//	
//	if (rowIndex % 2 == 0) {
//		rowView.backgroundColor = [NSColor colorWithDeviceWhite:0.87f alpha:1.0f];
//	} else {
//		rowView.backgroundColor = [NSColor colorWithDeviceWhite:0.915f alpha:1.0f];
//	}
}

#pragma mark - TableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    
    XFTarget *target = [_xfastProject currentTarget];
    return [[target members] count];
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row
{
    
    XFTarget *target = [_xfastProject currentTarget];
    NSString *identifier = [tableColumn identifier];
    NSTableCellView *result = [tableView makeViewWithIdentifier:identifier owner:self];
    result.textField.stringValue = [[[target members] objectAtIndex:row] displayName];
    result.imageView.objectValue = [[[target members] objectAtIndex:row] displayImage];;
    return result;
}



#pragma mark - Buttons

// NOTE: sheets could (or should) be in the same NIB file as the current
//       controller's NIB file. I cannot call endsheet.
//       I need to refresh the File Table, but I cannot do that because of
//       this.
//       How can I 
- (IBAction)addFiles:(id)sender
{
   
    if (_addFilesWindowController == nil) {
        _addFilesWindowController = [SCCAddFilesWindowController new];
        [_addFilesWindowController setXfastProject:_xfastProject];
        [_addFilesWindowController addObserver:self
                                    forKeyPath:@"xfastProject"
                                       options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                                       context:NULL];
    }
    
    [[[self view] window] beginSheet:[_addFilesWindowController window] completionHandler:^(NSModalResponse returnCode)
     {
         NSLog(@"END: add files complete handler");
     }];
    NSLog(@"END: add files");
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
    
    if ([keyPath isEqual:@"xfastProject"]) {
        NSLog(@"KVO: xfastProject to Module.m");
        
        [_fileTable reloadData];
    }
    /*
     Be sure to call the superclass's implementation *if it implements it*.
     NSObject does not implement the method.
     */
//    [super observeValueForKeyPath:keyPath
//                         ofObject:object
//                           change:change
//                          context:context];
}

- (IBAction)removeFiles:(id)sender
{
    
}



@end

