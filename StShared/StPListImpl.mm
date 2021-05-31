/**
 * Copyright © 2011-2015 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#include <StSettings/StSettings.h>

#include <StStrings/StLogger.h>
#include <StThreads/StProcess.h>

#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>

#include <sys/stat.h>    // mkdir under Linux
#include <sys/types.h>   // mkdir under Linux

#import <Cocoa/Cocoa.h>

bool StSettings::load() {
    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(myFilePath);
    myDict = [NSMutableDictionary dictionaryWithContentsOfFile: aPath.toStringNs()];
    if(myDict == nil) {
        ST_DEBUG_LOG("StConfig, failed to parse " + myFilePath);
        myDict = [[NSMutableDictionary alloc] init];
        return false;
    } else {
        [myDict retain];
    }
    return true;
}

bool StSettings::flush() {
    if(!myToFlush) {
        return true;
    }

    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(myFilePath);
    if([myDict writeToFile: aPath.toStringNs() atomically: NO] == NO) {
        ST_DEBUG_LOG("StConfig, failed write to " + myFilePath);
        return false;
    }

    myToFlush = false;
    return true;
}

StSettings::StSettings(const StHandle<StResourceManager>& theResMgr,
                       const StString&                    theSettingsSet)
: myDict(NULL),
  myToFlush(false) {
    myFilePath = theResMgr->getSettingsFolder() + theSettingsSet + ".plist";
    load();
}

StSettings::~StSettings() {
    flush();
    [myDict release];
}

bool StSettings::loadInt32(const StString& theParamPath,
                           int32_t&        theValue) {
    StCocoaString aPath(theParamPath);
    NSObject* anObj = [myDict objectForKey: aPath.toStringNs()];
    if(anObj == nil || [anObj isKindOfClass: [NSNumber class]] == NO) {
        ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not found");
        return false;
    }
    NSNumber* aValue = (NSNumber* )anObj;
    theValue = [aValue intValue];
    return true;
}

bool StSettings::saveInt32(const StString& theParamPath,
                           const int32_t&  theValue) {
    StCocoaString aPath(theParamPath);
    NSNumber* aNumber = [[NSNumber alloc] initWithInt: theValue];
    [myDict setObject: aNumber forKey: aPath.toStringNs()];
    [aNumber release];
    myToFlush = true;
    return true;
}

bool StSettings::loadString(const StString& theParamPath,
                            StString&       theValue) {
    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(theParamPath);
    NSObject* anObj = [myDict objectForKey: aPath.toStringNs()];
    if(anObj == nil || [anObj isKindOfClass: [NSString class]] == NO) {
        ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not found");
        return false;
    }
    NSString* aValue = (NSString* )anObj;
    theValue = [aValue UTF8String];
    return true;
}

bool StSettings::saveString(const StString& theParamPath,
                            const StString& theValue) {
    StCocoaString aPath(theParamPath);
    StCocoaString aValue(theValue);
    [myDict setObject: aValue.toStringNs() forKey: aPath.toStringNs()];
    myToFlush = true;
    return true;
}

#endif // __APPLE__
