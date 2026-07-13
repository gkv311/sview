/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StStrings/StLogger.h>

#include <StStrings/stConsole.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>

void StMessageBox::Info(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::InfoConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
ST_DISABLE_DEPRECATION_WARNINGS
    NSRunAlertPanel(@"Info", @"%@", @"OK", nil, nil, aMessage);
ST_ENABLE_DEPRECATION_WARNINGS
}

void StMessageBox::Warn(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::WarnConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
ST_DISABLE_DEPRECATION_WARNINGS
    NSRunAlertPanel(@"Warning", @"%@", @"OK", nil, nil, aMessage);
ST_ENABLE_DEPRECATION_WARNINGS
}

void StMessageBox::Error(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::ErrorConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
ST_DISABLE_DEPRECATION_WARNINGS
    NSRunAlertPanel(@"Error", @"%@", @"OK", nil, nil, aMessage);
ST_ENABLE_DEPRECATION_WARNINGS
}

bool StMessageBox::Question(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::QuestionConsole(theMessage);
    }
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
ST_DISABLE_DEPRECATION_WARNINGS
    int aResult = NSRunAlertPanel(@"Question", @"%@", @"Yes", @"No", nil, aMessage);
    return aResult == NSAlertDefaultReturn;
ST_ENABLE_DEPRECATION_WARNINGS
}

#endif // __APPLE__
