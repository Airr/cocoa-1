//
//  SCCViewControllerSettings.m
//  Xfast
//
//  Created by Sang Chul Choi on 8/19/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCViewControllerSettings.h"
#import "SCCConfiguration.h"
#import "SCCConfigurationGroup.h"
#import "XfastEditor.h"

@interface SCCViewControllerSettings ()

@property (weak) IBOutlet NSOutlineView *settingsOutlineView;
@property (weak) IBOutlet NSTableView *locationTableView;
@property (weak) IBOutlet NSTableView *optionTableView;

@property (readonly) NSArray *configurations;
@property XFProject* xfastProject;
//@property NSDictionary *buildSettings;

@end

@implementation SCCViewControllerSettings

//- (void)viewDidLoad {
//    [super viewDidLoad];
//    // Do view setup here.
////    XFBuildConfiguration *b = [_xfastProject defaultConfiguration];
////    _buildSettings = [b specifiedBuildSettings];
//}

- (void)loadView
{
    [super loadView];
    // The following codes should be in here rather in viewDidLoad.
    [self.settingsOutlineView setFloatsGroupRows:NO];
//    [[self settingsOutlineView] registerNib:[[NSNib alloc] initWithNibNamed:@"SCCTableViewSettingLocation" bundle:nil]
//                              forIdentifier:@"SCCTableViewSettingLocation"];
    [[self settingsOutlineView] registerNib:[[NSNib alloc] initWithNibNamed:@"SCCTableViewSettingOption" bundle:nil]
                              forIdentifier:@"SCCTableViewSettingOption"];
    [self.settingsOutlineView expandItem:nil expandChildren:YES];
}


- (id)initWithXFProject:(XFProject *)xfastProject
{
    self = [super initWithNibName:NSStringFromClass([SCCViewControllerSettings class])
                           bundle:nil];
    if (self) {
//        SCCConfigurationGroup *identity = [SCCConfigurationGroup configurationWithTitle:@"Run Locations"];
//        identity.children = @[
//                              [SCCConfiguration configurationWithNibName:@"SCCTableViewSettingLocation" height:200.f]
//                              ];
        
        SCCConfigurationGroup *fileTable = [SCCConfigurationGroup configurationWithTitle:@"Run Options"];
        fileTable.children = @[
                               [SCCConfiguration configurationWithNibName:@"SCCTableViewSettingOption" height:270.f]
                               ];
        
        _configurations = @[
//                            identity,
                            fileTable
                            ];
        _xfastProject = xfastProject;
    }
    return self;
}

#pragma mark - OutlineView


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
}

- (void) outlineView:(NSOutlineView *)outlineView
       didAddRowView:(NSTableRowView *)rowView
              forRow:(NSInteger)row
{

}

#pragma mark - TableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    XFTarget *target = [_xfastProject currentTarget];

//    if ([self locationTableView] == aTableView) {
//        
//    } else if ([self optionTableView] == aTableView) {
//        
//    }
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



@end
