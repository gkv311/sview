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

void stInfo(const StString& theMessage) {
    if(NSApp == nil) {
        return stInfoConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Info", aMessage, @"OK", nil, nil);
}

void stWarn(const StString& theMessage) {
    if(NSApp == nil) {
        return stWarnConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Warning", aMessage, @"OK", nil, nil);
}

void stError(const StString& theMessage) {
    if(NSApp == nil) {
        return stErrorConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Error", aMessage, @"OK", nil, nil);
}

void stSuccess(const StString& theMessage) {
    if(NSApp == nil) {
        return stSuccessConsole(theMessage);
    }
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    NSRunAlertPanel(@"Success", aMessage, @"OK", nil, nil);
}

bool stQuestion(const StString& theMessage) {
    if(NSApp == nil) {
        return stQuestionConsole(theMessage);
    }
    StCocoaLocalPool aLocalPool;
    NSString* aMessage = [NSString stringWithUTF8String: theMessage.toCString()];
    int aResult = NSRunAlertPanel(@"Question", aMessage, @"Yes", @"No", nil);
    return aResult == NSAlertDefaultReturn;
}

#endif // __APPLE__
