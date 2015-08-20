/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
    NSRunAlertPanel(@"Info", aMessage, @"OK", nil, nil);
}

void StMessageBox::Warn(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::WarnConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Warning", aMessage, @"OK", nil, nil);
}

void StMessageBox::Error(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::ErrorConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Error", @"%@", @"OK", nil, nil, aMessage);
}

bool StMessageBox::Question(const StString& theMessage) {
    if(NSApp == nil) {
        return StMessageBox::QuestionConsole(theMessage);
    }
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    int aResult = NSRunAlertPanel(@"Question", @"%@", @"Yes", @"No", nil, aMessage);
    return aResult == NSAlertDefaultReturn;
}

#endif // __APPLE__
