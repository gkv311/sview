/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#include <StFile/StFileNode.h>
#include <StFile/StMIMEList.h>

#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

@interface StOpenFileInfo : NSObject
    {
        NSMutableArray* myFilter;
        NSString*       myTitle;
        NSString*       myFolder;
        NSString*       myInitFileName;
        @public
        StString        myResult;
        bool            myToSave;
        @public
        bool            myIsFileSelected;
    }

    - (id ) init: (NSMutableArray* ) theFilter
       withTitle: (NSString* )       theTitle
      withFolder: (NSString* )       theFolder
        withFile: (NSString* )       theInitFileName
          toSave: (bool )            theToSave;

    - (void ) doDialog: (id ) theSender;
@end

@implementation StOpenFileInfo

    - (id ) init: (NSMutableArray* ) theFilter
       withTitle: (NSString* )       theTitle
      withFolder: (NSString* )       theFolder
        withFile: (NSString* )       theInitFileName
          toSave: (bool )            theToSave {
        self = [super init];
        if(self == NULL) {
            return NULL;
        }
        myFilter = theFilter;
        myTitle  = theTitle;
        myFolder = theFolder;
        myInitFileName = theInitFileName;
        myToSave = theToSave;
        myIsFileSelected = false;
        return self;
    }

    - (void ) doDialog: (id ) theSender {
        StCocoaLocalPool aLocalPool;
        NSSavePanel* aFilePanel = myToSave ? [NSSavePanel savePanel] : [NSOpenPanel openPanel];
        [aFilePanel setTitle: myTitle];
        [aFilePanel setNameFieldStringValue: myInitFileName];
        [aFilePanel setDirectoryURL: [NSURL fileURLWithPath: myFolder]];

        if(myFilter != NULL) {
            [aFilePanel setAllowedFileTypes: myFilter];
        }

    ST_DISABLE_DEPRECATION_WARNINGS
        if([aFilePanel runModal] == NSOKButton) {
            myIsFileSelected = true;
            // automatically convert filenames from decomposed form used by Mac OS X file systems
            NSString* aFileName = [[aFilePanel filename] precomposedStringWithCanonicalMapping];
            myResult = [aFileName UTF8String];
        }
    ST_ENABLE_DEPRECATION_WARNINGS
    }

@end

bool StFileNode::openFileDialog(const StString& theFolder,
                                const StString& theTitle,
                                const StMIMEList& theFilter,
                                StString& theFilePath,
                                bool toSave) {
    if(NSApp == nil) {
        return false;
    }

    StString aFolderSrc, aFileNameSrc;
    StFileNode::getFolderAndFile(theFilePath, aFolderSrc, aFileNameSrc);

    StCocoaLocalPool aLocalPool;
    NSString* aTitle     = [NSString stringWithUTF8String: theTitle.toCString()];
    NSString* aFolder    = [NSString stringWithUTF8String: theFolder.toCString()];
    NSString* anInitFile = [NSString stringWithUTF8String: aFileNameSrc.toCString()];
    NSMutableArray* aFilter = NULL;
    if(!theFilter.isEmpty()) {
        aFilter = [NSMutableArray arrayWithCapacity: theFilter.size()];
        for(size_t aMimeId = 0; aMimeId < theFilter.size(); ++aMimeId) {
            const StMIME& aMime = theFilter[aMimeId];
            [aFilter addObject: [NSString stringWithUTF8String: aMime.getExtension().toCString()]];
        }
    }

    StOpenFileInfo* anOpenFile = [[StOpenFileInfo alloc] init: aFilter
                                                    withTitle: aTitle
                                                   withFolder: aFolder
                                                     withFile: anInitFile
                                                       toSave: toSave];
    if([NSThread isMainThread]) {
        [anOpenFile doDialog: NULL];
    } else {
        // create open file dialog in main thread due to Cocoa stupidity
        [anOpenFile performSelectorOnMainThread: @selector(doDialog:)
                                     withObject: NULL
                                  waitUntilDone: YES];
    }

    bool isFileSelected = anOpenFile->myIsFileSelected;
    theFilePath = anOpenFile->myResult;
    [anOpenFile release];

    return isFileSelected;
}

#endif // __APPLE__
