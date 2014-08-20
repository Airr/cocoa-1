//
//  SCCApplicationDelegate.m
//  Xfast
//
//  Created by Sang Chul Choi on 7/30/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#import "SCCApplicationDelegate.h"
#import "SCCDefaultsKeys.h"

static NSDictionary *defaultValues() {
    static NSDictionary *dict = nil;
    if (!dict) {
        dict = [[NSDictionary alloc] initWithObjectsAndKeys:
                [NSNumber numberWithInteger:30], AutosavingDelay,
                [NSNumber numberWithBool:NO], NumberPagesWhenPrinting,
                [NSNumber numberWithBool:YES], WrapToFitWhenPrinting,
                [NSNumber numberWithBool:YES], RichText,
                [NSNumber numberWithBool:NO], ShowPageBreaks,
                [NSNumber numberWithBool:NO], OpenPanelFollowsMainWindow,
                [NSNumber numberWithBool:YES], AddExtensionToNewPlainTextFiles,
                [NSNumber numberWithInteger:90], WindowWidth,
                [NSNumber numberWithInteger:30], WindowHeight,
//                [NSNumber numberWithUnsignedInteger:NoStringEncoding], PlainTextEncodingForRead,
//                [NSNumber numberWithUnsignedInteger:NoStringEncoding], PlainTextEncodingForWrite,
                [NSNumber numberWithInteger:8], TabWidth,
                [NSNumber numberWithInteger:50000], ForegroundLayoutToIndex,
                [NSNumber numberWithBool:NO], IgnoreRichText,
                [NSNumber numberWithBool:NO], IgnoreHTML,
                [NSNumber numberWithBool:YES], CheckSpellingAsYouType,
                [NSNumber numberWithBool:NO], CheckGrammarWithSpelling,
                [NSNumber numberWithBool:[NSSpellChecker isAutomaticSpellingCorrectionEnabled]], CorrectSpellingAutomatically,
                [NSNumber numberWithBool:YES], ShowRuler,
                [NSNumber numberWithBool:YES], SmartCopyPaste,
                [NSNumber numberWithBool:NO], SmartQuotes,
                [NSNumber numberWithBool:NO], SmartDashes,
                [NSNumber numberWithBool:NO], SmartLinks,
                [NSNumber numberWithBool:NO], DataDetectors,
                [NSNumber numberWithBool:[NSSpellChecker isAutomaticTextReplacementEnabled]], TextReplacement,
                [NSNumber numberWithBool:NO], SubstitutionsEnabledInRichTextOnly,
                @"", AuthorProperty,
                @"", CompanyProperty,
                @"", CopyrightProperty,
                [NSNumber numberWithBool:NO], UseXHTMLDocType,
                [NSNumber numberWithBool:NO], UseTransitionalDocType,
                [NSNumber numberWithBool:YES], UseEmbeddedCSS,
                [NSNumber numberWithBool:NO], UseInlineCSS,
                [NSNumber numberWithUnsignedInteger:NSUTF8StringEncoding], HTMLEncoding,
                [NSNumber numberWithBool:YES], PreserveWhitespace,
                nil];
    }
    return dict;
}


@implementation SCCApplicationDelegate

+ (void)initialize {
    // Set up default values for preferences managed by NSUserDefaultsController
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues()];
    [[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:defaultValues()];
}


- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSLog(@"Started app");
}

//- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
//{
//    return NO;
//}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    NSArray* urls = [[NSDocumentController sharedDocumentController] recentDocumentURLs];
    if ([urls count] > 0){
        NSURL *lastURL= [urls objectAtIndex: 0];
        
        if ([[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:lastURL display:YES error:nil]){
            return NO;
        }
        
    }
    
    return YES;
}

@end
